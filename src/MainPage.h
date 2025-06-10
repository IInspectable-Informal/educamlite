#pragma once
#include "MainPage.g.h"

using ns winrt;
using ns Windows::Devices::Enumeration;
using ns Windows::Foundation;
using ns Windows::Foundation::Collections;
using ns Windows::Media;
using ns Windows::Media::Capture;
using ns Windows::Media::Devices;
using ns Windows::Media::MediaProperties;
using ns Windows::Storage;
using ns Windows::Storage::Streams;
using ns Windows::UI::Core;
using ns Windows::UI::ViewManagement;
using ns Windows::UI::Xaml;
using ns Windows::UI::Xaml::Controls;
using ns Windows::UI::Xaml::Controls::Primitives;
ns muxc = Microsoft::UI::Xaml::Controls;

namespace winrt::educamlite::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        //XAML binding properties and events
        fire_and_forget OnCameraChanged(IInspectable const&, SelectionChangedEventArgs const&);
        fire_and_forget OnResolutionChanged(IInspectable const&, SelectionChangedEventArgs const&);
        fire_and_forget RefreshVideoInputDevices(IInspectable const& = nullptr, RoutedEventArgs const& = nullptr);
        void IsFullScreenMode(IInspectable const&, RoutedEventArgs const&);
        void SetFullScreen(IInspectable const&, RoutedEventArgs const&);
        fire_and_forget CloseApp(IInspectable const&, RoutedEventArgs const&);
        void HandleFlyoutClose(FlyoutBase const&, FlyoutBaseClosingEventArgs const&);
        //Initialization
        void Init(IInspectable const&, RoutedEventArgs const&);

    private:
        ApplicationView _AppView = ApplicationView::GetForCurrentView(); bool _ListOpened = false;
        IObservableVector<DeviceInformation> _Devices = single_threaded_observable_vector<DeviceInformation>();
        std::vector<VideoEncodingProperties> _Resolutions;
        VideoDeviceController _VDControl = nullptr;
        MediaDeviceControl _FocusControl = nullptr; bool _AllowAutoFocus = false;
        MediaDeviceControl _ExposureControl = nullptr; ExposureCompensationControl _AdvEC = nullptr; winrt::event_token _ECEToken;
        MediaDeviceControl _ZoomControl = nullptr; ZoomControl _AdvZC = nullptr; winrt::event_token _ZCEToken;
        std::map<hstring, MediaCapture> Cameras; MediaCapture mediaCapture = nullptr;

        void ShowInfo(muxc::InfoBarSeverity const&, hstring const&);
        void UpdateVDControl(VideoDeviceController const&);
        void FocusChanged(IInspectable const&, RangeBaseValueChangedEventArgs const&);
        void ExposureChanged(IInspectable const&, RangeBaseValueChangedEventArgs const&);
        fire_and_forget AdvExposureChanged(IInspectable const&, RangeBaseValueChangedEventArgs const&);
        void ZoomChanged(IInspectable const&, RangeBaseValueChangedEventArgs const&);
        void AdvZoomChanged(IInspectable const&, RangeBaseValueChangedEventArgs const&);
        fire_and_forget CaptureReqquested(IInspectable const& = nullptr, RoutedEventArgs const& = nullptr);
        void ListOpened(IInspectable const&, IInspectable const&);
        void ListClosed(IInspectable const&, IInspectable const&);
    };
}

namespace winrt::educamlite::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
