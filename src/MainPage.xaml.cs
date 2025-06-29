using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Media.Capture;
using Windows.Media.Devices;
using Windows.Media.MediaProperties;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI.Popups;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using muxc = Microsoft.UI.Xaml.Controls;

// https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x804 上介绍了“空白页”项模板

namespace educamlite.cs
{
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        ApplicationView _AppView = ApplicationView.GetForCurrentView(); bool _ListOpened = false;
        ObservableCollection<DeviceInformation> _Devices = new ObservableCollection<DeviceInformation>();
        List<VideoEncodingProperties> _Resolutions = new List<VideoEncodingProperties>();
        VideoDeviceController _VDControl = null;
        MediaDeviceControl _FocusControl = null; bool _AllowAutoFocus = false;
        MediaDeviceControl _ExposureControl = null; ExposureCompensationControl _AdvEC = null; RangeBaseValueChangedEventHandler _ECEToken = null;
        MediaDeviceControl _ZoomControl = null; ZoomControl _AdvZC = null; RangeBaseValueChangedEventHandler _ZCEToken = null;
        Dictionary<string, MediaCapture> Cameras = new Dictionary<string, MediaCapture>(); MediaCapture mediaCapture = null;

        public MainPage()
        {
            this.InitializeComponent();
        }

        async void OnCameraChanged(object sender, SelectionChangedEventArgs e)
        {
            SettingsPanelRoot.IsEnabled = false;
            CameraList.IsEnabled = false;
            ResolutionList.IsEnabled = false;
            ResolutionList.SelectedIndex = -1;
            CaptureButton.IsEnabled = false;
            CaptureDelayButton.IsEnabled = false;
            WaitingControl.IsIndeterminate = true;
            try
            {
                int i = CameraList.SelectedIndex;
                if (mediaCapture != null)
                {
                    await mediaCapture.StopPreviewAsync();
                    PreviewControl.Source = null;
                }
                if (i >= 0)
                {
                    _Resolutions.Clear(); var items = ResolutionList.Items; items.Clear();
                    string deviceId = _Devices[i].Id;
                    if (!Cameras.ContainsKey(deviceId))
                    {
                        mediaCapture = await InitCamera(deviceId);
                        Cameras.Add(deviceId, mediaCapture);
                    } else { mediaCapture = Cameras[deviceId]; }

                    var prevProps = mediaCapture.VideoDeviceController.GetAvailableMediaStreamProperties(MediaStreamType.VideoPreview);
                    foreach (var prop in prevProps)
                    {
                        if (prop is VideoEncodingProperties videoProp)
                        {
                            _Resolutions.Add(videoProp);
                            items.Add(videoProp.Width + "x" + videoProp.Height + " " +
                                videoProp.FrameRate.Numerator / videoProp.FrameRate.Denominator + "fps " +
                                videoProp.Subtype);
                        }
                    }
                    PreviewControl.Source = mediaCapture;
                    await mediaCapture.StartPreviewAsync();
                    ResolutionList.SelectedIndex = 0;
                    ResolutionList.IsEnabled = true;
                    CaptureDelayButton.IsEnabled = true;
                    CaptureButton.IsEnabled = true;

                    UpdateVDControl(mediaCapture.VideoDeviceController);
                } else { mediaCapture = null; }
                ResolutionList.PlaceholderText = i >= 0 ? "请选择属性" : "请先选择摄像头";
            }
            catch (Exception ex)
            { ShowInfo(muxc.InfoBarSeverity.Error, ex.ToString()); }
            WaitingControl.IsIndeterminate = false;
            CameraList.IsEnabled = true;
            SettingsPanelRoot.IsEnabled = true;
        }

        async void OnResolutionChanged(object sender, SelectionChangedEventArgs e)
        {
            SettingsPanelRoot.IsEnabled = false;
            ResolutionList.IsEnabled = false;
            WaitingControl.IsIndeterminate = true;
            try
            {
                int i = ResolutionList.SelectedIndex;
                if (i >= 0)
                {
                    await mediaCapture.VideoDeviceController.SetMediaStreamPropertiesAsync(MediaStreamType.VideoPreview, _Resolutions[i]);
                }
            }
            catch (Exception ex)
            { ShowInfo(muxc.InfoBarSeverity.Error, ex.ToString()); }
            WaitingControl.IsIndeterminate = false;
            if (CameraList.SelectedIndex >= 0) { ResolutionList.IsEnabled = true; }
            SettingsPanelRoot.IsEnabled = true;
        }

