#pragma once
#include "common/util.h"

#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <fstream>
#include <iostream>
#include <array>

#include "common/process.h"

//working pubg versions
//14.2.6.7
//15.1.4.9
//15.1.6.7
#define szPatternAllClass "48 8d 05 ? ? ? ? 48 ? ? ? ? 4c 8d 0d ? ? ? ? 4c 8d 05 ? ? ? ? 48 8d 15 ? ? ? ? 48 8d 0d ? ? ? ? e8"e
#define szPatternAllStruct "4c 8d 05 ? ? ? ? 48 ? ? 48 8d 0d ? ? ? ? e8 ? ? ? ? 48 89 05"e
//#define szPatternFuncOffset "4c 8d ? ? ? 00 00 48 ? ? 08 49 ? ? eb"e

enum class Type {
	Member,
	Function,
	Enum
};

//e8 ? ? 00 00 48 8b d0 f7 83 ? ? 00 00 00 04 00 00
inline DWORD UObjectFuncOffset = 0x120;

static auto GetStructMap(Process& process) {
	std::map<std::wstring, uintptr_t> StructMap;

	for (uintptr_t ScanAddress = process.GetBaseAddress(); true; ScanAddress++) {
		uintptr_t RemainSize = process.GetSizeOfImage() - (ScanAddress - process.GetBaseAddress());
		ScanAddress = process.ScanRange(ScanAddress, RemainSize, szPatternAllStruct);
		if (!ScanAddress)
			break;

		int IntValue;
		if (!process.Read(ScanAddress + 0x3, &IntValue)) {
			dprintf("struct1");
			continue;
		}

		uintptr_t NamePtr = (INT64)ScanAddress + 0x3 + 0x4 + IntValue;

		std::array<wchar_t, 0x100> wStructName;
		if (!process.Read(NamePtr, &wStructName)) {
			dprintf("struct2");
			continue;
		}

		if (!process.Read(ScanAddress + 0xd, &IntValue)) {
			dprintf("struct3");
			continue;
		}

		uintptr_t FuncStartAddress = (INT64)ScanAddress + 0xd + 0x4 + IntValue;

		StructMap[wStructName.data()] = FuncStartAddress;
	}

	return StructMap;
}

static auto GetClassMap(Process& process) {
	std::map<std::wstring, uintptr_t> ClassMap;

	for (uintptr_t ScanAddress = process.GetBaseAddress(); true; ScanAddress++) {
		uintptr_t RemainSize = process.GetSizeOfImage() - (ScanAddress - process.GetBaseAddress());
		ScanAddress = process.ScanRange(ScanAddress, RemainSize, szPatternAllClass);
		if (!ScanAddress)
			break;

		int value;
		if (!process.Read(ScanAddress + 0x3, &value)) {
			dprintf("class1");
			continue;
		}

		uintptr_t pClassName = (INT64)ScanAddress + 0x3 + 0x4 + value;

		std::array<wchar_t, 0x100> wClassName;
		if (!process.Read(pClassName, &wClassName)) {
			dprintf("class2");
			continue;
		}

		if (!process.Read(ScanAddress + 0x1d, &value)) {
			dprintf("class3");
			continue;
		}

		uintptr_t pFuncAddress = (INT64)ScanAddress + 0x1d + 0x4 + value;

		ClassMap[wClassName.data()] = pFuncAddress;
		ScanAddress++;
	}

	return ClassMap;
}

