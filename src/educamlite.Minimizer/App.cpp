#include "pch.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

const ApplicationDataContainer AppDC = ApplicationData::Current().LocalSettings();

const DWORD g_UWPProcessID = static_cast<DWORD>(ApplicationData::Current().LocalSettings().Values().Lookup(L"UWPProcessID").as<uint32_t>());
const DWORD g_CurrentProcessID = GetCurrentProcessId();

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    MessageBoxW(nullptr, to_hstring(static_cast<uint32_t>(g_CurrentProcessID)).c_str(), L"", 0x00000000L);
    return 0;
}
