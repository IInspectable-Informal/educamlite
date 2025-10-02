#pragma once
//Windows
#include <unknwn.h>
#include <winbase.h>
#include <processthreadsapi.h>
#include <winuser.h>

extern "C" {
    int WINAPI MessageBoxW(_In_opt_ HWND hWnd, _In_opt_ LPCWSTR lpText, _In_opt_ LPCWSTR lpCaption, _In_ UINT uType);
}

#pragma comment(lib, "user32.lib")

//WinRT
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.AppService.h>
#include <winrt/Windows.ApplicationModel.Background.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>