static auto GetMemberMap(Process& process, uintptr_t StartAddress) {
	std::map<std::wstring, std::pair<Type, uintptr_t>> MemberMap;

	for (uintptr_t CurrentAddress = StartAddress; true; CurrentAddress++) {
		DWORD dwOpCode;
		if (!process.Read(CurrentAddress, &dwOpCode))
			break;

		//Detect function epilog
		if (dwOpCode == 0xE8CC3348)
			break;

		int IntValue;
		uintptr_t ScanResult;

		//Detect enum
		ScanResult = process.ScanRange(CurrentAddress, 0x20, "48 8d 15 ? ? ? ? 48 8b ? ? ? ? ? ? e8 ? ? ? ? c7 84");
		if (ScanResult == CurrentAddress) {
			do {
				if (!process.Read(ScanResult + 0x3, &IntValue)) {
					dprintf("enum1");
					break;
				}

				uintptr_t pString = INT64(ScanResult) + 0x3 + 0x4 + IntValue;

				std::array<wchar_t, 0x100> wString;
				if (!process.Read(pString, &wString)) {
					dprintf("enum2");
					break;
				}

				DWORD dwEnum;
				if (!process.Read(ScanResult + 0x1B, &dwEnum)) {
					dprintf("enum3");
					break;
				}

				MemberMap[wString.data()] = {Type::Enum, dwEnum - 1};
			} while (0);

			continue;
		}

		//Detect member or function
		ScanResult = process.ScanRange(CurrentAddress, 0x20, "41 b8 01 00 00 00 48 8d 15");
		if (ScanResult == CurrentAddress) {
			do {
				if (!process.Read(ScanResult + 0x9, &IntValue)) {
					dprintf("s1");
					break;
				}

				uintptr_t pString = INT64(ScanResult) + 0x9 + 0x4 + IntValue;

				std::array<char, 0x100> szString;
				if (!process.Read(pString, &szString)) {
					dprintf("s2");
					break;
				}

				if (szString[1] == 0) {
					//wchar detected -> this is member
					std::array<wchar_t, 0x100> wString;
					memcpy(wString.data(), szString.data(), sizeof(szString));

					do {
						uintptr_t NextScanResult = process.ScanRange(ScanResult + 1, 0x100, "41 b8 01 00 00 00 48 8d 15");
						if (!NextScanResult)
							NextScanResult = ScanResult + 0x100;


						uintptr_t BitFieldScanResult = process.ScanRange(ScanResult - 0x100, 0x100, "e8 ? ? ? ? 48 8d 05 ? ? ? ? 48 89");
						if (BitFieldScanResult && BitFieldScanResult < NextScanResult) {
							if (!process.Read(BitFieldScanResult + 0x8, &IntValue)) {
								dprintf("BitField0 1");
								break;
							}

							uintptr_t FuncAddress = INT64(BitFieldScanResult) + 0x8 + 0x4 + IntValue;
							if (!process.Read(FuncAddress, &FuncAddress)) {
								dprintf("BitField0 2");
								break;
							}

							uintptr_t Result = process.ScanRange(FuncAddress, 0x60, "C3");
							if (!Result) {
								dprintf("BitField0 3");
								break;
							}

							BYTE ByteOffset;
							if (!process.Read(Result - 0x1, &ByteOffset)) {
								dprintf("BitField0 4");
								break;
							}

							DWORD DwordOffset;
							if (!process.Read(Result - 0x4, &DwordOffset)) {
								dprintf("BitField0 5");
								break;
							}

							DWORD Offset = ByteOffset > 0 ? ByteOffset : DwordOffset;

							MemberMap[wString.data()] = {Type::Member, Offset};
							//MemberMap[std::wstring(wString) + L"_Bit"] = { Type::Member, Offset };
							break;
						}

						BitFieldScanResult = process.ScanRange(ScanResult - 0x100, 0x100, "e8 ? ? ? ? 48 8d ? ? ? ? 00 00 e8");
						if (BitFieldScanResult && BitFieldScanResult < NextScanResult) {
							if (!process.Read(BitFieldScanResult + 0xe, &IntValue)) {
								dprintf("BitField1 1");
								break;
							}

							uintptr_t FuncAddress = INT64(BitFieldScanResult) + 0xe + 0x4 + IntValue;
							uintptr_t Result = process.ScanRange(FuncAddress, 0x30, "48 8d ? ? ? ? ? 48");
							if (!Result) {
								dprintf("BitField1 3");
								break;
							}

							if (!process.Read(Result + 0x3, &IntValue)) {
								dprintf("BitField1 4");
								break;
							}

							Result = INT64(Result) + 0x7 + IntValue;
							if (!process.Read(Result, &Result)) {
								dprintf("BitField1 5");
								break;
							}

							Result = process.ScanRange(Result, 0x60, "C3");
							if (!Result) {
								dprintf("BitField1 6");
								break;
							}

							BYTE ByteOffset;
							if (!process.Read(Result - 0x1, &ByteOffset)) {
								dprintf("BitField1 7");
								break;
							}

							DWORD DwordOffset;
							if (!process.Read(Result - 0x4, &DwordOffset)) {
								dprintf("BitField1 8");
								break;
							}

							DWORD Offset = ByteOffset > 0 ? ByteOffset : DwordOffset;

							MemberMap[wString.data()] = { Type::Member, Offset };
							//MemberMap[std::wstring(wString) + L"_Bit"] = { Type::Member, Offset };
							break;
						}

						uintptr_t MemberScanResult = process.ScanRange(ScanResult, 0xC0, "41 b9 ? ? 00 00 45 33 c0 48 8b");
						if (MemberScanResult && MemberScanResult < NextScanResult) {
							DWORD Offset;
							if (!process.Read(MemberScanResult + 0x2, &Offset)) {
								dprintf("Member 1");
								break;
							}

							if (!MemberMap.contains(wString.data()))
								MemberMap[wString.data()] = { Type::Member, Offset };

							break;
						}

						uintptr_t MemberZeroOffsetScanResult = process.ScanRange(ScanResult, 0xC0, "45 33 c9 45 33 c0 48 8b");
						if (MemberZeroOffsetScanResult && MemberZeroOffsetScanResult < NextScanResult) {
							if (std::wstring(wString.data()).find(L"_Key") == std::wstring::npos &&
								std::wstring(wString.data()) != L"UnderlyingType") {

								if (!MemberMap.contains(wString.data()))
									MemberMap[wString.data()] = { Type::Member, 0 };
							}

							break;
						}

					} while (0);
				}
				else {
					//char detected  -> this is function

					uintptr_t Result = process.ScanRange(ScanResult, 0x40, "e8 ? ? ? ? 48 8b");
					if (!Result) {
						dprintf("Function 1");
						break;
					}

					if (!process.Read(Result + 1, &IntValue)) {
						dprintf("Function 2");
						break;
					}

					uintptr_t FuncAddress = (INT64)Result + 0x5 + IntValue;

					Result = process.ScanRange(FuncAddress, 0x60, "48 83 3d ? ? ? ? 00 0f 85");
					if (!Result) {
						dprintf("Function 3");
						break;
					}

					if (!process.Read(Result + 0x3, &IntValue)) {
						dprintf("Function 4");
						break;
					}

					uintptr_t pObject = (INT64)Result + 0x8 + IntValue;

					if (!process.Read(pObject, &pObject)) {
						dprintf("Function 5");
						break;
					}

					uintptr_t MemberFuncAddress;
					if (!process.Read(pObject + UObjectFuncOffset, &MemberFuncAddress)) {
						dprintf("Function 6");
						break;
					}

					MemberFuncAddress -= process.GetBaseAddress();

					std::string stdString = szString.data();
					std::wstring wString(stdString.begin(), stdString.end());
					//wString.append(stdString.begin(), stdString.end());

					MemberMap[wString] = { Type::Function, MemberFuncAddress };
				}
			} while (0);

			continue;
		}
	}

	return MemberMap;
}

