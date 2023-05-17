#include "ACPI.h"
#include "utils.h"
#include "multiboot.h"
#include "Logging.h"
#include "VMM.h"
#include "PMM.h"
#include "MEM.h"

uint64_t apic_base_address;
uint64_t ioapic_base_address;
struct MADTEntry_IOAPIC_InterruptSourceOverride ioapic_source_overrides[IO_APIC_SOURCE_OVERRIDE_MAX_ITEMS];
uint8_t ioapic_source_override_array_size;

inline unsigned char in(int portnum)
{
    unsigned char data=0;
    __asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (portnum));       
    return data;
}

inline void out(int portnum, unsigned char data)
{
    __asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data),"d" (portnum));        
}

inline uint64_t rdmsr(uint32_t address){
    uint32_t low=0, high=0;
    asm("movl %2, %%ecx;" 
        "rdmsr;"
        : "=a" (low), "=d" (high)
        : "g" (address)
    );

    return (uint64_t) low | ((uint64_t)high << 32);
}

inline void wrmsr(uint32_t address, uint64_t value)
{
    asm("wrmsr"
    :
    : "a" ((uint32_t)value), "d"(value >> 32), "c"(address)
    );
}

inline void __get_cpuid(uint32_t function, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
                 : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                 : "a" (function));
}

// setting up local-apic
inline void disable_pic() {
    out(PIC_COMMAND_MASTER, ICW_1);
    out(PIC_COMMAND_SLAVE, ICW_1);
    out(PIC_DATA_MASTER, ICW_2_M);
    out(PIC_DATA_SLAVE, ICW_2_S);
    out(PIC_DATA_MASTER, ICW_3_M);
    out(PIC_DATA_SLAVE, ICW_3_S);
    out(PIC_DATA_MASTER, ICW_4);
    out(PIC_DATA_SLAVE, ICW_4);
    out(PIC_DATA_MASTER, 0xFF);
    out(PIC_DATA_SLAVE, 0xFF);
}

unsigned char SDTChecksum(struct SDTHeader *tableHeader)
{
    unsigned char sum = 0;
 
    for (uint32_t i = 0; i < tableHeader->Length; i++)
    {
        sum += ((char *) tableHeader)[i];
    }
 
    return sum;
}

uint8_t validate_ACPIChecksum(struct SDTHeader *sdtHeader)
{
    uint32_t checksum = SDTChecksum(sdtHeader);
    return (checksum == 0);
}

uint8_t validate_RSDPChecksum(struct RSDPDescriptor* rsdp_desc) {
    // CHECKSUM CALC
    uint64_t checksum_itr=0;
    uint8_t rsdp_checksum = 0;
    for(uint8_t* ptr = (uint8_t*)rsdp_desc; checksum_itr < sizeof(struct RSDPDescriptor); checksum_itr++, ptr++)
    {
        rsdp_checksum += *ptr;
    }

    return (rsdp_checksum == 0);
}

/*       Advanced Configuration and Power interface (ACPI)      */
void acpi_init(struct multiboot_info_header* mboot_header) {
    struct multiboot_tag_old_acpi* acpi = (struct multiboot_tag_old_acpi*)find_tag(mboot_header, MULTIBOOT_TAG_TYPE_ACPI_OLD);

    struct RSDPDescriptor* rsdp = (struct RSDPDescriptor*)acpi->rsdp;
    if(!validate_RSDPChecksum(rsdp)) {
        logf("RSDP Invalid Checksum\n");
        return;
    }
    struct RSDT* rsdt = (struct RSDT*)(uint64_t)rsdp->RSDTAddress;
    // map RSDT memory
    identity_map_phys_address((void*)rsdt, PRESENT_BIT);
    _bitmap_set_bit_from_address(ALIGN_PHYSADDRESS((uint64_t)rsdt));
    uint32_t header_pages_count =rsdt->sdtHeader.Length / PAGE_SIZE + 1;
    // map more pages, if needed
    if(header_pages_count > 1) {
        for(uint32_t i=0; i < header_pages_count; i++) {
            void* next_page_address = VPTR(rsdp->RSDTAddress + (i * PAGE_SIZE));
            identity_map_phys_address(next_page_address, PRESENT_BIT);
            _bitmap_set_bit_from_address((uint64_t)next_page_address);
        }
    }
    if(!validate_ACPIChecksum(&(rsdt->sdtHeader))) {
        logf("RSDT Invalid Checksum\n");
        return;
    }
    // map all SDT addresses
    for(uint32_t i=0; i < RSDT_ITEMS_COUNT(rsdt); i++) {
        identity_map_phys_address((void*)ALIGN_PHYSADDRESS((uint64_t)rsdt->sdtAddresses[i]), PRESENT_BIT);
        _bitmap_set_bit_from_address(ALIGN_PHYSADDRESS((uint64_t)rsdt->sdtAddresses[i]));
    }
    // sdt_apic
    struct MADT* madt = (struct MADT*)find_sdt(mboot_header, SDT_SIGNATURE_APIC);
    struct MADTEntry*  madt_entries = (struct MADTEntry* )((uint64_t)madt + sizeof(struct MADT));

    // map madt data
    identity_map_phys_address((void*)ALIGN_PHYSADDRESS((uint64_t)madt_entries), PRESENT_BIT);
    _bitmap_set_bit_from_address(ALIGN_PHYSADDRESS((uint64_t)madt_entries));

    apic_init();
    ioapic_init(madt);
}

