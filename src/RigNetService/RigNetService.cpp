/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include "RigNetService.h"
#include "ThreadPool.h"
#pragma endregion

CSampleService::CSampleService(PWSTR pszServiceName,
                               BOOL fCanStop,
                               BOOL fCanShutdown,
                               BOOL fCanPauseContinue)
    : CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}

CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}

//
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the
//   service by the SCM or when the operating system starts (for a service
//   that starts automatically). It specifies actions to take when the
//   service starts. In this code sample, OnStart logs a service-start
//   message to the Application log, and queues the main service function for
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore,
//   it usually polls or monitors something in the system. The monitoring is
//   set up in the OnStart method. However, OnStart does not actually do the
//   monitoring. The OnStart method must return to the operating system after
//   the service's operation has begun. It must not loop forever or block. To
//   set up a simple monitoring mechanism, one general solution is to create
//   a timer in OnStart. The timer would then raise events in your code
//   periodically, at which time your service could do its monitoring. The
//   other solution is to spawn a new thread to perform the main service
//   functions, which is demonstrated in this code sample.
//
void CSampleService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"RigNetService in OnStart",
                       EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}

//
//   FUNCTION: CSampleService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs
//   on a thread pool worker thread.
//
void CSampleService::ServiceWorkerThread(void)
{
    auto cpath = svandex::tools::GetCurrentPath();
	google::InitGoogleLogging("RigNetServiceGlog");
	google::SetLogDestination(google::GLOG_INFO, (cpath + "\\LogFiles\\").c_str());
	LOG(INFO) << "glog started.";

    tv::Setting *st = tv::Setting::instance();
    st->filepath = cpath + "\\..\\data\\setting.json";

    if (!st->LoadSetting())
    {
        // Signal the stopped event.
		LOG(INFO) << "Load Setting faied";
        SetEvent(m_hStoppedEvent);
        return;
    }

    server ts;
    //const std::string strlog = strpt.substr(0, pos) + "\\output.log";
    //std::ofstream log(strlog.c_str(), std::fstream::out);
    //std::ofstream log("C:\\Users\\Public\\Softwares\\RigNet-Test\\output.log",std::fstream::out);
	std::ofstream log((cpath + "\\LogFiles\\WebSocketConnection.log").c_str(), std::fstream::out|std::fstream::app);

    //logging
    ts.set_message_handler(bind(&on_message, &ts, ::_1, ::_2));

    //logging
    ts.set_access_channels(websocketpp::log::alevel::connect);
    ts.set_access_channels(websocketpp::log::alevel::disconnect);
    ts.set_access_channels(websocketpp::log::alevel::app);

    ts.get_alog().set_ostream(&log);
    ts.get_elog().set_ostream(&log);

    //preparation
    ts.init_asio();
    ts.listen(websocketpp::lib::asio::ip::tcp::v4(), 9006);
    ts.start_accept();

    //tvrig_server.run();
    std::thread t(&server::run, &ts);

    // Periodically check if the service is stopping.
    while (!m_fStopping)
    {
        // Perform main service function here...
        ::Sleep(2000);
    }

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);

    ts.stop_listening();
    t.join();

}

//
//   FUNCTION: CSampleService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the
//   service by SCM. It specifies actions to take when a service stops
//   running. In this code sample, OnStop logs a service-stop message to the
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with
//   SERVICE_STOP_PENDING if the procedure is going to take long time.
//
void CSampleService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"RigNetService in OnStop",
                       EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}