        async void RefreshVideoInputDevices(object sender = null, RoutedEventArgs e = null)
        {
            SettingsPanelRoot.IsEnabled = false;
            CameraList.IsEnabled = false;
            ResolutionList.IsEnabled = false;
            CameraList.PlaceholderText = "正在获取可用设备";
            ResolutionList.PlaceholderText = "等待获取";
            ResolutionList.SelectedIndex = -1;
            CameraList.SelectedIndex = -1;
            _Devices.Clear();
            DeviceInformationCollection devs = await DeviceInformation.FindAllAsync(DeviceClass.VideoCapture);
            int size = devs.Count;
            if (size > 0)
            {
                for (int i = 0; i<size; ++i)
                { _Devices.Add(devs[i]); }
                CameraList.PlaceholderText = "请选择摄像头";
                ResolutionList.PlaceholderText = "请先选择摄像头";
                CameraList.IsEnabled = true;
            }
            else
            {
                CameraList.PlaceholderText = "没有可用的摄像头"; ResolutionList.PlaceholderText = "不可用";
                FocusAdjustmentButton.IsEnabled = false; FocusAdjustmentButton.Content = "不可用";
            }
            SettingsPanelRoot.IsEnabled = true;
        }
        void IsFullScreenMode(object sender, RoutedEventArgs e)
        {
            var item = sender as MenuFlyoutItem;
            item.Icon.As<FontIcon>().Glyph = _AppView.IsFullScreenMode ? "\xE73F" : "\xE740";
            item.Text = _AppView.IsFullScreenMode ? "退出全屏" : "全屏";
        }

        void SetFullScreen(object sender, RoutedEventArgs e)
        { if (_AppView.IsFullScreenMode) { _AppView.ExitFullScreenMode(); } else { _AppView.TryEnterFullScreenMode(); } }

        async void CloseApp(object sender, RoutedEventArgs e)
        { await _AppView.TryConsolidateAsync(); }

        void HandleFlyoutClose(FlyoutBase sender, FlyoutBaseClosingEventArgs e)
        { if (_ListOpened) { e.Cancel = true; } }

        //Private functions
        void ShowInfo(muxc.InfoBarSeverity severity, string message)
        {
            Info.Severity = severity;
            Info.Message = message;
            Info.IsOpen = true;
        }

        void UpdateVDControl(VideoDeviceController control)
        {
            _VDControl = control; _FocusControl = control.Focus;
            _ExposureControl = control.Exposure; _AdvEC = control.ExposureCompensationControl;
            _ZoomControl = control.Zoom; _AdvZC = control.ZoomControl;
            var fc = _FocusControl.Capabilities; var ec = _ExposureControl.Capabilities; var zc = _ZoomControl.Capabilities;
            ExposureSlider.ValueChanged -= _ECEToken;
            ZoomSlider.ValueChanged -= _ZCEToken;
            if (fc.Supported)
            {
                FocusAdjustmentButton.IsEnabled = true;
                _AllowAutoFocus = fc.AutoModeSupported;
                FocusSlider.Minimum = fc.Min - (_AllowAutoFocus ? fc.Step : 0);
                FocusSlider.Maximum = fc.Max;
                FocusSlider.StepFrequency = fc.Step;
                FocusSlider.Value = fc.Default;
            } else { FocusAdjustmentButton.IsEnabled = false; FocusAdjustmentButton.Content = "不可用"; }
            if (_AdvEC.Supported)
            {
                ExposureSlider.Minimum = _AdvEC.Min;
                ExposureSlider.Maximum = _AdvEC.Max;
                ExposureSlider.StepFrequency = _AdvEC.Step;
                ExposureSlider.Value = _AdvEC.Value;
                _ECEToken = AdvExposureChanged;
                ExposureSlider.ValueChanged += _ECEToken;
                ExposureAdjustmentButton.IsEnabled = true;
            }
            else if (ec.Supported)
            {
                _ExposureControl.TrySetAuto(false);
                ExposureSlider.Minimum = ec.Min;
                ExposureSlider.Maximum = ec.Max;
                ExposureSlider.StepFrequency = ec.Step;
                ExposureSlider.Value = ec.Default;
                _ECEToken = ExposureChanged;
                ExposureSlider.ValueChanged += _ECEToken;
                ExposureAdjustmentButton.IsEnabled = true;
            }
            else { ExposureAdjustmentButton.IsEnabled = false; ExposureAdjustmentButton.Content = "不可用"; }
            if (_AdvZC.Supported)
            {
                ZoomSlider.Minimum = _AdvZC.Min;
                ZoomSlider.Maximum = _AdvZC.Max;
                ZoomSlider.StepFrequency = _AdvZC.Step;
                ZoomSlider.Value = _AdvZC.Value;
                _ZCEToken = AdvZoomChanged;
                ZoomSlider.ValueChanged += _ZCEToken;
                ZoomAdjustmentButton.IsEnabled = true;
            }
            else if (zc.Supported)
            {
                _ZoomControl.TrySetAuto(false);
                ZoomSlider.Minimum = zc.Min;
                ZoomSlider.Maximum = zc.Max;
                ZoomSlider.StepFrequency = zc.Step;
                ZoomSlider.Value = zc.Default;
                _ZCEToken = ZoomChanged;
                ZoomSlider.ValueChanged += _ZCEToken;
                ZoomAdjustmentButton.IsEnabled = true;
            }
            else { ZoomAdjustmentButton.IsEnabled = false; ZoomAdjustmentButton.Content = "不可用"; }
        }

