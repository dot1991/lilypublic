#pragma once
#include "common/util.h"

struct PackageInfo {
	fixstr::basic_fixed_string<char, 0x100> Name;
	bool IsSmall;
};

static PackageInfo GetPackageInfo(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT({ ""e, false });

		HASH_CASE("DeathDropItemPackage_C"h, { "Box"e, false });
		HASH_CASE("Carapackage_RedBox_C"h, { "Supply"e, false });
		HASH_CASE("Carapackage_FlareGun_C"h, { "Flare"e, false });
		HASH_CASE("BP_Loot_AmmoBox_C"h, { "Ammo"e, true });
		HASH_CASE("BP_Loot_MilitaryCrate_C"h, { "Crate"e, true });
		HASH_CASE("BP_Loot_TruckHaul_C"h, { "LootTruck"e, false });
		HASH_CASE("Carapackage_SmallPackage_C"h, { "Small"e, true });
		HASH_CASE("BP_ItemPackage_GuardedLoot_Heaven_C"h, { "Pillar"e, true });
	}
}