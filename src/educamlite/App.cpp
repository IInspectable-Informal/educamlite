#include "pch.h"
#include "App.h"
#include "helpers.h"

#include <processthreadsapi.h>

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::educamlite::implementation
{
    /// <summary>
    /// Creates the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
        Suspending({ this, &App::OnSuspending });
        AppDC.Values().Insert(L"UWPProcessID", box_value(static_cast<uint32_t>(GetCurrentProcessId())));
        #if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
        #endif
    }

    /// <summary>
    /// Invoked when the application is launched normally by the end user.  Other entry points
    /// will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    void App::OnLaunched(LaunchActivatedEventArgs const&)
    {
        if (!_IsInitialized)
        {
            _IsInitialized = true;
            Frame rootFrame;
            rootFrame.Navigate(xaml_typename<local::MainPage>());
            _CurWin.Content(rootFrame);
        }
        _CurWin.Activate();
    }

    void App::OnBackgroundActivated(BackgroundActivatedEventArgs const& args)
    { PrepareService(args); }

    /// <summary>
    /// Invoked when application execution is being suspended.  Application state is saved
    /// without knowing whether the application will be terminated or resumed with the contents
    /// of memory still intact.
    /// </summary>
    /// <param name="sender">The source of the suspend request.</param>
    /// <param name="e">Details about the suspend request.</param>
    void App::OnSuspending([[maybe_unused]] IInspectable const& sender, [[maybe_unused]] SuspendingEventArgs const& e)
    {
        // Save application state and stop any background activity
    }

    //Private functions
    fire_and_forget App::PrepareService(BackgroundActivatedEventArgs const& args)
    {
        unbox_value<IApplicationOverrides2>(*this).OnBackgroundActivated(args);
        InProcService.Run(args.TaskInstance());
        co_await InProcService.GetFullTrustPIDAsync();
    }
}
