#ifndef RSDP_H
#define RSDP_H

#include <stdint.h>
#include <stddef.h>

// Root System Descriptro Table (Harware Infor)
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

struct RSDP {
    struct SDTHeader sdtHeader;    // signature "RSDP"
    uint32_t sdtAddresses[];
} __attribute__((packed));

struct XSDT {
    struct SDTHeader sdtHeader;    // signature "XSDT"
    uint64_t sdtAddresses[];
} __attribute__((packed));

uint8_t validate_RSDP(char* byte_array, size_t size);

#endif