#pragma once
#include <utility>

class CompileTime {
private:
	//These are constants which crt.rand use
	constexpr static unsigned Rand_Multiplier = 0x343FD;
	constexpr static unsigned Rand_Increment = 0x269EC3;
	constexpr static unsigned Rand_Modulus = 0x7FFF;
public:
	template<class Type>
	constexpr static unsigned Hash(const Type* Data, size_t Size, unsigned Seed) {
		unsigned Result = 5381 + Seed;
		for (auto i = 0; i < Size; i++)
			Result = Data[i] + 33 * Result;
		return Result;
	}

	constexpr static unsigned TimeSeed = Hash(__DATE__, sizeof(__DATE__), 0);

	constexpr static unsigned Rand(unsigned& Seed) {
		Seed = Seed * Rand_Multiplier + Rand_Increment;
		return (Seed >> 0x10) & Rand_Modulus;
	}

	template<class Type>
	constexpr static size_t StrLen(const Type* str) {
		size_t Len = 0;
		while (str[Len++]);
		return Len - 1;
	}

	template<class Type>
	constexpr static unsigned Hash(const Type* Data, size_t Size) {
		return Hash(Data, Size, TimeSeed);
	}

	template<class Type>
	constexpr static unsigned StrHash(const Type* Str) {
		return Hash(Str, StrLen(Str) + 1);
	}

	template<class Type>
	constexpr static unsigned StrHash(std::basic_string<Type> Str) {
		return StrHash(Str.c_str());
	}

	consteval static unsigned Rand(unsigned Count, unsigned Seed) {
		for (unsigned i = 0; i < Count; i++)
			Rand(Seed);
		return Rand(Seed);
	}

	consteval static auto ConstEval(auto Val) { return Val; }

	template <size_t N>
	constexpr static void Repeat(auto f) {
		[f] <auto... Index>(std::index_sequence<Index...>) [[msvc::forceinline]] {
			(f.operator() < Index > (), ...);
		}(std::make_index_sequence<N>());
	}
};

#pragma warning(disable : 4455)
consteval static unsigned operator""h(const char* str, size_t len) { return CompileTime::StrHash(str); }
consteval static unsigned operator""h(const wchar_t* str, size_t len) { return CompileTime::StrHash(str); }
consteval static unsigned operator""h(const char8_t* str, size_t len) { return CompileTime::StrHash(str); }
consteval static unsigned operator""h(const char16_t* str, size_t len) { return CompileTime::StrHash(str); }
consteval static unsigned operator""h(const char32_t* str, size_t len) { return CompileTime::StrHash(str); }