// ====================== LOCAL APIC ======================

void apic_init() {
    // configure lapic
    uint64_t msr_output = rdmsr(IA32_APIC_BASE);
    apic_base_address = (msr_output&APIC_BASE_ADDRESS_MASK);
    if(apic_base_address == 0) {
        logf("Could not determine apic-base-address\n");
        return;
    }

    uint32_t ignored, xApicLeaf = 0, x2ApicLeaf = 0;
    __get_cpuid(1, &ignored, &ignored, &x2ApicLeaf, &xApicLeaf);
    (void)ignored;

    if(x2ApicLeaf & (1 << 21)) {
        logf("X2APIC Supported!");
        // x2apic is accessed through msr
        // once enabled, cannot be disabled without resetting the system
        msr_output |= (1 << 10);
        write_apic_register(IA32_APIC_BASE, msr_output);
    } else if(xApicLeaf & (1 << 9)) {

        logf("XAPIC Supported");
        // xapic is accessed through mmio (memory mapped registers)
        identity_map_phys_address((void*)(uint64_t)apic_base_address, PRESENT_BIT | PF_WRITE);
    }

    // setup spurious vector register entry
    // uint32_t spurious_interrupt_reg = read_apic_register(APIC_SPURIOUS_VECTOR_REGISTER_OFFSET);

    if(!(1 & (msr_output >> APIC_GLOBAL_ENABLE_BIT))) {
        logf("APIC Disabled Globally\n");
        return;
    }

    write_apic_register(APIC_SPURIOUS_VECTOR_REGISTER_OFFSET, APIC_SOFTWARE_ENABLE | INTR_APIC_SPURIOUS_INTERRUPT);

    if(apic_base_address < phys_mem.total_memory) {
        logf("APIC base address in physical memory area");
        _bitmap_set_bit(FRAME_POS1(apic_base_address));
    }

    disable_pic();
}

uint32_t read_apic_register(uint32_t register_offset) {
    return READMEM32(apic_base_address + register_offset);
}

void write_apic_register(uint32_t register_offset, uint32_t value) {
    WRITEMEM32(apic_base_address + register_offset, value);
}

uint32_t lapic_id()
{
    uint32_t id = read_apic_register(APIC_ID_REGISTER_OFFSET);
    return (id >> 24);
}


// ========================= IOAPIC ============================
void ioapic_init(struct MADT* madt) {
    struct MADTEntry_IOAPIC* ioapic = (struct MADTEntry_IOAPIC*)find_madt_record(madt, MADT_IO_APIC, 0);
    // uint8_t ioapic_source_override_array_size = 0;
    if(ioapic == NULL) return;

    if(is_phyisical_address_mapped((uint64_t)ioapic, (uint64_t)ioapic) == ADDRESS_NOT_MAPPED) {
        identity_map_phys_address((void*)ioapic, PRESENT_BIT);
    }

    ioapic_base_address = (uint64_t)ioapic->address;
    identity_map_phys_address((void*)ioapic_base_address, PRESENT_BIT);
    _bitmap_set_bit(FRAME_POS1(ioapic_base_address));

    // uint32_t ioapic_version = read_ioapic_register(IO_APIC_VER_OFFSET);
    IOAPIC_RedirectEntry redtbl_entry;
    if(read_ioapic_redirect(010, &redtbl_entry) != 0) {
        logf("IOAPIC Read Redirecty Failed!");
        return;
    }

    // uint8_t ioapic_redirections_count = (uint8_t)(ioapic_version >> 16);
    ioapic_source_override_array_size = parse_ioapic_interrupt_source_overrides(madt);
}

