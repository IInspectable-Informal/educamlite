﻿#pragma once
#include "App.xaml.g.h"

namespace winrt::educamlite::implementation
{
    struct App : AppT<App>
    {
        App();
        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
        void OnSuspending(IInspectable const&, Windows::ApplicationModel::SuspendingEventArgs const&);

    private:
        const winrt::Windows::UI::Xaml::Window _CurWin = winrt::Windows::UI::Xaml::Window::Current();
        bool _IsInitialized = false;
    };
}
