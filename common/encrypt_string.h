#pragma once
#include <string>
#include <atomic>
#include "fixed_string.hpp"
#include "compiletime.h"

template<std::array Data>
struct EncryptedData {
public:
	using TStdArray = std::remove_cvref_t<decltype(Data)>;
	using TElem = std::remove_cvref_t<decltype(Data[0])>;
	constexpr static size_t Size = Data.size();
	constexpr static unsigned Seed = CompileTime::Hash(Data.data(), Size);
	constexpr static TStdArray _Data = [] {
		TStdArray temp;
		unsigned _Seed = Seed;
		for (size_t i = 0; i < Size; i++)
			temp[i] = Data[i] ^ (TElem)CompileTime::Rand(_Seed);
		return temp;
	}();

	__forceinline static void Decrypt(TElem* Dst) {
		CompileTime::Repeat<Size>([Dst]<size_t Index>() [[msvc::forceinline]] {
			if constexpr ((Index * sizeof(TElem)) % 4 == 0)
				std::atomic_thread_fence(std::memory_order_acq_rel);
			constexpr TElem Value = _Data[Index];
			Dst[Index] = Value;
			});

		//+[] makes lambda not inlined.
		auto _Decrypt = +[](TElem* Dst, size_t Size, unsigned _Seed) {
			for (size_t i = 0; i < Size; i++)
				Dst[i] ^= (TElem)CompileTime::Rand(_Seed);
		};

		_Decrypt(Dst, Size, Seed);
	}
};

template <fixstr::basic_fixed_string Src>
class EncryptedString {
private:
	using TElem = std::remove_cvref_t<decltype(Src[0])>;
	constexpr static auto Size = Src.size() + 1;

	template <size_t N>
	using TArray = TElem(&)[N];
	template <size_t N>
	using TStdArray = std::array<TElem, N>;
	template <size_t N>
	using TFixedString = fixstr::basic_fixed_string<TElem, N>;
	using TStdString = std::basic_string<TElem>;

public:
	void MoveString(TElem* Dst) const {
		EncryptedData<Src._data>::Decrypt(Dst);
	}

	template <size_t N>
	void MoveArray(TArray<N> Dst) const {
		static_assert(N >= Size, "Too less array size");
		MoveString(Dst);
	}

	template <size_t N>
	void MoveStdArray(TStdArray<N>& Dst) const {
		MoveArray<N>(Dst._Elems);
	}

	template <size_t N>
	void MoveFixedString(TFixedString<N>& Dst) const {
		MoveArray<N + 1>(Dst._data._Elems);
	}

	void MoveStdString(TStdString& Dst) const {
		Dst.resize(Size - 1);
		MoveString(Dst.data());
	}

	template <size_t N>
	TStdArray<N> GetStdArray() const {
		TStdArray<N> Dst;
		MoveStdArray<N>(Dst);
		return Dst;
	}

	template <size_t N>
	TFixedString<N> GetFixedString() const {
		TFixedString<N> Dst;
		MoveFixedString<N>(Dst);
		return Dst;
	}

	TStdString GetStdString() const {
		TStdString Dst;
		MoveStdString(Dst);
		return Dst;
	}

	template <size_t N>
	operator const TStdArray<N>() const {
		return GetStdArray<N>();
	}

	template <size_t N>
	operator const TFixedString<N>() const {
		return GetFixedString<N>();
	}

	operator const TStdString() const {
		return GetStdString();
	}

private:
	mutable TElem _Buf[Size];

public:
	EncryptedString() {}
	~EncryptedString() { /* std::fill_n((volatile TElem*)_Buf, Size, 0); */ }

	//Pay attention to the lifetime of the string!
	//Encrypted string constant can be used as a function parameter. 
	//ex) printf("encrypted string"e);
	//If you want to replace a string pointer without worrying about its lifetime, use the eg suffix. 
	//ex) const char* pStr = "encrypted string"eg;
	operator const TElem* () const noexcept {
		MoveArray(_Buf);
		return _Buf;
	}

	friend TElem* operator<<(TElem* Dst, const EncryptedString<Src> Str) {
		Str.MoveString(Dst);
		return Dst;
	}
};

template <fixstr::basic_fixed_string Src>
class EncryptedStringGlobal {
private:
	using TElem = std::remove_cvref_t<decltype(Src[0])>;
	constexpr static auto Size = Src.size() + 1;

	template <size_t N>
	using TFixedString = fixstr::basic_fixed_string<TElem, N>;

	template <size_t N>
	using TStdArray = std::array<TElem, N>;

	TElem _ForSizeOf[Size];
	static inline TElem* pData = 0;

public:
	static const TElem* Get() {
		if (!pData) {
			pData = (TElem*)new TStdArray<Size>;
			EncryptedData<Src._data>::Decrypt(pData);
		}
		return pData;
	}

	static void Clear() {
		std::fill_n((volatile TElem*)pData, Size, 0);
		delete[] pData;
		pData = 0;
	}

	operator const TElem* () const { return Get(); }
};

#pragma warning(disable : 4455)
#ifndef __INTELLISENSE__
template <fixstr::basic_fixed_string Src>
auto operator""e() {
	return EncryptedString<Src>();
}

template <fixstr::basic_fixed_string Src>
auto operator""eg() {
	return EncryptedStringGlobal<Src>();
}
#else
//Pay attention to the lifetime of the string!
//Encrypted string constant can be used as a function parameter. 
//ex) printf("encrypted string"e);
//If you want to replace a string pointer without worrying about its lifetime, use the eg suffix. 
//ex) const char* pStr = "encrypted string"eg;
EncryptedString<"str"> operator""e(const char*, size_t) {}
EncryptedString<L"str"> operator""e(const wchar_t*, size_t) {}
EncryptedString<u8"str"> operator""e(const char8_t*, size_t) {}
EncryptedString<u"str"> operator""e(const char16_t*, size_t) {}
EncryptedString<U"str"> operator""e(const char32_t*, size_t) {}
EncryptedStringGlobal<"str"> operator""eg(const char*, size_t) {}
EncryptedStringGlobal<L"str"> operator""eg(const wchar_t*, size_t) {}
EncryptedStringGlobal<u8"str"> operator""eg(const char8_t*, size_t) {}
EncryptedStringGlobal<u"str"> operator""eg(const char16_t*, size_t) {}
EncryptedStringGlobal<U"str"> operator""eg(const char32_t*, size_t) {}
#endif