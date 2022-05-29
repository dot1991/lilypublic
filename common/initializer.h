#pragma once
#define CONCAT_DIRECT(x, y) x##y
#define CONCAT(x, y) CONCAT_DIRECT(x, y)

//Don't use __COUNTER__
#define INITIALIZER_GLOBAL(UniqueLabel) \
struct CONCAT(____InitializerClass, UniqueLabel){\
	inline CONCAT(____InitializerClass, UniqueLabel)(auto f) { f(); }\
}inline CONCAT(____InitializerMember, UniqueLabel) = []()

//Don't use __COUNTER__
#define INITIALIZER_INCLASS(UniqueLabel) \
struct CONCAT(____InitializerClass, UniqueLabel){\
	inline CONCAT(____InitializerClass, UniqueLabel)(auto f) { f(); }\
}static inline CONCAT(____InitializerMember, UniqueLabel) = []()