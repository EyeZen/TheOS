#ifndef _RSDP_H
#define _RSDP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot.h"

// Root System Descriptro Table (Hardware Information) Descriptor
struct RSDPDescriptor {
  char Signature[8]; // "RSDP PTR", not null terminated
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RSDTAddress;
} __attribute__((packed));

struct RSDPDescriptorV2 {
  char Signature[8]; // "RSDP PTR", not null terminated
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RSDTAddress;

  // v2 fields
  uint32_t Length;
  uint64_t XSDTAddress;
  uint8_t ExtendedChecksum;
  uint8_t Reserved[3];
} __attribute__((packed));




// System Descriptor Table Headers
struct SDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__((packed));

struct RSDT {
    struct SDTHeader sdtHeader;    // signature "RSDP"
    uint32_t sdtAddresses[];
} __attribute__((packed));

struct XSDT {
    struct SDTHeader sdtHeader;    // signature "XSDT"
    uint64_t sdtAddresses[];
} __attribute__((packed));





// The ACPI SDT
struct MADT {
  struct SDTHeader sdtHeader;
  uint32_t local_apic_base;
  uint32_t flags;
} __attribute__((packed));

// MADT entry header
struct MADTEntry {
  uint8_t type;
  uint8_t length;
} __attribute__((packed));

// Entry Type 0
struct MADTEntry_ProcessorLocalAPIC {
  struct MADTEntry header;
  uint8_t acpi_processor_id;
  uint8_t apic_id;
  uint32_t flags;
} __attribute__((packed));
// Entry Type 1
struct MADTEntry_IOAPIC
{
  struct MADTEntry header;
  uint8_t id;
  uint8_t reserved;
  uint32_t address;
  uint64_t global_system_interrupt_base;
} __attribute__((packed));
// Entry Type 2
struct MADTEntry_IOAPIC_InterruptSourceOverride
{
  struct MADTEntry header;
  uint8_t bus_source;
  uint8_t irq_source;
  uint32_t global_system_interrupt;
  uint16_t flags;
} __attribute__((packed));
// Entry Type 3
struct MADTEntry_IOAPIC_NMISource
{
  struct MADTEntry header;
  uint8_t nmi_source;
  uint8_t reserved;
  uint16_t flags;
  uint32_t global_system_interrupt;
} __attribute__((packed));
// Entry Type 4
struct MADTEntry_LAPIC_NMI
{
  struct MADTEntry header;
  uint8_t acpi_processor_id;
  uint16_t flags;
  uint8_t LINT;
} __attribute__((packed));
// Entry Type 5
struct MADTEntry_LAPIC_AddressOverride
{
  struct MADTEntry header;
  uint16_t reserved;
  uint64_t lapic_physical_address;
} __attribute__((packed));
// Entry Type 9
struct MADTEntry_Localx2APIC
{
  struct MADTEntry header;
  uint16_t reserved;
  uint32_t id;
  uint32_t flags;
  uint32_t acpi_id;
} __attribute__((packed));

union IOAPIC_RedirectEntry_t
{
  struct {
      uint64_t    vector  :8;
      uint64_t    delivery_mode   :3;
      uint64_t    destination_mode    :1;
      uint64_t    delivery_status :1;
      uint64_t    pin_polarity    :1;
      uint64_t    remote_irr  :1;
      uint64_t    trigger_mode    :1;
      uint64_t    interrupt_mask  :1;
      uint64_t    reserved    :39;
      uint64_t    destination_field   :8;
  };
  uint64_t raw;
} __attribute__((packed));

typedef union IOAPIC_RedirectEntry_t IOAPIC_RedirectEntry;


/* -------------- Root System Descriptors ------------- */
#define RSDT_ITEMS_COUNT(rsdt) ((rsdt->sdtHeader.Length - sizeof(struct SDTHeader)) / sizeof(uint32_t))
#define XSDT_ITEMS_COUNT(xsdt) ((xsdt->sdtHeader.Length - sizeof(struct SDTHeader)) / sizeof(uint64_t))

#define SDT_SIGNATURE_RSDT "RSDT"
#define SDT_SIGNATURE_XSDT "XSDT"

#define SDT_SIGNATURE_FACP "FACP"
#define SDT_SIGNATURE_APIC "APIC"
#define SDT_SIGNATURE_HPET "HPET"
#define SDT_SIGNATURE_WAET "WAET"
#define SDT_SIGNATURE_SSDT "SSDT"
#define SDT_SIGNATURE_DSDT "DSDT"
#define SDT_SIGNATURE_MCFG "MCFG"
#define SDT_SIGNATURE_DBG2 "DBG2"

/* ----------- Programmable Interrupt Controller ---------- */
#define PIC_COMMAND_MASTER 0x20
#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE 0xA1