int parse_ioapic_interrupt_source_overrides(struct MADT* madt) {
    uint32_t total_length = sizeof(struct MADT);
    uint32_t source_override_count = 0;
    struct MADTEntry* madt_record = find_madt_record(madt, MADT_IO_APIC_INTERRUPT_SOURCE_OVERRIDE, source_override_count);

    while(total_length < madt->sdtHeader.Length && source_override_count < IO_APIC_SOURCE_OVERRIDE_MAX_ITEMS)
    {
        total_length += madt_record->length;

        if(madt_record != NULL && madt_record->type ==  MADT_IO_APIC_INTERRUPT_SOURCE_OVERRIDE)
        {
            struct MADTEntry_IOAPIC_InterruptSourceOverride* madt_record_source_override = (struct MADTEntry_IOAPIC_InterruptSourceOverride*)madt_record;
            memcpy((void*)(
                ioapic_source_overrides + source_override_count), 
                (void*)madt_record_source_override, 
                sizeof(struct MADTEntry_IOAPIC_InterruptSourceOverride));

            source_override_count++;
        }

        madt_record = find_madt_record(madt, MADT_IO_APIC_INTERRUPT_SOURCE_OVERRIDE, source_override_count);
        if(madt_record == NULL) break;
    }
    return source_override_count;
}

uint32_t read_ioapic_register(uint8_t offset) {
    if(ioapic_base_address == 0) return 0;

    *(volatile uint32_t*)ioapic_base_address = offset;
    return *(volatile uint32_t*)(ioapic_base_address + 0x10);
}

void write_ioapic_register(uint8_t offset, uint32_t value) {
    *(volatile uint32_t*) ioapic_base_address = offset;
    *(volatile uint32_t*) (ioapic_base_address + 0x10) = value;
}

int read_ioapic_redirect(uint8_t index, IOAPIC_RedirectEntry *redtbl_entry) {
    if (index < 0x10 && index > 0x3F) {
        return -1;
    }
    if ((index%2) != 0) {
        return -1;
    }
    uint32_t low_word;
    uint32_t high_word;
    low_word = read_ioapic_register(index);
    high_word = read_ioapic_register(index+1);
    uint64_t raw_value = ((uint64_t) high_word << 32) | ((uint64_t) low_word);
    redtbl_entry->raw = raw_value;
    return 0;
}

int write_ioapic_redirect(uint8_t index, IOAPIC_RedirectEntry redtbl_entry) {
    if (index < 0x10 && index > 0x3F) {
        return -1;
    }
    if ((index%2) != 0) {
        return -1;
    }
    uint32_t high_word = (uint32_t) (redtbl_entry.raw >> 32);
    uint32_t low_word = (uint32_t) redtbl_entry.raw;
    write_ioapic_register(index, low_word);
    write_ioapic_register(index+1, high_word);
    return 0;
}

void set_irq(uint8_t irq_type, uint8_t redirect_table_index, uint8_t idt_entry, uint8_t destination_field, uint32_t flags, bool masked)
{
    uint8_t counter = 0;
    // uint8_t selected_pin = irq_type;
    IOAPIC_RedirectEntry entry;
    entry.raw = flags | (idt_entry & 0xFF);
    while(counter < ioapic_source_override_array_size) {
        if(ioapic_source_overrides[counter].irq_source == irq_type) {
            // selected_pin = ioapic_source_overrides[counter].global_system_interrupt;
            if((ioapic_source_overrides[counter].flags & 0b11) == 2) {
                entry.pin_polarity = 0b1;
            } else {
                entry.pin_polarity = 0b0;
            }
            if(((ioapic_source_overrides[counter].flags >>2) & 0b11) == 2) {
                entry.pin_polarity = 0b1;
            } else {
                entry.pin_polarity = 0b0;
            }
            break;
        }
        counter++;
    }

    entry.destination_field = destination_field;
    entry.interrupt_mask = masked;
    write_ioapic_redirect(redirect_table_index, entry);
    // IOAPIC_RedirectEntry read_entry;
    // int ret_val = read_ioapic_redirect(IOREDTBL1, &read_entry);
}

int set_irq_mask(uint8_t redirect_table_index, bool masked_status) {
    IOAPIC_RedirectEntry read_entry;
    int ret_val = read_ioapic_redirect(redirect_table_index, &read_entry);
    if(ret_val==0) {
        read_entry.interrupt_mask = masked_status;
        write_ioapic_redirect(redirect_table_index, read_entry);
    } else {
        logf("Invalid redirect table index\n");
        return -1;
    }
    return 0;
}