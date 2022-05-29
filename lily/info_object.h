#pragma once
#include "common/util.h"

struct ObjectInfo {
	fixstr::basic_fixed_string<char, 0x100> Name;
	bool IsLong = false;
	bool bFrendly = false;
};

static ObjectInfo GetObjectInfo(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT({ ""e, false });

		HASH_CASE("ProjGrenade_C"h, { "Grenade"e });
		HASH_CASE("ProjFlashBang_C"h, { "Flash"e });
		HASH_CASE("ProjMolotov_C"h, { "Molotov"e });
		HASH_CASE("ProjSmokeBomb_v2_C"h, { "Smoke"e });
		HASH_CASE("ProjSpiketrap_C"h, { "Spike"e });
		HASH_CASE("BP_Spiketrap_C"h, { "Spike"e });
		HASH_CASE("ProjStickyGrenade_C"h, { "Bomb"e });
		HASH_CASE("ProjC4_C"h, { "C4"e });
		HASH_CASE("ProjDecoyGrenade_C"h, { "Decoy"e });
		HASH_CASE("BP_Projectile_40mm_Smoke_C"h, { "Smoke"e });
		HASH_CASE("ProjBluezoneBomb_C"h, { "BlueBomb"e });
		HASH_CASE("BP_Drone_C"h, { "Drone"e });
		HASH_CASE("WeapMortar_C"h, { "Mortar"e });

		HASH_CASE("WeapMacheteProjectile_C"h, { "Machete"e });
		HASH_CASE("WeapCowbarProjectile_C"h, { "Cowbar"e });
		HASH_CASE("WeapPanProjectile_C"h, { "Pan"e });
		HASH_CASE("WeapSickleProjectile_C"h, { "Sickle"e });

		HASH_CASE("PanzerFaust100M_Projectile_C"h, { "Rocket"e, true });
		HASH_CASE("Mortar_Projectile_C"h, { "Shell"e, true });
		HASH_CASE("Flare_Projectile_C"h, { "Flare"e, true });

		HASH_CASE("BP_KillTruck_C"h, { "KillTruck"e, true });
		HASH_CASE("BP_Helicopter_C"h, { "Helicopter"e, true });

		HASH_CASE("BP_Vending_machine_1_C"h, { "Vendor"e, false, true });
	}
}