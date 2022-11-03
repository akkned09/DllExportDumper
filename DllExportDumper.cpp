#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>


enum class OUTPUT_MODE : uintptr_t
{
	DEFAULT = 0,
	WITH_INDEX,
	CPP_VECTOR,
	PY_LIST,
	WITH_DLL_OFFSET_PLAIN,
	WITH_DLL_OFFSET_MAP,
};


int main(int argc, char** argv)
{
#ifdef _WIN64
	std::cout << "[i] dll export dumper x64\n";
#else
	std::cout << "[i] dll export dumper x32\n";
#endif

	if (argc < 4)
	{
		std::cout << "usage: " << argv[0] << " [dll name] [mode] [output file]\n";
		std::cout << "modes:\n";
		std::cout << "0 - default(one per line)\n";
		std::cout << "1 - index, name\n";
		std::cout << "2 - c++ std::vector\n";
		std::cout << "3 - python list\n";
		std::cout << "4 - dll offset, name\n";
		std::cout << "5 - c++ std::map (name to offset)\n";
		return 0;
	}

	std::string dllName = argv[1];
	OUTPUT_MODE mode = (OUTPUT_MODE)std::atoi(argv[2]);

	HANDLE dllHandle = ::GetModuleHandleA(dllName.c_str());
	if (!dllHandle)
	{
		std::cout << "[-] GetModuleHandle failed, trying LoadLibraryA\n";
		dllHandle = ::LoadLibraryA(dllName.c_str());
	}

	if (!dllHandle)
	{
		std::cout << "[-] failed to load dll: " << dllName << std::endl;
		return 0;
	}

	std::cout << "[+] dll loaded successfully\n";

	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)dllHandle;
	IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((BYTE*)dllHandle + dosHeader->e_lfanew);
	IMAGE_OPTIONAL_HEADER* optionalHeader = (IMAGE_OPTIONAL_HEADER*)&ntHeaders->OptionalHeader;
	IMAGE_EXPORT_DIRECTORY* exportDirectory = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)dllHandle + optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	ULONG* nameAddresses = (ULONG*)((BYTE*)dllHandle + exportDirectory->AddressOfNames);
	ULONG* functionAddresses = (ULONG*)((BYTE*)dllHandle + exportDirectory->AddressOfFunctions);
	USHORT* nameOrdinals = (USHORT*)((BYTE*)dllHandle + exportDirectory->AddressOfNameOrdinals);

	std::cout << "[+] function names loaded\n";
	std::cout << "[+] function addresses loaded\n";
	std::cout << "[+] name ordinals loaded\n";

	std::ofstream fout = std::ofstream(argv[3]);

	for (char& c : dllName)
	{
		if (c == '.')
			c = '_';
	}

	std::cout << "[+] dll name fixed\n";

	if (mode == OUTPUT_MODE::CPP_VECTOR)
		fout << "std::vector<const char*> " << dllName << "Functions = {\n";

	if (mode == OUTPUT_MODE::PY_LIST)
		fout << dllName << "_functions = [\n";

	if (mode == OUTPUT_MODE::WITH_DLL_OFFSET_MAP)
		fout << "std::map<const char*, unsigned long> " << dllName << "Functions = {\n";

	std::cout << "[+] using mode: " << (uintptr_t)mode << '\n';

	for (uintptr_t i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		switch (mode)
		{
		case OUTPUT_MODE::DEFAULT:
		{
			fout << (char*)((BYTE*)dllHandle + nameAddresses[i]) << '\n';
			break;
		}

		case OUTPUT_MODE::WITH_INDEX:
		{
			fout << i << ',' << (char*)((BYTE*)dllHandle + nameAddresses[i]) << '\n';
			break;
		}

		case OUTPUT_MODE::CPP_VECTOR:
		{
			fout << '\"' << (char*)((BYTE*)dllHandle + nameAddresses[i]) << "\",\n";
			break;
		}

		case OUTPUT_MODE::PY_LIST:
		{
			fout << '\'' << (char*)((BYTE*)dllHandle + nameAddresses[i]) << "\',\n";
			break;
		}

		case OUTPUT_MODE::WITH_DLL_OFFSET_PLAIN:
		{
			fout << std::hex << "0x" << functionAddresses[nameOrdinals[i]] << ',' << (char*)((BYTE*)dllHandle + nameAddresses[i]) << '\n';
			break;
		}

		case OUTPUT_MODE::WITH_DLL_OFFSET_MAP:
		{
			fout << std::hex << "{\"" << (char*)((BYTE*)dllHandle + nameAddresses[i]) << "\", 0x" << functionAddresses[nameOrdinals[i]] << "}, \n";
			break;
		}
		}
	}

	if (mode == OUTPUT_MODE::CPP_VECTOR)
		fout << "};";

	if (mode == OUTPUT_MODE::PY_LIST)
		fout << ']';

	if (mode == OUTPUT_MODE::WITH_DLL_OFFSET_MAP)
		fout << "};";

	fout.close();

	std::cout << "[+] successfully dumped " << exportDirectory->NumberOfNames << " functions to " << argv[3] << std::endl;
	return 0;
}