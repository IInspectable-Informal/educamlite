#pragma once
#include "MinimizeService.h"

ns winrt
{
    const local::implementation::MinimizeService InProcService;

    constexpr std::chrono::sys_days winEpoch{ std::chrono::year{1601}/1/1 };

    const Windows::Storage::ApplicationData AppData = Windows::Storage::ApplicationData::Current();
    const Windows::Storage::ApplicationDataContainer AppDC = Windows::Storage::ApplicationData::Current().LocalSettings();
    const Windows::Storage::AccessCache::StorageItemAccessList FA = Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList();

    hstring ToHex(long const& hresult);
    Windows::Foundation::IAsyncOperation<Windows::Media::Capture::MediaCapture> InitCamera(hstring const& deviceId);
    hstring SubtypeToFileExt(hstring const& subtype);
    hstring GetCurrentTimeString();

    void GetWindowHandle(Windows::UI::Core::CoreWindow const&, HWND* hWnd);
}