// start of initialization sequence
#define ICW_1   0x11  
// interrupt vector addresses (8) above 31 system reserved
#define ICW_2_M 0x20
#define ICW_2_S 0x28
// verify and modify
#define ICW_3_M 0x04
#define ICW_3_S 0x02
// 8086 mode of operation
#define ICW_4   0x01

/* ------------------ Local APIC ------------------ */

#define APIC_SPURIOUS_VECTOR_REGISTER_OFFSET 0xF0
#define APIC_SPURIOUS_INTERRUPT_IDT_ENTRY 0xFF
#define APIC_SOFTWARE_ENABLE (1 << 8)
#define APIC_ID_REGISTER_OFFSET 0x20

#define IA32_APIC_BASE 0x1b

#define APIC_BSP_BIT 8
#define APIC_GLOBAL_ENABLE_BIT 11
#define APIC_BASE_ADDRESS_MASK 0xFFFFF000

/* ---------------- IOAPIC ---------------- */

#define MADT_PROCESSOR_LOCAL_APIC    0
#define MADT_IO_APIC 1
#define MADT_IO_APIC_INTERRUPT_SOURCE_OVERRIDE 2
#define MADT_NMI_INTERRUPT_SOURCE    3
#define MADT_LOCAL_APIC_NMI  4
#define MADT_LOCAL_APIC_ADDRESS_OVERRIDE 5
#define MADT_PRORCESSOR_LOCAL_X2APIC 9



#define IO_APIC_ID_OFFSET   0x0
#define IO_APIC_VER_OFFSET  0x1
#define IO_APIC_ARB_OFFSET  0x2
#define IO_APIC_REDTBL_START_OFFSET 0x10

#define IO_APIC_SOURCE_OVERRIDE_MAX_ITEMS   0x10

#define IO_APIC_DELIVERY_MODE_FIXED 0x0
#define IO_APIC_DESTINATION_MODE_PHYSICAL 0x0
#define IO_APIC_INTERRUPT_PIN_POLARITY_HIGH_ACTIVE  0x0
#define IO_APIC_TRIGGER_MODE_EDGE   0x0

#define IO_APIC_POLARITY_BIT_MASK 0x2000
#define IO_APIC_TRIGGER_MODE_BIT_MASK 0x8000
#define TIMER_IRQ 0x00
#define KEYBOARD_IRQ 0x01
#define PIT_IRQ 0x02

#define IOREDTBL0   0x10
#define IOREDTBL1   0x12
#define IOREDTBL2   0x14
#define IOREDTBL3   0x16
#define IOREDTBL4   0x18
#define IOREDTBL5   0x1A
#define IOREDTBL6   0x1C
#define IOREDTBL7   0x1E
#define IOREDTBL8   0x20
#define IOREDTBL9   0x22
#define IOREDTBL10  0x24
#define IOREDTBL11  0x26
#define IOREDTBL12  0x28
#define IOREDTBL13  0x2A
#define IOREDTBL14  0x2C
#define IOREDTBL15  0x2E
#define IOREDTBL16  0x30
#define IOREDTBL17  0x32
#define IOREDTBL18  0x34
#define IOREDTBL19  0x36
#define IOREDTBL20  0x38
#define IOREDTBL21  0x3A
#define IOREDTBL22  0x3C
#define IOREDTBL23  0x3E


unsigned char SDTChecksum(struct SDTHeader *tableHeader);
uint8_t validate_ACPIChecksum(struct SDTHeader *sdtHeader);
uint8_t validate_RSDPChecksum(struct RSDPDescriptor* rsdp_desc);
void acpi_init(struct multiboot_info_header* mboot_header);

// LAPIC
void apic_init();
uint32_t read_apic_register(uint32_t register_offset);
void write_apic_register(uint32_t register_offset, uint32_t value);
uint32_t lapic_id();
bool lapic_is_x2();

// IOAPIC
void ioapic_init(struct MADT* madt);
int parse_ioapic_interrupt_source_overrides(struct MADT* madt);
uint32_t read_ioapic_register(uint8_t offset);
void write_ioapic_register(uint8_t offset, uint32_t value);
int read_ioapic_redirect(uint8_t index, IOAPIC_RedirectEntry *redtbl_entry);
int write_ioapic_redirect(uint8_t index, IOAPIC_RedirectEntry redtbl_entry);

void set_irq(uint8_t irq_type, uint8_t redirect_table_index, uint8_t idt_entry, uint8_t destination_field, uint32_t flags, bool masked);
int set_irq_mask(uint8_t redirect_table_index, bool masked_status);

extern uint64_t apic_base_address;
extern uint64_t ioapic_base_address;
extern uint8_t ioapic_source_override_array_size;

#endif