#include "pch.h"
#include "helpers.h"

#if defined(_M_IX86) || defined(__i386__)
#define _X86_
#elif defined(_M_X64) || defined(__x86_64__)
#define _AMD64_
#elif defined(_M_ARM64) || defined(__aarch64__)
#define _ARM64_
#elif defined(_M_ARM) || defined(__arm__)
#define _ARM_
#else
#error "Unsupported architecture"
#endif
#include <windef.h>

MIDL_INTERFACE("45D64A29-A63E-4CB6-B498-5781D298CB4F")
ICoreWindowInterop : public IUnknown
{
public:
    virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_WindowHandle( 
        /* [retval][out] */ __RPC__deref_out_opt HWND *hwnd) = 0;

    virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_MessageHandled( 
        /* [in] */ boolean value) = 0;
        
};

namespace winrt
{
    using ns Windows::Foundation;
    using ns Windows::Media::Capture;
    using ns Windows::UI::Core;

    hstring ToHex(long const& hresult)
    {
        wchar_t buffer[11];
        swprintf_s(buffer, 11, L"0x%08X", static_cast<unsigned int>(hresult));
        return hstring(buffer);
    }

    IAsyncOperation<MediaCapture> InitCamera(hstring const& deviceId)
    {
        MediaCapture mediaCapture = MediaCapture();
        MediaCaptureInitializationSettings settings;
        settings.VideoDeviceId(deviceId);
        settings.StreamingCaptureMode(StreamingCaptureMode::Video);
        co_await mediaCapture.InitializeAsync(settings);
        co_return mediaCapture;
    }

    hstring SubtypeToFileExt(hstring const& subtype)
    {
        if (subtype == L"NV12") { return L".jpg"; }
        if (subtype == L"MJPG") { return L".jpg"; }
        if (subtype == L"YUY2") { return L".jpg"; }
        return L".jpg";
    }

    hstring GetCurrentTimeString()
    {
        DateTime dateTime = winrt::clock::now();
        auto tp = winEpoch + std::chrono::duration_cast<std::chrono::system_clock::duration>(dateTime.time_since_epoch());

        auto days = floor<std::chrono::days>(tp);
        std::chrono::year_month_day ymd{days};

        std::chrono::hh_mm_ss hms{ tp - days };

        return std::format(
                   L"{}-{:02d}-{:02d}_{:02d}-{:02d}-{:02d}-{:03d}",
                   int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
                   hms.hours().count(), hms.minutes().count(), hms.seconds().count(),
                   std::chrono::duration_cast<std::chrono::milliseconds>(hms.subseconds()).count()
               ).c_str();
    }

    void GetWindowHandle(CoreWindow const& coreWindow, HWND* hWnd)
    { coreWindow.as<::ICoreWindowInterop>()->get_WindowHandle(hWnd); }
}