#pragma once
#include "common/util.h"

static bool IsPlayerCharacter(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT(false);

		HASH_CASE("PlayerFemale_A_C"h, true);
		HASH_CASE("PlayerMale_A_C"h, true);
	}
}

static bool IsAICharacter(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT(false);

		HASH_CASE("AIPawn_Base_Female_C"h, true);
		HASH_CASE("AIPawn_Base_Male_C"h, true);
		HASH_CASE("UltAIPawn_Base_Female_C"h, true);
		HASH_CASE("UltAIPawn_Base_Male_C"h, true);
		HASH_CASE("AIPawn_Base_Male_Pillar_C"h, true);
	}
}