#include "hack.h"

FILE* Hack::OpenDesktopFile(const char* szFileName, const char* Mode) {
	char szBuf[0x100];
	GetDesktopPath(szBuf, szFileName);
	return fopen(szBuf, Mode);
}

void Hack::LoadList(std::vector<unsigned>& List, const char* szFileName) {
	FILE* fp = OpenDesktopFile(szFileName, "r"e);
	List.clear();

	if (!fp)
		return;

	char szBuf[0x100];
	while (fgets(szBuf, sizeof(szBuf), fp)) {
		char* pNewLine = strchr(szBuf, '\n');
		if (pNewLine)
			*pNewLine = 0;
		List.push_back(CompileTime::StrHash(szBuf));
	}

	fclose(fp);
}

void Hack::AddUserToList(std::vector<unsigned>& List, const char* szFileName, const char* szUserName) {
	if (!szUserName || !*szUserName)
		return;

	LoadList(List, szFileName);
	if (IsUserInList(List, szUserName))
		return;

	FILE* fp = OpenDesktopFile(szFileName, "a+"e);
	fprintf(fp, "%s\n"e, szUserName);
	fclose(fp);
	LoadList(List, szFileName);
}

void Hack::RemoveUserFromList(std::vector<unsigned>& List, const char* szFileName, const char* szUserName) {
	if (!szUserName || !*szUserName)
		return;

	LoadList(List, szFileName);
	if (!IsUserInList(List, szUserName))
		return;

	std::vector<std::string> Names;
	char szBuf[0x100];

	FILE* fp = OpenDesktopFile(szFileName, "r"e);
	while (fgets(szBuf, sizeof(szBuf), fp)) {
		char* pNewLine = strchr(szBuf, '\n');
		if (pNewLine)
			*pNewLine = 0;
		if (strcmp(szBuf, szUserName) == 0)
			continue;
		Names.push_back(szBuf);
	}
	fclose(fp);

	fp = OpenDesktopFile(szFileName, "w"e);
	for (const auto& Name : Names)
		fprintf(fp, "%s\n", Name.c_str());
	fclose(fp);

	LoadList(List, szFileName);
}

bool Hack::IsUserInList(const std::vector<unsigned>& List, const char* szUserName) const {
	if (!szUserName || !*szUserName)
		return false;

	const unsigned NameHash = CompileTime::StrHash(szUserName);
	for (const auto& Elem : List)
		if (Elem == NameHash)
			return true;

	return false;
}