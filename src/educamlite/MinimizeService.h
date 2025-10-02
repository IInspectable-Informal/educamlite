#pragma once

namespace winrt::educamlite::implementation
{
    struct MinimizeService : implements<MinimizeService, Windows::ApplicationModel::Background::IBackgroundTask>
    {
    public:
        MinimizeService();

        Windows::Foundation::IAsyncOperation<uint32_t> GetFullTrustPIDAsync() const;

        //IBackgroundTask
        void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance const&) const;

    private:
        mutable Windows::ApplicationModel::Background::BackgroundTaskDeferral m_TaskDeferral = nullptr;
        mutable Windows::ApplicationModel::AppService::AppServiceConnection m_SvcConnection = nullptr;

        mutable uint32_t m_FullTrustPID = 0; mutable bool m_IsGettingFullTrustPID = false;

        fire_and_forget OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection const&, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs const&) const;
        void OnTaskCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance const&, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason const&) const;
        void OnTaskClosed(Windows::ApplicationModel::AppService::AppServiceConnection const&, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs const&) const;
    };
}
