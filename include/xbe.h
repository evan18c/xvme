// XBE Structs
// Author: Evan Cassidy
// Date: 4/22/2026
#ifndef XBE_H
#define XBE_H

#include <stdint.h>

typedef struct {
    uint32_t MagicNumber;
    char DigitalSignature[256];
    uint32_t BaseAddress;
    uint32_t SizeOfHeaders;
    uint32_t SizeOfImage;
    uint32_t SizeOfImageHeader;
    uint32_t TimeDate;
    uint32_t CertificateAddress;
    uint32_t NumberOfSections;
    uint32_t SectionHeaderAddress;
    uint32_t InitializationFlags;
    uint32_t EntryPoint;
    uint32_t TLSAddress;
    uint32_t StackSize;
    uint32_t PEHeapReserve;
    uint32_t PEHeapCommit;
    uint32_t PEBaseAddress;
    uint32_t PESizeOfImage;
    uint32_t PEChecksum;
    uint32_t PETimeDate;
    uint32_t DebugPathNameAddress;
    uint32_t DebugFileNameAddress;
    uint32_t UTF16DebugFileNameAddress;
    uint32_t KernelImageThunkAddress;
    uint32_t NonKernelImportDirectoryAddress;
    uint32_t NumberOfLibraryVersions;
    uint32_t LibraryVersionsAddress;
    uint32_t KernelLibraryVersionAddress;
    uint32_t XAPILibraryVersionAddress;
    uint32_t LogoBitmapAddress;
    uint32_t LogoBitmapSize;
} XBEImageHeader;

typedef struct {
    uint32_t SizeOfCertificate;
    uint32_t TimeDate;
    uint32_t TitleID;
    char TitleName[80];
    char AlternateTitleIDs[64];
    uint32_t AllowedMedia;
    uint32_t GameRegion;
    uint32_t GameRatings;
    uint32_t DiskNumber;
    uint32_t Version;
    char LANKey[16];
    char SignatureKey[16];
    char AlternateSignatureKeys[256];
} XBECertificate;

typedef struct {
    uint32_t SectionFlags;
    uint32_t VirtualAddress;
    uint32_t VirtualSize;
    uint32_t RawAddress;
    uint32_t RawSize;
    uint32_t SectionNameAddress;
    uint32_t SectionNameReferenceCount;
    uint32_t HeadSharedPageReferenceCountAddress;
    uint32_t TailSharedPageReferenceCountAddress;
    char SectionDigest[20];
} XBESectionHeader;

typedef struct {
    char LibraryName[8];
    uint16_t MajorVersion;
    uint16_t MinorVersion;
    uint16_t BuildVersion;
    uint32_t LibraryFlags;
} XBELibraryVersion;

typedef struct {
    uint32_t DataStartAddress;
    uint32_t DataEndAddress;
    uint32_t TLSIndexAddress;
    uint32_t TLSCallbackAddress;
    uint32_t SizeOfZeroFill;
    uint32_t Characteristics;
} XBETLS;

#endif