static void DumpAll(Process& process) {
	//uintptr_t ScanResult = process.ScanCurrentModule(szPatternFuncOffset);
	//if (!ScanResult || !process.Read(ScanResult + 0x3, &UObjectFuncOffset)) {
	//	dprintf("UObjectFuncOffset not found");
	//	return;
	//}

	std::map<std::wstring, uintptr_t> ClassMap = GetClassMap(process);
	std::map<std::wstring, uintptr_t> StructMap = GetStructMap(process);

	std::wofstream outfile("out.txt");
	if (!outfile.is_open()) {
		dprintf("Outfile not created");
		return;
	}

	for (const auto& Element : ClassMap) {
		auto MemberMap = GetMemberMap(process, Element.second);

		std::wcout << "class " << Element.first << std::endl;

		outfile << Element.first << " {\n";

		std::map<uintptr_t, std::pair<Type, std::wstring>> OrderedMap;

		for (const auto& Member : MemberMap)
			OrderedMap[Member.second.second] = { Member.second.first , Member.first };

		for (const auto& Member : OrderedMap) {
			switch (Member.second.first) {
			case Type::Member:
				outfile << "Offset\t\t0x" << std::hex << Member.first << "\t\t" << Member.second.second.c_str() << "\n";
				break;
			case Type::Function:
				outfile << "Address\t\t0x" << std::hex << Member.first << "\t" << Member.second.second.c_str() << "\n";
				break;
			case Type::Enum:
				outfile << "Enum\t\t0x" << std::hex << Member.first << "\t\t" << Member.second.second.c_str() << "\n";
				break;
			}
		}
		outfile << "}\n\n";
	}

	for (const auto& Element : StructMap) {
		auto MemberMap = GetMemberMap(process, Element.second);

		std::wcout << "struct " << Element.first << std::endl;

		outfile << Element.first << " {\n";

		std::map<uintptr_t, std::pair<Type, std::wstring>> OrderedMap;

		for (const auto& Member : MemberMap)
			OrderedMap[Member.second.second] = { Member.second.first , Member.first };

		for (const auto& Member : OrderedMap) {
			switch (Member.second.first) {
			case Type::Member:
				outfile << "Offset\t\t0x" << std::hex << Member.first << "\t\t" << Member.second.second.c_str() << "\n";
				break;
			case Type::Function:
				outfile << "Address\t\t0x" << std::hex << Member.first << "\t" << Member.second.second.c_str() << "\n";
				break;
			case Type::Enum:
				outfile << "Enum\t\t0x" << std::hex << Member.first << "\t\t" << Member.second.second.c_str() << "\n";
				break;
			}
		}
		outfile << "}\n\n";
	}
}

