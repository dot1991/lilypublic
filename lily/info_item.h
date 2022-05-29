#pragma once
#include "common/util.h"

struct ItemInfo {
	fixstr::basic_fixed_string<char, 0x100> Name;
	int ItemPriority;
};

static ItemInfo GetItemInfo(unsigned Hash) {
	switch (Hash) {
		HASH_DEFAULT({ ""e, 0 });

		//Ammo
		HASH_CASE("Item_Ammo_Mortar_C"h, { "Shell"e, 4 });
		HASH_CASE("Item_Ammo_Bolt_C"h, { "Bolt"e, 4 });
		HASH_CASE("Item_Ammo_Flare_C"h, { "Flare"e, 4 });
		HASH_CASE("Item_Ammo_57mm_C"h, { "5.7"e, 4 });
		HASH_CASE("Item_Ammo_300Magnum_C"h, { "300"e, 4 });
		HASH_CASE("Item_Ammo_556mm_C"h, { "5"e, 3 });
		HASH_CASE("Item_Ammo_762mm_C"h, { "7"e, 3 });
		HASH_CASE("Item_Ammo_40mm_C"h, { "40"e, 3 });
		HASH_CASE("Item_Ammo_9mm_C"h, { "9"e, 2 });
		HASH_CASE("Item_Ammo_12Guage_C"h, { "12"e, 2 });
		HASH_CASE("Item_Ammo_45ACP_C"h, { "45"e, 2 });

		//Attach
		HASH_CASE("Item_Attach_Weapon_Lower_Foregrip_C"h, { "L_Fore"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Lower_AngledForeGrip_C"h, { "L_Angle"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Lower_HalfGrip_C"h, { "L_Half"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Lower_LightweightForeGrip_C"h, { "L_Light"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Lower_ThumbGrip_C"h, { "L_Thumb"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Lower_LaserPointer_C"h, { "L_Laser"e, 1 });

		HASH_CASE("Item_Attach_Weapon_Lower_QuickDraw_Large_Crossbow_C"h, { "Q_CBow"e, 3 });

		HASH_CASE("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Large_C"h, { "EQ_AR"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Magazine_Extended_Large_C"h, { "E_AR"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Magazine_QuickDraw_Large_C"h, { "Q_AR"e, 2 });

		HASH_CASE("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_SniperRifle_C"h, { "EQ_SR"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Magazine_Extended_SniperRifle_C"h, { "E_SR"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Magazine_QuickDraw_SniperRifle_C"h, { "Q_SR"e, 2 });

		HASH_CASE("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Medium_C"h, { "EQ_SMG"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Magazine_Extended_Medium_C"h, { "E_SMG"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Magazine_QuickDraw_Medium_C"h, { "Q_SMG"e, 1 });
		
		HASH_CASE("Item_Attach_Weapon_Muzzle_Compensator_Large_C"h, { "C_AR"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_Compensator_Medium_C"h, { "C_SMG"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_Compensator_SniperRifle_C"h, { "C_SR"e, 2 });

		HASH_CASE("Item_Attach_Weapon_Muzzle_FlashHider_Large_C"h, { "F_AR"e, 1 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_FlashHider_Medium_C"h, { "F_SMG"e, 1 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_FlashHider_SniperRifle_C"h, { "F_SR"e, 1 });

		HASH_CASE("Item_Attach_Weapon_Muzzle_Suppressor_SniperRifle_C"h, { "S_SR"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_Suppressor_Large_C"h, { "S_AR"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_Suppressor_Medium_C"h, { "S_SMG"e, 3 });

		HASH_CASE("Item_Attach_Weapon_Muzzle_Choke_C"h, { "Choke"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Muzzle_Duckbill_C"h, { "Duckbill"e, 2 });

		HASH_CASE("Item_Attach_Weapon_SideRail_DotSight_RMR_C"h, { "Canted"e, 4 });

		HASH_CASE("Item_Attach_Weapon_Stock_AR_Composite_C"h, { "ST_AR"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Stock_SniperRifle_CheekPad_C"h, { "ST_SR"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Stock_UZI_C"h, { "ST_UZI"e, 2 });
		HASH_CASE("Item_Attach_Weapon_Stock_SniperRifle_BulletLoops_C"h, { "Belt"e, 2 });
		
		HASH_CASE("Item_Attach_Weapon_Upper_PM2_01_C"h, { "15X"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Upper_CQBSS_C"h, { "8X"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Upper_Scope6x_C"h, { "6X"e, 4 });
		HASH_CASE("Item_Attach_Weapon_Upper_ACOG_01_C"h, { "4X"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Upper_Scope3x_C"h, { "3X"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Upper_Aimpoint_C"h, { "2X"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Upper_DotSight_01_C"h, { "U_Dot"e, 3 });
		HASH_CASE("Item_Attach_Weapon_Upper_Holosight_C"h, { "U_Holo"e, 3 });

		HASH_CASE("Item_Back_BackupParachute_C"h, { "Parachute"e, 3 });
		HASH_CASE("Item_Armor_C_01_Lv3_C"h, { "Vest3"e, 4 });
		HASH_CASE("Item_Armor_D_01_Lv2_C"h, { "Vest2"e, 3 });
		HASH_CASE("Item_Armor_E_01_Lv1_C"h, { "Vest1"e, 2 });
		HASH_CASE("Item_Back_B_08_Lv3_C"h, { "Back3"e, 4 });
		HASH_CASE("Item_Back_C_01_Lv3_C"h, { "Back3"e, 4 });
		HASH_CASE("Item_Back_C_02_Lv3_C"h, { "Back3"e, 4 });
		HASH_CASE("Item_Back_F_01_Lv2_C"h, { "Back2"e, 3 });
		HASH_CASE("Item_Back_F_02_Lv2_C"h, { "Back2"e, 3 });
		HASH_CASE("Item_Back_E_01_Lv1_C"h, { "Back1"e, 2 });
		HASH_CASE("Item_Back_E_02_Lv1_C"h, { "Back1"e, 2 });
		HASH_CASE("Item_Back_BlueBlocker"h, { "JammerPack"e, 4 });
		HASH_CASE("Item_Ghillie_01_C"h, { "GhillieG"e, 4 });
		HASH_CASE("Item_Ghillie_02_C"h, { "GhillieB"e, 4 });
		HASH_CASE("Item_Ghillie_03_C"h, { "GhillieB"e, 4 });
		HASH_CASE("Item_Ghillie_04_C"h, { "GhillieW"e, 4 });
		HASH_CASE("Item_Ghillie_05_C"h, { "CamoSuit"e, 4 });
		HASH_CASE("Item_Ghillie_06_C"h, { "GhillieD"e, 4 });
		HASH_CASE("Item_Head_E_01_Lv1_C"h, { "Head1"e, 2 });
		HASH_CASE("Item_Head_E_02_Lv1_C"h, { "Head1"e, 2 });
		HASH_CASE("Item_Head_F_01_Lv2_C"h, { "Head2"e, 3 });
		HASH_CASE("Item_Head_F_02_Lv2_C"h, { "Head2"e, 3 });
		HASH_CASE("Item_Head_G_01_Lv3_C"h, { "Head3"e, 4 });

		//Throw
		HASH_CASE("Item_Weapon_C4_C"h, { "C4"e, 4 });
		HASH_CASE("Item_Weapon_BluezoneGrenade_C"h, { "BlueBomb"e, 3 });
		HASH_CASE("Item_Weapon_Grenade_C"h, { "Grenade"e, 3 });
		HASH_CASE("Item_Weapon_Grenade_Warmode_C"h, { "Grenade"e, 3 });
		HASH_CASE("Item_Weapon_FlashBang_C"h, { "Flash"e, 3 });
		HASH_CASE("Item_Weapon_StickyGrenade_C"h, { "Bomb"e, 3 });
		HASH_CASE("Item_Weapon_Molotov_C"h, { "Molotov"e, 3 });
		HASH_CASE("Item_Weapon_SmokeBomb_C"h, { "Smoke"e, 3 });
		HASH_CASE("Item_Weapon_SpikeTrap_C"h, { "Trap"e, 3 });
		HASH_CASE("Item_Weapon_DecoyGrenade_C"h, { "Decoy"e, 1 });

		//Melee
		HASH_CASE("Item_Weapon_Pan_C"h, { "Pan"e, 3 });
		HASH_CASE("Item_Weapon_Cowbar_C"h, { "Crawbar"e, 1 });
		HASH_CASE("Item_Weapon_Machete_C"h, { "Machete"e, 1 });
		HASH_CASE("Item_Weapon_Sickle_C"h, { "Sickle"e, 1 });

		//AR
		HASH_CASE("Item_Weapon_Groza_C"h, { "Groza"e, 4 });
		HASH_CASE("Item_Weapon_BerylM762_C"h, { "Beryl"e, 4 });
		HASH_CASE("Item_Weapon_ACE32_C"h, { "ACE"e, 4 });
		HASH_CASE("Item_Weapon_HK416_C"h, { "M4"e, 4 });
		HASH_CASE("Item_Weapon_Duncans_M416_C"h, { "M4"e, 4 });
		HASH_CASE("Item_Weapon_AUG_C"h, { "AUG"e, 4 });
		HASH_CASE("Item_Weapon_AK47_C"h, { "AK"e, 3 });
		HASH_CASE("Item_Weapon_Lunchmeats_AK47_C"h, { "AK"e, 3 });
		HASH_CASE("Item_Weapon_SCAR-L_C"h, { "SCAR"e, 3 });
		HASH_CASE("Item_Weapon_G36C_C"h, { "G36C"e, 3 });
		HASH_CASE("Item_Weapon_QBZ95_C"h, { "QBZ"e, 3 });
		HASH_CASE("Item_Weapon_K2_C"h, { "K2"e, 3 });
		HASH_CASE("Item_Weapon_Mk47Mutant_C"h, { "Mutant"e, 2 });
		HASH_CASE("Item_Weapon_M16A4_C"h, { "M16"e, 2 });

		//LMG
		HASH_CASE("Item_Weapon_MG3_C"h, { "MG3"e, 4 });
		HASH_CASE("Item_Weapon_DP28_C"h, { "DP28"e, 2 });
		HASH_CASE("Item_Weapon_M249_C"h, { "M249"e, 2 });

		//SR
		HASH_CASE("Item_Weapon_L6_C"h, { "Lynx"e, 4 });
		HASH_CASE("Item_Weapon_AWM_C"h, { "AWM"e, 4 });
		HASH_CASE("Item_Weapon_M24_C"h, { "M24"e, 3 });
		HASH_CASE("Item_Weapon_Julies_Kar98k_C"h, { "K98"e, 3 });
		HASH_CASE("Item_Weapon_Kar98k_C"h, { "K98"e, 3 });
		HASH_CASE("Item_Weapon_Mosin_C"h, { "Mosin"e, 3 });
		HASH_CASE("Item_Weapon_Win1894_C"h, { "Win94"e, 1 });

		//DMR
		HASH_CASE("Item_Weapon_Mk14_C"h, { "Mk14"e, 4 });
		HASH_CASE("Item_Weapon_FNFal_C"h, { "SLR"e, 4 });
		HASH_CASE("Item_Weapon_Mk12_C"h, { "Mk12"e, 4 });
		HASH_CASE("Item_Weapon_SKS_C"h, { "SKS"e, 3 });
		HASH_CASE("Item_Weapon_Mads_QBU88_C"h, { "QBU"e, 3 });
		HASH_CASE("Item_Weapon_QBU88_C"h, { "QBU"e, 3 });
		HASH_CASE("Item_Weapon_Mini14_C"h, { "Mini"e, 3 });
		HASH_CASE("Item_Weapon_VSS_C"h, { "VSS"e, 3 });
		
		//SG
		HASH_CASE("Item_Weapon_DP12_C"h, { "DBS"e, 4 });
		HASH_CASE("Item_Weapon_Saiga12_C"h, { "S12K"e, 3 });
		HASH_CASE("Item_Weapon_Winchester_C"h, { "S1897"e, 2 });
		HASH_CASE("Item_Weapon_Berreta686_C"h, { "S686"e, 2 });

		//Pistol
		HASH_CASE("Item_Weapon_G18_C"h, { "P18C"e, 4 });
		HASH_CASE("Item_Weapon_DesertEagle_C"h, { "Deagle"e, 3 });
		HASH_CASE("Item_Weapon_Rhino_C"h, { "R45"e, 3 });
		HASH_CASE("Item_Weapon_NagantM1895_C"h, { "R1895"e, 3 });
		HASH_CASE("Item_Weapon_vz61Skorpion_C"h, { "Skorpion"e, 3 });
		HASH_CASE("Item_Weapon_M1911_C"h, { "P1911"e, 2 });
		HASH_CASE("Item_Weapon_M9_C"h, { "P92"e, 2 });
		HASH_CASE("Item_Weapon_Sawnoff_C"h, { "Sawnoff"e, 1 });

		//SMG
		HASH_CASE("Item_Weapon_P90_C"h, { "P90"e, 4 });
		HASH_CASE("Item_Weapon_Vector_C"h, { "Vec"e, 3 });
		HASH_CASE("Item_Weapon_UZI_C"h, { "UZI"e, 3 });
		HASH_CASE("Item_Weapon_UMP_C"h, { "UMP"e, 3 });
		HASH_CASE("Item_Weapon_Thompson_C"h, { "Tom"e, 2 });
		HASH_CASE("Item_Weapon_BizonPP19_C"h, { "Bizon"e, 2 });
		HASH_CASE("Item_Weapon_MP5K_C"h, { "MP5K"e, 2 });

		//Special Weapon
		HASH_CASE("Item_Weapon_Mortar_C"h, { "Mortar"e, 4 });
		HASH_CASE("Item_Weapon_Crossbow_C"h, { "CBow"e, 4 });
		HASH_CASE("Item_Weapon_FlareGun_C"h, { "FlareGun"e, 4 });
		HASH_CASE("Item_Weapon_M79_C"h, { "M79"e, 4 });
		HASH_CASE("Item_Weapon_PanzerFaust100M_C"h, { "Rocket"e, 4 });

		//Boost
		HASH_CASE("Item_Boost_AdrenalineSyringe_C"h, { "Syringe"e, 3 });
		HASH_CASE("Item_Boost_EnergyDrink_C"h, { "Drink"e, 3 });
		HASH_CASE("Item_Boost_PainKiller_C"h, { "Drug"e, 3 });

		//Heal
		HASH_CASE("Item_Heal_MedKit_C"h, { "MedKit"e, 4 });
		HASH_CASE("Item_Heal_FirstAid_C"h, { "FirstAid"e, 3 });
		HASH_CASE("Item_Heal_Bandage_C"h, { "Band"e, 2 });

		//Key
		HASH_CASE("Item_Heaven_Key_C"h, { "Key"e, 4 });
		HASH_CASE("Item_Chimera_Key_C"h, { "Key"e, 4 });
		HASH_CASE("Item_Tiger_Key_C"h, { "Key"e, 4 });

		HASH_CASE("Item_JerryCan_C"h, { "Fuel"e, 4 });
		HASH_CASE("Item_EmergencyPickup_C"h, { "Pickup"e, 4 });
		HASH_CASE("InstantRevivalKit_C"h, { "Revival"e, 4 });
		HASH_CASE("Item_Tiger_SelfRevive_C"h, { "AED"e, 4 });
		HASH_CASE("Item_Mountainbike_C"h, { "Bike"e, 4 });
		HASH_CASE("Item_Weapon_Drone_C"h, { "Drone"e, 4 });
		HASH_CASE("Item_Weapon_TraumaBag_C"h, { "TraumaBag"e, 4 });
		HASH_CASE("Item_Weapon_Spotter_Scope_C"h, { "Spotter"e, 4 });
		HASH_CASE("Item_Weapon_TacPack_C"h, { "TacPack"e, 4 });
	}
}