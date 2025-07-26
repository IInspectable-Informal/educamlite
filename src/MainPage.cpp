#include "pch.h"
#include "App.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

namespace winrt::educamlite::implementation
{
    constexpr std::chrono::sys_days winEpoch{ std::chrono::year{1601}/1/1 };
    const StorageFolder AppFolder = ApplicationData::Current().LocalFolder();

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

        return to_hstring(int(ymd.year())) + L"-" +
               to_hstring(unsigned(ymd.month())) + L"-" +
               to_hstring(unsigned(ymd.day())) + L"_" +
               to_hstring(hms.hours().count()) + L"-" +
               to_hstring(hms.minutes().count()) + L"-" +
               to_hstring(hms.seconds().count()) + L"-" +
               to_hstring(std::chrono::duration_cast<std::chrono::milliseconds>(hms.subseconds()).count());
    }

    fire_and_forget MainPage::OnCameraChanged(IInspectable const& sender, SelectionChangedEventArgs const&)
    {
        SettingsPanelRoot().IsEnabled(false);
        CameraList().IsEnabled(false);
        ResolutionList().IsEnabled(false);
        ResolutionList().SelectedIndex(-1);
        CaptureButton().IsEnabled(false);
        CaptureDelayButton().IsEnabled(false);
        WaitingControl().IsIndeterminate(true);
        try
        {
            int const& i = CameraList().SelectedIndex();
            if (mediaCapture)
            {
                co_await mediaCapture.StopPreviewAsync();
                PreviewControl().Source(nullptr);
            }
            if (i >= 0)
            {
                _Resolutions.clear(); auto items = ResolutionList().Items(); items.Clear();
                hstring const& deviceId = _Devices[i].Id();
                if (Cameras.find(deviceId) == Cameras.end())
                {
                    mediaCapture = co_await InitCamera(deviceId);
                    Cameras.emplace(deviceId, mediaCapture);
                } else { mediaCapture = Cameras[deviceId]; }

                auto prevProps = mediaCapture.VideoDeviceController().GetAvailableMediaStreamProperties(MediaStreamType::VideoPreview);
                for (auto const& prop : prevProps)
                {
                    if (auto videoProp = prop.try_as<VideoEncodingProperties>())
                    {
                        _Resolutions.push_back(videoProp);
                        items.Append(box_value(to_hstring(videoProp.Width()) + L"x" + to_hstring(videoProp.Height()) + L" " +
                                               to_hstring(videoProp.FrameRate().Numerator() / videoProp.FrameRate().Denominator()) + L"fps " +
                                               videoProp.Subtype()));
                    }
                }
                PreviewControl().Source(mediaCapture);
                co_await mediaCapture.StartPreviewAsync();
                ResolutionList().SelectedIndex(0);
                ResolutionList().IsEnabled(true);
                CaptureDelayButton().IsEnabled(true);
                CaptureButton().IsEnabled(true);

                UpdateVDControl(mediaCapture.VideoDeviceController());
            } else { mediaCapture = nullptr; }
            ResolutionList().PlaceholderText(i >= 0 ? L"请选择属性" : L"请先选择摄像头");
        }
        catch (hresult_error const& ex)
        { ShowInfo(muxc::InfoBarSeverity::Error, ex.message() + L"\nHRESULT: " + ToHex(ex.code())); }
        WaitingControl().IsIndeterminate(false);
        CameraList().IsEnabled(true);
        SettingsPanelRoot().IsEnabled(true);
    }

    fire_and_forget MainPage::OnResolutionChanged(IInspectable const& sender, SelectionChangedEventArgs const&)
    {
        SettingsPanelRoot().IsEnabled(false);
        ResolutionList().IsEnabled(false);
        WaitingControl().IsIndeterminate(true);
        try
        {
            int const& i = ResolutionList().SelectedIndex();
            if (i >= 0)
            {
                co_await mediaCapture.VideoDeviceController().SetMediaStreamPropertiesAsync(MediaStreamType::VideoPreview, _Resolutions[i]);
            }
        }
        catch (hresult_error const& ex)
        { ShowInfo(muxc::InfoBarSeverity::Error, ex.message() + L"\nHRESULT: " + ToHex(ex.code())); }
        WaitingControl().IsIndeterminate(false);
        if (CameraList().SelectedIndex() >= 0) { ResolutionList().IsEnabled(true); }
        SettingsPanelRoot().IsEnabled(true);
    }

    fire_and_forget MainPage::RefreshVideoInputDevices(IInspectable const& sender, RoutedEventArgs const&)
    {
        SettingsPanelRoot().IsEnabled(false); CameraList().IsEnabled(false); ResolutionList().IsEnabled(false);
        CameraList().PlaceholderText(L"正在获取可用设备"); ResolutionList().PlaceholderText(L"等待获取");
        ResolutionList().SelectedIndex(-1); CameraList().SelectedIndex(-1);
        auto const& items = CameraList().Items(); items.Clear(); _Devices.clear();
        DeviceInformationCollection const& devs = co_await DeviceInformation::FindAllAsync(DeviceClass::VideoCapture);
        uint32_t const& size = devs.Size();
        if (size > 0)
        {
            for (auto const& dev : devs)
            {
                _Devices.push_back(dev);
                items.Append(box_value(dev.Name()));
            }
            CameraList().PlaceholderText(L"请选择摄像头");
            ResolutionList().PlaceholderText(L"请先选择摄像头");
            CameraList().IsEnabled(true);
        }
        else
        {
            CameraList().PlaceholderText(L"没有可用的摄像头"); ResolutionList().PlaceholderText(L"不可用");
            FocusAdjustmentButton().IsEnabled(false); FocusAdjustmentButton().Content(box_value(L"不可用"));
        }
        SettingsPanelRoot().IsEnabled(true);
    }

    void MainPage::TmpDirMgrFlyoutOpening(IInspectable const&, RoutedEventArgs const&)
    { _TempFolderManagerFlyout.ShowAt(MenuButton()); }

    void MainPage::IsFullScreenMode(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto item = sender.as<MenuFlyoutItem>();
        item.Icon().as<FontIcon>().Glyph(_AppView.IsFullScreenMode() ? L"\xE73F" : L"\xE740");
        item.Text(_AppView.IsFullScreenMode() ? L"退出全屏" : L"全屏");
    }

    void MainPage::SetFullScreen(IInspectable const&, RoutedEventArgs const&)
    { if (_AppView.IsFullScreenMode()) { _AppView.ExitFullScreenMode(); } else { _AppView.TryEnterFullScreenMode(); } }

    fire_and_forget MainPage::CloseApp(IInspectable const&, RoutedEventArgs const&)
    {
        co_await _AppView.TryConsolidateAsync();
    }

    void MainPage::HandleFlyoutClose(FlyoutBase const&, FlyoutBaseClosingEventArgs const& e)
    { if (_ListOpened) { e.Cancel(true); } }

    void MainPage::RadioButtonClicked(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto control = sender.as<RadioButton>();
        auto tag = control.Tag().as<hstring>()[0];
        if (tag != _Selection)
        { _Selection = tag; }
        else
        {
            switch (tag)
            {
                case L'1':
                {
                    _PenColorPickerFlyout.ShowAt(control);
                    break;
                }
                case L'2':
                {
                    _EraserButtonFlyout.ShowAt(control);
                    break;
                }
            }
        }
    }

    void MainPage::ListButtonClicked(IInspectable const& sender, RoutedEventArgs const&)
    {
        try
        {
            bool isOpened = sender.as<ToggleButton>().IsChecked().Value();
            ListButtonText().Text(isOpened ? L"收起" : L"展开");
            auto children = PicListPanelStoryboard().Children();
            auto doubleAnimation = children.GetAt(0).as<DoubleAnimation>();
            auto discreteObjectKeyFrame = children.GetAt(1).as<ObjectAnimationUsingKeyFrames>().KeyFrames().GetAt(0);
            if (isOpened)
            {
                doubleAnimation.From(ActualHeight());
                doubleAnimation.To(0.0);
                discreteObjectKeyFrame.KeyTime(KeyTimeHelper::FromTimeSpan(TimeSpan(0)));
                discreteObjectKeyFrame.Value(box_value(Visibility::Visible));
            }
            else
            {
                doubleAnimation.From(0.0);
                doubleAnimation.To(ActualHeight());
                discreteObjectKeyFrame.KeyTime(KeyTimeHelper::FromTimeSpan(TimeSpan(3000000)));
                discreteObjectKeyFrame.Value(box_value(Visibility::Collapsed));
            }
            PicListPanelStoryboard().Begin();
        }
        catch (hresult_error const& ex)
        {
            ShowInfo(muxc::InfoBarSeverity::Error, ex.message() + L"\nHRESULT: " + ToHex(ex.code()));
        }
    }

    //Private functions
    void MainPage::ShowInfo(muxc::InfoBarSeverity const& severity, hstring const& message)
    {
        Info().Severity(severity);
        Info().Message(message);
        Info().IsOpen(true);
    }

    void MainPage::UpdateVDControl(VideoDeviceController const& control)
    {
        _VDControl = control; _FocusControl = control.Focus();
        _ExposureControl = control.Exposure(); _AdvEC = control.ExposureCompensationControl();
        _ZoomControl = control.Zoom(); _AdvZC = control.ZoomControl();
        auto fc = _FocusControl.Capabilities(); auto ec = _ExposureControl.Capabilities(); auto zc = _ZoomControl.Capabilities();
        ExposureSlider().ValueChanged(_ECEToken);
        ZoomSlider().ValueChanged(_ZCEToken);
        if (fc.Supported())
        {
            FocusAdjustmentButton().IsEnabled(true);
            _AllowAutoFocus = fc.AutoModeSupported();
            FocusSlider().Minimum(fc.Min() - _AllowAutoFocus * fc.Step());
            FocusSlider().Maximum(fc.Max());
            FocusSlider().StepFrequency(fc.Step());
            FocusSlider().Value(fc.Default());
        } else { FocusAdjustmentButton().IsEnabled(false); FocusAdjustmentButton().Content(box_value(L"不可用")); }
        if (_AdvEC.Supported())
        {
            ExposureSlider().Minimum(_AdvEC.Min());
            ExposureSlider().Maximum(_AdvEC.Max());
            ExposureSlider().StepFrequency(_AdvEC.Step());
            ExposureSlider().Value(_AdvEC.Value());
            _ECEToken = ExposureSlider().ValueChanged({ this, &MainPage::AdvExposureChanged });
            ExposureAdjustmentButton().IsEnabled(true);
        }
        else if (ec.Supported())
        {
            _ExposureControl.TrySetAuto(false);
            ExposureSlider().Minimum(ec.Min());
            ExposureSlider().Maximum(ec.Max());
            ExposureSlider().StepFrequency(ec.Step());
            ExposureSlider().Value(ec.Default());
            _ECEToken = ExposureSlider().ValueChanged({ this, &MainPage::ExposureChanged });
            ExposureAdjustmentButton().IsEnabled(true);
        } else { ExposureAdjustmentButton().IsEnabled(false); ExposureAdjustmentButton().Content(box_value(L"不可用")); }
        if (_AdvZC.Supported())
        {
            ZoomSlider().Minimum(_AdvZC.Min());
            ZoomSlider().Maximum(_AdvZC.Max());
            ZoomSlider().StepFrequency(_AdvZC.Step());
            ZoomSlider().Value(_AdvZC.Value());
            _ECEToken = ZoomSlider().ValueChanged({ this, &MainPage::AdvZoomChanged });
            ZoomAdjustmentButton().IsEnabled(true);
        }
        else if (zc.Supported())
        {
            _ZoomControl.TrySetAuto(false);
            ZoomSlider().Minimum(zc.Min());
            ZoomSlider().Maximum(zc.Max());
            ZoomSlider().StepFrequency(zc.Step());
            ZoomSlider().Value(zc.Default());
            _ZCEToken = ZoomSlider().ValueChanged({ this, &MainPage::ZoomChanged });
            ZoomAdjustmentButton().IsEnabled(true);
        } else { ZoomAdjustmentButton().IsEnabled(false); ZoomAdjustmentButton().Content(box_value(L"不可用")); }
    }

    void MainPage::FocusChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        if (_AllowAutoFocus && e.NewValue() == FocusSlider().Minimum())
        {
            _FocusControl.TrySetAuto(true);
            FocusAdjustmentButton().Content(box_value(L"自动"));
        }
        else
        {
            _FocusControl.TrySetAuto(false);
            _FocusControl.TrySetValue(e.NewValue());
            FocusAdjustmentButton().Content(box_value(e.NewValue()));
        }
    }

    void MainPage::ExposureChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        _ExposureControl.TrySetValue(e.NewValue());
        ExposureAdjustmentButton().Content(box_value(e.NewValue()));
    }

    fire_and_forget MainPage::AdvExposureChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        co_await _AdvEC.SetValueAsync(static_cast<float>(e.NewValue()));
        ExposureAdjustmentButton().Content(box_value(e.NewValue()));
    }

    void MainPage::ZoomChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        _ZoomControl.TrySetValue(e.NewValue());
        ZoomAdjustmentButton().Content(box_value(e.NewValue()));
    }

    void MainPage::AdvZoomChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        _AdvZC.Value(static_cast<float>(e.NewValue()));
        ZoomAdjustmentButton().Content(box_value(e.NewValue()));
    }

    fire_and_forget MainPage::CaptureRequested(IInspectable const& sender, RoutedEventArgs const&)
    {
        CaptureButton().IsEnabled(false);
        try
        {
            int const& i = ResolutionList().SelectedIndex();
            InMemoryRandomAccessStream memStream;
            co_await mediaCapture.CapturePhotoToStreamAsync(ImageEncodingProperties::CreateJpeg(), memStream);
            CaptureButton().IsEnabled(true);
            memStream.Seek(0);
            auto fileStream = co_await(co_await AppFolder.CreateFileAsync(GetCurrentTimeString() + SubtypeToFileExt(_Resolutions[i].Subtype()),
                                                CreationCollisionOption::GenerateUniqueName)
                                      ).OpenAsync(FileAccessMode::ReadWrite);
            co_await RandomAccessStream::CopyAndCloseAsync(memStream, fileStream);
            memStream.Seek(0);
            auto items = PicList().Items();
            ListButton().IsChecked(true);
            items.Append(PicItem(memStream, items.Size() + 1));
        }
        catch (hresult_error const& ex)
        { ShowInfo(muxc::InfoBarSeverity::Error, ex.message() + L"\nHRESULT: " + ToHex(ex.code())); CaptureButton().IsEnabled(true); }
    }

    void MainPage::PicListSizeChanged(IInspectable const&, SizeChangedEventArgs const& e)
    {
        if (e.NewSize().Height > e.PreviousSize().Height)
        {
            auto scrollViewer = PicList().Parent().as<ScrollViewer>();
            scrollViewer.ScrollToVerticalOffset(scrollViewer.ExtentHeight());
        }
    }

    void MainPage::ListOpened(IInspectable const&, IInspectable const&)
    { _ListOpened = true; }

    void MainPage::ListClosed(IInspectable const&, IInspectable const&)
    { _ListOpened = false; }

    //Initialization
    void MainPage::Init(IInspectable const&, RoutedEventArgs const&)
    {
        //InitializeComponent();
        winrt::com_ptr<App> app = Application::Current().as<App>();
        app->EnteredBackground([this](auto const&, auto const& e) -> IAsyncAction
        {
            auto deferral = e.GetDeferral();
            if (mediaCapture)
            {
                co_await mediaCapture.StopPreviewAsync();
                PreviewControl().Source(nullptr);
            }
            for (auto const& item : Cameras) { item.second.Close(); }
            Cameras.clear();
            deferral.Complete();
        });
        app->LeavingBackground([this](auto const&, auto const& e) -> IAsyncAction
        {
            auto deferral = e.GetDeferral();
            if (mediaCapture)
            {
                hstring const& deviceId = _Devices[CameraList().SelectedIndex()].Id();
                mediaCapture = co_await InitCamera(deviceId);
                Cameras.emplace(deviceId, mediaCapture);
                PreviewControl().Source(mediaCapture);
                co_await mediaCapture.StartPreviewAsync();
                UpdateVDControl(mediaCapture.VideoDeviceController());
            } deferral.Complete();
        });
        _PenColorPickerFlyout = Resources().Lookup(box_value(L"PenColorPickerFlyout")).as<Flyout>();
        _EraserButtonFlyout = Resources().Lookup(box_value(L"EraserButtonFlyout")).as<Flyout>();
        _TempFolderManagerFlyout = Resources().Lookup(box_value(L"TempFolderManagerFlyout")).as<Flyout>();
        Resources().Clear();
        CameraList().DropDownOpened({ this, &MainPage::ListOpened });
        CameraList().DropDownClosed({ this, &MainPage::ListClosed });
        ResolutionList().DropDownOpened({ this, &MainPage::ListOpened });
        ResolutionList().DropDownClosed({ this, &MainPage::ListClosed });
        FocusSlider().ValueChanged({ this, &MainPage::FocusChanged });
        PicList().SizeChanged({ this, &MainPage::PicListSizeChanged });
        _ECEToken = ExposureSlider().ValueChanged([](auto const& ...) { });
        _ZCEToken = ZoomSlider().ValueChanged([](auto const& ...) { });
        CaptureButton().Click({ this, &MainPage::CaptureRequested });
        RefreshVideoInputDevices();
    }

    MainPage::MainPage()
    {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
    }

    //Destructor
    MainPage::~MainPage()
    {
        for (auto const& item : Cameras)
        { item.second.Close(); }
    }
}
