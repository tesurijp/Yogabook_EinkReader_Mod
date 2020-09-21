#include "pch.h"
#include <tuple>
#include <algorithm>
#include <Windows.h>
#include "ConvertUtil.h"

using std::vector;
using std::tuple;
using std::tie;
using std::all_of;

namespace
{
	typedef vector<tuple<HKEY, const wchar_t*>> KeyPathList;

	KeyPathList kOffice16InstallKeyPathList = {
		{ HKEY_LOCAL_MACHINE,  LR"###(SOFTWARE\Microsoft\Office\16.0)###"},
		{ HKEY_CURRENT_USER,  LR"###(SOFTWARE\Microsoft\Office\16.0)###"},
	};

	KeyPathList kWord16InstallKeyPathList = {
		{ HKEY_CURRENT_USER,  LR"###(SOFTWARE\Microsoft\Office\16.0\Word)###"},
	};

	KeyPathList kExcel16InstallKeyPathList = {
		{ HKEY_CURRENT_USER,  LR"###(SOFTWARE\Microsoft\Office\16.0\Excel)###"},
	};

	KeyPathList kPowerPoint16InstallKeyPathList = {
		{ HKEY_CURRENT_USER,  LR"###(SOFTWARE\Microsoft\Office\16.0\PowerPoint)###"},
	};

	KeyPathList kViso16InstallKeyPathList = {
		{ HKEY_CURRENT_USER,  LR"###(SOFTWARE\Microsoft\Office\16.0\Visio)###"},
	};

	bool KeyFound(HKEY root, const wchar_t* path)
	{
		HKEY key = NULL;
		auto resultCode = RegOpenKeyExW(root, path, 0, KEY_READ, &key);
		if (resultCode == ERROR_SUCCESS)
		{
			RegCloseKey(key);
			return true;
		}
		return false;
	}

	bool AllKeyFound(const KeyPathList& keyList)
	{
		return std::all_of(
			keyList.begin(),
			keyList.end(),
			[](const std::tuple<HKEY, const wchar_t*>& val)->bool {
				HKEY root;
				const wchar_t* path;
				std::tie(root, path) = val;
				return KeyFound(root, path);
		});
	}
}

namespace ConvertUtil
{
	bool Office16Installed()
	{
		return AllKeyFound(kOffice16InstallKeyPathList);
	}

	bool WordInstalled()
	{
		return AllKeyFound(kOffice16InstallKeyPathList) && AllKeyFound(kWord16InstallKeyPathList);
	}

	bool ExcelInstalled()
	{
		return AllKeyFound(kOffice16InstallKeyPathList) && AllKeyFound(kExcel16InstallKeyPathList);
	}

	bool PowerPointInstalled()
	{
		return AllKeyFound(kOffice16InstallKeyPathList) && AllKeyFound(kPowerPoint16InstallKeyPathList);
	}

	bool VisoInstalled()
	{
		return AllKeyFound(kOffice16InstallKeyPathList) && AllKeyFound(kViso16InstallKeyPathList);
	}
}
