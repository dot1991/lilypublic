#pragma once
#include <windows.h>
#include "function_ref.hpp"

static_assert(sizeof(uintptr_t) == 8, "Size mismatch, only 64-bit supported.");

struct CR3
{
    union
    {
        struct
        {
            uintptr_t Ignored : 3;
            uintptr_t PWT : 1;
            uintptr_t PCD : 1;
            uintptr_t Ignored2 : 7;
            uintptr_t PageFrameNumber : 36;
            uintptr_t Reserved : 16;
        };
        uintptr_t Value;
    };
    bool IsValid() const { return Value != 0; }
    operator uintptr_t() const { return Value; }
    CR3& operator=(uintptr_t Val) { Value = Val; return *this; }
    CR3(uintptr_t Val = 0) : Value(Val) {}
};
struct PML4E
{
    union
    {
        struct
        {
            uintptr_t Present : 1;              // Must be 1, region invalid if 0.
            uintptr_t ReadWrite : 1;            // If 0, writes not allowed.
            uintptr_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uintptr_t PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
            uintptr_t PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
            uintptr_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uintptr_t Ignored1 : 1;
            uintptr_t PageSize : 1;             // Must be 0 for PML4E.
            uintptr_t Ignored2 : 4;
            uintptr_t PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
            uintptr_t Reserved : 4;
            uintptr_t Ignored3 : 11;
            uintptr_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uintptr_t Value;
    };
    bool IsValid() const { return Value != 0; }
    operator uintptr_t() const { return Value; }
    PML4E& operator=(uintptr_t Val) { Value = Val; return *this; }
    PML4E(uintptr_t Val = 0) : Value(Val) {}
};
struct PDPTE
{
    union
    {
        struct
        {
            uintptr_t Present : 1;              // Must be 1, region invalid if 0.
            uintptr_t ReadWrite : 1;            // If 0, writes not allowed.
            uintptr_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uintptr_t PageWriteThrough : 1;     // Determines the memory type used to access PD.
            uintptr_t PageCacheDisable : 1;     // Determines the memory type used to access PD.
            uintptr_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uintptr_t Ignored1 : 1;
            uintptr_t PageSize : 1;             // If 1, this entry maps a 1GB page.
            uintptr_t Ignored2 : 4;
            uintptr_t PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
            uintptr_t Reserved : 4;
            uintptr_t Ignored3 : 11;
            uintptr_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uintptr_t Value;
    };
    bool IsValid() const { return Value != 0; }
    operator uintptr_t() const { return Value; }
    PDPTE& operator=(uintptr_t Val) { Value = Val; return *this; }
    PDPTE(uintptr_t Val = 0) : Value(Val) {}
};
struct PDE
{
    union
    {
        struct
        {
            uintptr_t Present : 1;              // Must be 1, region invalid if 0.
            uintptr_t ReadWrite : 1;            // If 0, writes not allowed.
            uintptr_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uintptr_t PageWriteThrough : 1;     // Determines the memory type used to access PT.
            uintptr_t PageCacheDisable : 1;     // Determines the memory type used to access PT.
            uintptr_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uintptr_t Ignored1 : 1;
            uintptr_t PageSize : 1;             // If 1, this entry maps a 2MB page.
            uintptr_t Ignored2 : 4;
            uintptr_t PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
            uintptr_t Reserved : 4;
            uintptr_t Ignored3 : 11;
            uintptr_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uintptr_t Value;
    };
    bool IsValid() const { return Value != 0; }
    operator uintptr_t() const { return Value; }
    PDE& operator=(uintptr_t Val) { Value = Val; return *this; }
    PDE(uintptr_t Val = 0) : Value(Val) {}
};
struct PTE
{
    union
    {
        struct
        {
            uintptr_t Present : 1;              // Must be 1, region invalid if 0.
            uintptr_t ReadWrite : 1;            // If 0, writes not allowed.
            uintptr_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uintptr_t PageWriteThrough : 1;     // Determines the memory type used to access the memory.
            uintptr_t PageCacheDisable : 1;     // Determines the memory type used to access the memory.
            uintptr_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uintptr_t Dirty : 1;                // If 0, the memory backing this page has not been written to.
            uintptr_t PageAccessType : 1;       // Determines the memory type used to access the memory.
            uintptr_t Global : 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
            uintptr_t Ignored2 : 3;
            uintptr_t PageFrameNumber : 36;     // The page frame number of the backing physical page.
            uintptr_t Reserved : 4;
            uintptr_t Ignored3 : 7;
            uintptr_t ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
            uintptr_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uintptr_t Value;
    };
    bool IsValid() const { return Value != 0; }
    operator uintptr_t() const { return Value; }
    PTE& operator=(uintptr_t Val) { Value = Val; return *this; }
    PTE(uintptr_t Val = 0) : Value(Val) {}
};

class PhysicalAddress {
private:
    //Maximum Address 1TB
    constexpr static size_t MAXPHYADDR_BITSIZE = 40;
    union {
        uintptr_t PA = 0;
        uintptr_t _PA : MAXPHYADDR_BITSIZE;
    };
public:
    void MaskPA(uintptr_t NewPA) { PA = NewPA; PA = _PA; }
    operator uintptr_t() const { return _PA; }
    PhysicalAddress& operator =(uintptr_t NewPA) { MaskPA(NewPA); return *this; }
    PhysicalAddress(uintptr_t NewPA) { MaskPA(NewPA); }
    PhysicalAddress(const PhysicalAddress& rhs) { MaskPA(rhs.PA); }
};

using tReadPhysicalMemory = tl::function<bool(PhysicalAddress PA, void* Buffer, size_t Size)>;
using tWritePhysicalMemory = tl::function<bool(PhysicalAddress PA, const void* Buffer, size_t Size)>;

class PhysicalMemory {
public:
    static PhysicalAddress GetPTEAddress(uintptr_t VirtualAddress, CR3 cr3, tReadPhysicalMemory ReadPhysicalMemory) {
        const uintptr_t Address = VirtualAddress;
        const uintptr_t IndexPML4 = (Address >> 39) & 0x1FF;
        const uintptr_t IndexPageDirPtr = (Address >> 30) & 0x1FF;
        const uintptr_t IndexPageDir = (Address >> 21) & 0x1FF;
        const uintptr_t IndexPageTable = (Address >> 12) & 0x1FF;

        PML4E EntryPML4;
        PDPTE EntryPageDirPtr;
        PDE EntryPageDir;

        if (!cr3.IsValid())
            return 0;

        if (!ReadPhysicalMemory(cr3.PageFrameNumber * 0x1000 + IndexPML4 * 8, &EntryPML4, 8))
            return 0;

        if (!EntryPML4.IsValid())
            return 0;

        if (!ReadPhysicalMemory(EntryPML4.PageFrameNumber * 0x1000 + IndexPageDirPtr * 8, &EntryPageDirPtr, 8))
            return 0;

        if (!EntryPageDirPtr.IsValid())
            return 0;

        if (EntryPageDirPtr.PageSize)
            return 0;

        if (!ReadPhysicalMemory(EntryPageDirPtr.PageFrameNumber * 0x1000 + IndexPageDir * 8, &EntryPageDir, 8))
            return 0;

        if (!EntryPageDir.IsValid())
            return 0;

        if (EntryPageDir.PageSize)
            return 0;

        return EntryPageDir.PageFrameNumber * 0x1000 + IndexPageTable * 8;
    }
    static PhysicalAddress GetPhysicalAddress(uintptr_t VirtualAddress, CR3 cr3, tReadPhysicalMemory ReadPhysicalMemory) {
        const uintptr_t Address = VirtualAddress;
        const uintptr_t IndexPML4 = (Address >> 39) & 0x1FF;
        const uintptr_t IndexPageDirPtr = (Address >> 30) & 0x1FF;
        const uintptr_t IndexPageDir = (Address >> 21) & 0x1FF;
        const uintptr_t IndexPageTable = (Address >> 12) & 0x1FF;

        PML4E EntryPML4;
        PDPTE EntryPageDirPtr;
        PDE EntryPageDir;
        PTE EntryPageTable;

        if (!cr3.IsValid())
            return 0;

        if (!ReadPhysicalMemory(cr3.PageFrameNumber * 0x1000 + IndexPML4 * 8, &EntryPML4, 8))
            return 0;

        if (!EntryPML4.IsValid())
            return 0;

        if (!ReadPhysicalMemory(EntryPML4.PageFrameNumber * 0x1000 + IndexPageDirPtr * 8, &EntryPageDirPtr, 8))
            return 0;

        if (!EntryPageDirPtr.IsValid())
            return 0;

        if (EntryPageDirPtr.PageSize)
            return EntryPageDirPtr.PageFrameNumber * 0x1000 + (Address & 0x3FFFFFFF);

        if (!ReadPhysicalMemory(EntryPageDirPtr.PageFrameNumber * 0x1000 + IndexPageDir * 8, &EntryPageDir, 8))
            return 0;

        if (!EntryPageDir.IsValid())
            return 0;

        if (EntryPageDir.PageSize)
            return EntryPageDir.PageFrameNumber * 0x1000 + (Address & 0x1FFFFF);

        if (!ReadPhysicalMemory(EntryPageDir.PageFrameNumber * 0x1000 + IndexPageTable * 8, &EntryPageTable, 8))
            return 0;

        if (!EntryPageTable.IsValid())
            return 0;

        return EntryPageTable.PageFrameNumber * 0x1000 + (Address & 0xFFF);
    }
    static bool ReadProcessMemory(uintptr_t Address, void* Buffer, size_t Size, CR3 cr3, tReadPhysicalMemory ReadPhysicalMemory) {
        while (Size > 0) {
            size_t BlockSize = 0x1000 - (Address & 0xFFF);
            if (BlockSize > Size)
                BlockSize = Size;

            PhysicalAddress PA = GetPhysicalAddress(Address, cr3, ReadPhysicalMemory);
            if (!PA)
                return false;

            if (!ReadPhysicalMemory(PA, Buffer, BlockSize))
                return false;

            Address += BlockSize;
            Size -= BlockSize;
            Buffer = (void*)(uintptr_t(Buffer) + BlockSize);
        }

        return true;
    }
    static bool WriteProcessMemory(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3, tReadPhysicalMemory ReadPhysicalMemory, tWritePhysicalMemory WritePhysicalMemory) {
        while (Size > 0) {
            size_t BlockSize = 0x1000 - (Address & 0xFFF);
            if (BlockSize > Size)
                BlockSize = Size;

            PhysicalAddress PA = GetPhysicalAddress(Address, cr3, ReadPhysicalMemory);
            if (!PA)
                return false;

            if (!WritePhysicalMemory(PA, Buffer, BlockSize))
                return false;

            Address += BlockSize;
            Size -= BlockSize;
            Buffer = (const void*)(uintptr_t(Buffer) + BlockSize);
        }

        return true;
    }
};