        void FocusChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (_AllowAutoFocus && e.NewValue == FocusSlider.Minimum)
            {
                _FocusControl.TrySetAuto(true);
                FocusAdjustmentButton.Content = "自动";
            }
            else
            {
                _FocusControl.TrySetAuto(false);
                _FocusControl.TrySetValue(e.NewValue);
                FocusAdjustmentButton.Content = e.NewValue;
            }
        }

        void ExposureChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            _ExposureControl.TrySetValue(e.NewValue);
            ExposureAdjustmentButton.Content = e.NewValue;
        }

        async void AdvExposureChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            await _AdvEC.SetValueAsync(e.NewValue.As<float>());
            ExposureAdjustmentButton.Content = e.NewValue;
        }

        void ZoomChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            _ZoomControl.TrySetValue(e.NewValue);
            ZoomAdjustmentButton.Content = e.NewValue;
        }

        void AdvZoomChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            _AdvZC.Value = e.NewValue.As<float>();
            ZoomAdjustmentButton.Content = e.NewValue;
        }

        async void CaptureReqquested(object sender, RoutedEventArgs e)
        {
            CaptureButton.IsEnabled = false;
            try
            {
                DateTime now = DateTime.Now;
                int i = ResolutionList.SelectedIndex;
                InMemoryRandomAccessStream memStream = new InMemoryRandomAccessStream();
                await mediaCapture.CapturePhotoToStreamAsync(ImageEncodingProperties.CreateJpeg(), memStream);
                CaptureButton.IsEnabled = true;
                memStream.Seek(0);
                var fileStream = await(await AppFolder.CreateFileAsync($"{now.Year}.{now.Month}.{now.Day}_{now.Hour}.{now.Minute}.{now.Second}.{now.Millisecond}.jpg", CreationCollisionOption.GenerateUniqueName)).OpenAsync(FileAccessMode.ReadWrite);
                await RandomAccessStream.CopyAndCloseAsync(memStream, fileStream);
            }
            catch (Exception ex)
            { ShowInfo(muxc.InfoBarSeverity.Error, ex.ToString()); CaptureButton.IsEnabled = true; }
        }

        void ListOpened(object sender, object e)
        { _ListOpened = true; }

        void ListClosed(object sender, object e)
        { _ListOpened = false; }
        //Initialization
        void Init(object sender, RoutedEventArgs e0)
        {
            //InitializeComponent();
            App app = Application.Current.As<App>();
            app.EnteredBackground += async(s, e) =>
            {
                var deferral = e.GetDeferral();
                if (mediaCapture != null)
                {
                    await mediaCapture.StopPreviewAsync();
                    PreviewControl.Source = null;
                }
                foreach (var item in Cameras) { item.Value.Dispose(); }
                Cameras.Clear();
                deferral.Complete();
            };
            app.LeavingBackground += async(s, e) =>
            {
                var deferral = e.GetDeferral();
                if (mediaCapture != null)
                {
                    string deviceId = _Devices[CameraList.SelectedIndex].Id;
                    mediaCapture = await InitCamera(deviceId);
                    Cameras.Add(deviceId, mediaCapture);
                    PreviewControl.Source = mediaCapture;
                    await mediaCapture.StartPreviewAsync();
                    UpdateVDControl(mediaCapture.VideoDeviceController);
                }
                deferral.Complete();
            };
            CameraList.DropDownOpened += ListOpened;
            CameraList.DropDownClosed += ListClosed;
            ResolutionList.DropDownOpened += ListOpened;
            ResolutionList.DropDownClosed += ListClosed;
            FocusSlider.ValueChanged += FocusChanged;
            _ECEToken = (s, e) => { };
            ExposureSlider.ValueChanged += _ECEToken;
            _ZCEToken = (s, e) => { };
            ZoomSlider.ValueChanged += _ZCEToken;
            CaptureButton.Click += CaptureReqquested;
            CameraList.ItemsSource = _Devices;
            RefreshVideoInputDevices();
        }

        void GetButtonInf(object sender, RoutedEventArgs e)
        {
            var button = sender.As<RadioButton>();
            _ = new MessageDialog(button.IsChecked.ToString(), button.Tag.As<string>()).ShowAsync();
        }

        static async Task<MediaCapture> InitCamera(string deviceId)
        {
            MediaCapture mediaCapture = new MediaCapture();
            MediaCaptureInitializationSettings settings = new MediaCaptureInitializationSettings
            {
                VideoDeviceId = deviceId,
                StreamingCaptureMode = StreamingCaptureMode.Video
            };
            await mediaCapture.InitializeAsync(settings);
            return mediaCapture;
        }

        static readonly StorageFolder AppFolder = ApplicationData.Current.LocalFolder;
    }

    static class ObjectHelper
    {
        public static T As<T>(this object obj)
        {
            return (T)obj;
        }
    }
}
