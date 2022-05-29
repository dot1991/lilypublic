#pragma once
#include "common/util.h"

enum class VehicleType1 {
	Wheeled,
	Floating,
	None
};

enum class VehicleType2 {
	Destructible,
	Invincible
};

enum class VehicleType3 {
	Normal,
	Special
};

struct VehicleInfo {
	fixstr::basic_fixed_string<char, 0x100> Name;
	VehicleType1 Type1;
	VehicleType2 Type2;
	VehicleType3 Type3;
};

static VehicleInfo GetVehicleInfo(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT({ ""e, VehicleType1::None, VehicleType2::Destructible, VehicleType3::Normal });

		HASH_CASE("BP_EmPickup_Aircraft_C"h, { "Pickup"e, VehicleType1::None, VehicleType2::Invincible, VehicleType3::Special });
		HASH_CASE("BP_EmergencyPickupVehicle_C"h, { "Pickup"e, VehicleType1::None, VehicleType2::Invincible, VehicleType3::Special });
		HASH_CASE("TransportAircraft_Chimera_C"h, { "Aircraft"e, VehicleType1::None, VehicleType2::Invincible, VehicleType3::Special });
		HASH_CASE("BP_Bicycle_C"h, { "Bike"e, VehicleType1::None, VehicleType2::Invincible, VehicleType3::Normal });

		HASH_CASE("BP_BRDM_C"h, { "BRDM"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("Uaz_Armored_C"h, { "UAZ"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Mirado_Open_05_C"h, { "MiradoG"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_Blue_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_Green_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_Orange_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_Red_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_Motorglider_Teal_C"h, { "Glider"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });
		HASH_CASE("BP_LootTruck_C"h, { "LootTruck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Special });

		HASH_CASE("AquaRail_A_01_C"h, { "Aquarail"e, VehicleType1::Floating, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("AquaRail_A_02_C"h, { "Aquarail"e, VehicleType1::Floating, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("AquaRail_A_03_C"h, { "Aquarail"e, VehicleType1::Floating, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Boat_PG117_C"h, { "Boat"e, VehicleType1::Floating, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("PG117_A_01_C"h, { "Boat"e, VehicleType1::Floating, VehicleType2::Destructible, VehicleType3::Normal });

		HASH_CASE("BP_M_Rony_A_01_C"h, { "Rony"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_M_Rony_A_02_C"h, { "Rony"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_M_Rony_A_03_C"h, { "Rony"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_A_01_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_A_02_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_A_03_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_A_03_Esports_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_A_04_C"h, { "Mirado"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_Open_C"h, { "MiradoO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_Open_01_C"h, { "MiradoO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_Open_02_C"h, { "MiradoO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_Open_03_C"h, { "MiradoO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Mirado_Open_04_C"h, { "MiradoO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Motorbike_04_C"h, { "Motor"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Motorbike_04_Desert_C"h, { "Motor"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Motorbike_Solitario_C"h, { "Motor"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Motorbike_04_SideCar_C"h, { "MotorS"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Motorbike_04_SideCar_Desert_C"h, { "MotorS"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_01_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_02_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_03_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_04_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_05_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_06_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_07_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Niva_Esports_C"h, { "Zima"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_01_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_02_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_03_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_04_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_05_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_A_esports_C"h, { "Truck"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_B_01_C"h, { "TruckO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_B_02_C"h, { "TruckO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_B_03_C"h, { "TruckO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_B_04_C"h, { "TruckO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PickupTruck_B_05_C"h, { "TruckO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_TukTukTuk_A_01_C"h, { "Tukshai"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_TukTukTuk_A_02_C"h, { "Tukshai"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_TukTukTuk_A_03_C"h, { "Tukshai"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Van_A_01_C"h, { "Bus"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Van_A_02_C"h, { "Bus"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Van_A_03_C"h, { "Bus"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_MiniBus_C"h, { "Bus"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Scooter_01_A_C"h, { "Scooter"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Scooter_02_A_C"h, { "Scooter"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Scooter_03_A_C"h, { "Scooter"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Scooter_04_A_C"h, { "Scooter"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Snowbike_01_C"h, { "Snowmobile"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Snowbike_02_C"h, { "Snowmobile"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Snowmobile_01_C"h, { "Snowmobile"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Snowmobile_02_C"h, { "Snowmobile"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Snowmobile_03_C"h, { "Snowmobile"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_01_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_02_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_03_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_04_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_05_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Buggy_A_06_C"h, { "Buggy"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_01_v2_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_01_v2_snow_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_02_v2_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_03_v2_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_03_v2_Esports_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Dacia_A_04_v2_C"h, { "Dacia"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Uaz_A_01_C"h, { "UAZO"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Uaz_B_01_C"h, { "UAZ"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Uaz_B_01_esports_C"h, { "UAZ"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("Uaz_C_01_C"h, { "UAZ"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Dirtbike_C"h, { "DBike"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_CoupeRB_C"h, { "Coupe"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_ATV_C"h, { "Quad"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_PonyCoupe_C"h, { "Pony"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
		HASH_CASE("BP_Porter_C"h, { "Porter"e, VehicleType1::Wheeled, VehicleType2::Destructible, VehicleType3::Normal });
	}
}