static bool Dump(Process& process) {
	//uintptr_t ScanResult = process.ScanCurrentModule(szPatternFuncOffset);
	//if (!ScanResult || !process.Read(ScanResult + 0x3, &UObjectFuncOffset)) {
	//	dprintf("UObjectFuncOffset not found");
	//	return;
	//}

	std::map<std::wstring, uintptr_t> ClassMap = GetClassMap(process);
	std::map<std::wstring, uintptr_t> StructMap = GetStructMap(process);

	std::wifstream infile("pubg_class_input.h");
	verify(infile.is_open());

	std::wofstream outfile("pubg_class_output.h");
	verify(outfile.is_open());

	std::wstring wClass;
	std::wstring wLine;

	std::map<std::wstring, std::pair<Type, uintptr_t>> MemberMap;


	while (std::getline(infile, wLine)) {
		if (wLine.size() == 0) {
			outfile << std::endl;
			std::wcout << std::endl;
			continue;
		}

		std::wstringstream ss(wLine);

		std::wstring wToken;
		std::getline(ss, wToken, L' ');

		if (wToken == L"struct") {
			std::getline(ss, wClass);
			wClass = trim(wClass);

			if (!wClass.size()) {
				dprintf("no struct in file");
				return false;
			}

			auto i = StructMap.find(wClass);
			if (i == StructMap.end()) {
				dprintf("no struct in game memory : %ws", wClass.c_str());
				return false;
			}

			MemberMap = GetMemberMap(process, i->second);
			outfile << L"struct F" << wClass << std::endl;
			std::wcout << L"struct F" << wClass << std::endl;
			continue;
		}
		else if (wToken == L"class") {
			std::getline(ss, wClass);
			wClass = trim(wClass);

			if (!wClass.size()) {
				dprintf("no class in file");
				return false;
			}

			auto i = ClassMap.find(wClass);
			if (i == ClassMap.end()) {
				dprintf("no class in game memory : %ws", wClass.c_str());
				return false;
			}

			MemberMap = GetMemberMap(process, i->second);
			outfile << L"class " << wClass << std::endl;
			std::wcout << L"class " << wClass << std::endl;
			continue;
		}
		else if (wToken == L"function") {
			//implement
			std::wstring wFunc;
			std::getline(ss, wFunc);

			wFunc = trim(wFunc);

			auto data = MemberMap.find(wFunc);

			if (data == MemberMap.end()) {
				dprintf("no function in class : %ws.%ws", wClass.c_str(), wFunc.c_str());
				return false;
			}

			if (data->second.first != Type::Function) {
				dprintf("type is not function : %ws.%ws", wClass.c_str(), wFunc.c_str());
				return false;
			}

			std::wstring wOutName = wFunc;
			std::wstring wOutOffset = to_hex_string(data->second.second);

			outfile << L"//function " << wClass.c_str() << "." << wOutName.c_str() << L" " << wOutOffset.c_str() << std::endl;
			std::wcout << L"//function " << wClass.c_str() << "." << wOutName.c_str() << L" " << wOutOffset.c_str() << std::endl;
			continue;
		}
		else {
			std::wstring wType = wToken;
			std::wstring wRemain;
			std::getline(ss, wRemain);
			std::wstringstream ssRemain(wRemain);

			//CameraCache + CameraCacheEntry.POV + MinimalViewInfo.Fov

			std::wstring wOutName;
			std::wstring wOutOffset;

			while (std::getline(ssRemain, wToken, L'+')) {
				size_t pos = wToken.find('.');

				if (pos != std::string::npos) {
					auto wStruct = trim(wToken.substr(0, pos));
					auto wMember = trim(wToken.substr(pos + 1));

					if (!wStruct.size()) {
						dprintf("no struct in file : %ws", wStruct.c_str());
						return false;
					}

					auto i = StructMap.find(wStruct);
					if (i == StructMap.end()) {
						dprintf("no struct in game memory : %ws", wStruct.c_str());
						return false;
					}

					auto StructMemberMap = GetMemberMap(process, i->second);
					auto data = StructMemberMap.find(wMember);

					if (data == StructMemberMap.end()) {
						dprintf("no member in struct : %ws.%ws", wStruct.c_str(), wMember.c_str());
						return false;
					}

					if (data->second.first != Type::Member) {
						dprintf("type is not member : %ws.%ws", wStruct.c_str(), wMember.c_str());
						return false;
					}

					wOutName += L"_" + wMember;
					wOutOffset += L" + " + to_hex_string(data->second.second);
				}
				else {
					auto wMember = trim(wToken);
					auto data = MemberMap.find(wMember);

					if (data == MemberMap.end()) {
						dprintf("no member in class : %ws.%ws", wClass.c_str(), wMember.c_str());
						return false;
					}

					if (data->second.first != Type::Member) {
						dprintf("type is not member : %ws.%ws", wClass.c_str(), wMember.c_str());
						return false;
					}

					wOutName = wMember;
					wOutOffset = to_hex_string(data->second.second);
				}

			}

			if (wOutOffset == L"0x0") {
				outfile << L"MemberAtOffsetZero(" << wType.c_str() << L", " << wOutName.c_str() << L", " << wOutOffset.c_str() << L")" << std::endl;
				std::wcout << L"MemberAtOffsetZero(" << wType.c_str() << L", " << wOutName.c_str() << L", " << wOutOffset.c_str() << L")" << std::endl;
			}
			else {
				outfile << L"MemberAtOffset(" << wType.c_str() << L", " << wOutName.c_str() << L", " << wOutOffset.c_str() << L")" << std::endl;
				std::wcout << L"MemberAtOffset(" << wType.c_str() << L", " << wOutName.c_str() << L", " << wOutOffset.c_str() << L")" << std::endl;
			}
		}
	}

	return true;
}