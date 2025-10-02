#include "pch.h"
#include "MinimizeService.h"

#include <minwindef.h>
#include <processthreadsapi.h>

using ns winrt;
using ns Windows::ApplicationModel;
using ns Windows::ApplicationModel::AppService;
using ns Windows::ApplicationModel::Background;
using ns Windows::Foundation;
using ns Windows::Foundation::Collections;

using ns std::literals::chrono_literals;

namespace winrt::educamlite::implementation
{
    MinimizeService::MinimizeService()
    {
        
    }

    IAsyncOperation<uint32_t> MinimizeService::GetFullTrustPIDAsync() const
    {
        if (!m_IsGettingFullTrustPID)
        {
            m_IsGettingFullTrustPID = true;
            if (m_FullTrustPID == 0 || OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_FullTrustPID) == NULL)
            {
                co_await FullTrustProcessLauncher::LaunchFullTrustProcessForCurrentAppAsync();
                ValueSet message;
                message.Insert(L"Operation", box_value(L"GetFullTrustPID"));
                auto response = co_await m_SvcConnection.SendMessageAsync(message);
                if (response.Status() == AppServiceResponseStatus::Success)
                { m_FullTrustPID = response.Message().Lookup(L"PID").as<uint32_t>(); }
            }
            m_IsGettingFullTrustPID = false;
            co_return m_FullTrustPID;
        } else { co_return 4; }
    }

    //IBackgroundTask
    void MinimizeService::Run(IBackgroundTaskInstance const& taskInst) const
    {
        m_TaskDeferral = taskInst.GetDeferral();
        taskInst.Canceled({ this, &MinimizeService::OnTaskCanceled });
        m_SvcConnection = taskInst.TriggerDetails().as<AppServiceTriggerDetails>().AppServiceConnection();
        m_SvcConnection.RequestReceived({ this, &MinimizeService::OnRequestReceived });
        m_SvcConnection.ServiceClosed({ this, &MinimizeService::OnTaskClosed });
    }

    fire_and_forget MinimizeService::OnRequestReceived(AppServiceConnection const&, AppServiceRequestReceivedEventArgs const&) const
    {
        co_await resume_after(0s);
    }

    void MinimizeService::OnTaskCanceled(IBackgroundTaskInstance const&, BackgroundTaskCancellationReason const&) const
    {
        if (m_TaskDeferral != nullptr)
        { m_TaskDeferral.Complete(); }
    }

    void MinimizeService::OnTaskClosed(AppServiceConnection const&, AppServiceClosedEventArgs const&) const
    {
        if (m_TaskDeferral != nullptr)
        { m_TaskDeferral.Complete(); }
    }
}
