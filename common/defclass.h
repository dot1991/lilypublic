#pragma once
#include <cstdint>

#pragma warning(disable : 4359)

#define UNPACK(...) __VA_ARGS__

#define MemberAtOffset(type, name, offset) \
static_assert(offset > BaseOffset, "Offset must be greater than sizeof(Derived Class)");\
struct{class _zpad##name{uint8_t pad[offset-BaseOffset];}_zpad##name; type name;};

#define MemberAtOffsetZero(type, name, offset) \
static_assert(offset == BaseOffset, "Offset must be greater than sizeof(Derived Class)");\
struct{type name;};

#define DefClass(structname, derivedclass, members, ...) \
__declspec(align(8)) class structname : public derivedclass { \
	private:\
	static constexpr unsigned AlignNeeded = 8; \
	static constexpr unsigned BaseOffset = sizeof(derivedclass); \
	static_assert(sizeof(derivedclass) % AlignNeeded == 0, "DerivedClass is not aligned");\
	public: \
	structname(){} \
	union {members; uint8_t _zpad[AlignNeeded];}; \
	__VA_ARGS__ \
};

#define DefBaseClass(structname, members, ...) \
__declspec(align(8)) class structname { \
	private:\
	static constexpr unsigned AlignNeeded = 8; \
	static constexpr unsigned BaseOffset = 0; \
	public: \
	structname(){} \
	union {members; uint8_t _zpad[AlignNeeded];}; \
	__VA_ARGS__ \
};