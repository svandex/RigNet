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
    WriteEventLogEntry(L"CppWindowsService in OnStart",
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
    TCHAR dest[MAX_PATH];
    rignet::tools::GetCurrentPath(dest);
    size_t len=wcslen(dest)+1;
    size_t converted=0;
    char* destStr = (char*)malloc(len*sizeof(char));
    wcstombs_s(&converted,destStr,len,dest,_TRUNCATE);
#ifdef SvandexDebug
    std::ofstream debugLog("c:\\Users\\saictv\\Desktop\\debuglog.txt",std::fstream::out);
    debugLog<<"start"<<std::endl;
    debugLog<<destStr<<std::endl;
#endif

    /*
    //Get directory to load setting.json
    LPSTR pt = NULL;
    if (GetModuleFileNameA(NULL, pt, MAX_PATH) == 0)
    {
#ifdef SvandexDebug
        debugLog<<"Cannot get directory"<<std::endl<<pt<<std::endl;
#endif
        // Signal the stopped event.
        SetEvent(m_hStoppedEvent);
        return;
    }

#ifdef SvandexDebug
        debugLog<<"Directory: "<<std::endl<<pt<<std::endl;
#endif
    */

    tv::Setting *st = tv::Setting::instance();
    //setting.json
    //std::string strpt(pt);
    //int pos = strpt.find_last_of("\\", strpt.length());
    //st->filepath="C:\\Users\\Public\\Repositories\\RigNet\\data\\setting.json";
    st->filepath=std::string(destStr)+"\\setting.json";
#ifdef SvandexDebug
    debugLog<<"filepath set"<<std::endl;
#endif

    if (!st->LoadSetting())
    {
#ifdef SvandexDebug
        debugLog<<"Cannot load setting.json"<<std::endl<<st->filepath<<std::endl;
#endif
        // Signal the stopped event.
        SetEvent(m_hStoppedEvent);
        return;
    }

#ifdef SvandexDebug
    debugLog<<"All set, start server"<<std::endl;
#endif
    server ts;
    //const std::string strlog = strpt.substr(0, pos) + "\\output.log";
    //std::ofstream log(strlog.c_str(), std::fstream::out);
    //std::ofstream log("C:\\Users\\Public\\Softwares\\RigNet-Test\\output.log",std::fstream::out);
    std::ofstream log(std::string(destStr)+"\\output.log",std::fstream::out);

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
    WriteEventLogEntry(L"CppWindowsService in OnStop",
                       EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}
