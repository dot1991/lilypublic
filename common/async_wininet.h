#pragma once
#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "Wininet.lib")
#include <vector>
#include <atomic>

#include "util.h"

class AsyncWinInet
{
private:
    enum class WinInetStatus {
        Idle,
        WaitUrlHandle,
        WaitComplete,
    };

    std::atomic<WinInetStatus> Status = WinInetStatus::Idle;
    HINTERNET hInet = 0;
    HINTERNET hOpenUrl = 0;

    std::vector<uint8_t> data;

    constexpr static size_t DATASIZE = 0x1000;
    uint8_t temp[DATASIZE] = {};
    INTERNET_BUFFERSA ib = {};

public:
    AsyncWinInet(const char* Agent = ""e) {
        hInet = InternetOpenA(Agent, INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_ASYNC);
        verify(hInet);
        InternetSetStatusCallbackA(hInet, CallbackFunction);
    }

    ~AsyncWinInet() {
        CloseConnect();
        InternetSetStatusCallback(hInet, 0);
        InternetCloseHandle(hInet);
    }

    AsyncWinInet& operator=(const AsyncWinInet& rhs) {
        //for std::map
        //Do not copy anything
        return *this;
    }

    void CloseConnect() {
        InternetCloseHandle(hOpenUrl);
        hOpenUrl = 0;
        while (!IsIdle());
    }

    bool IsIdle() const { return Status == WinInetStatus::Idle; }
    std::vector<uint8_t> GetData() const { return data; }

    bool AddOpenUrl(const char* Url) {
        if (!IsIdle())
            return false;

        hOpenUrl = 0;
        Status = WinInetStatus::WaitUrlHandle;

        InternetOpenUrlA(
            hInet,
            Url,
            0,
            0,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE,
            (DWORD_PTR)this);

        switch (GetLastError()) {
        case ERROR_IO_PENDING:
            return true;
        default:
            Status = WinInetStatus::Idle;
            return false;
        }
    }

    bool AddRead() {
        if (!IsIdle())
            return false;

        if (!hOpenUrl)
            return false;

        data.clear();

        ib = {
            .dwStructSize = sizeof(ib),
            .lpvBuffer = temp,
            .dwBufferLength = sizeof(temp)
        };

        Status = WinInetStatus::WaitComplete;

        InternetReadFileExA(hOpenUrl, &ib, IRF_ASYNC, (DWORD_PTR)this);

        switch (GetLastError()) {
        case ERROR_SUCCESS:
            Status = WinInetStatus::Idle;
            return true;
        case ERROR_IO_PENDING:
            return true;
        default:
            Status = WinInetStatus::Idle;
            return false;
        }
    }

    bool UpdateRead() {
        if (!IsIdle())
            return false;

        if (ib.dwBufferLength == 0)
            return true;

        data.insert(data.end(), temp, temp + ib.dwBufferLength);

        ib = {
            .dwStructSize = sizeof(ib),
            .lpvBuffer = temp,
            .dwBufferLength = sizeof(temp)
        };

        Status = WinInetStatus::WaitComplete;

        InternetReadFileExA(hOpenUrl, &ib, IRF_ASYNC, (DWORD_PTR)this);

        switch (GetLastError()) {
        case ERROR_SUCCESS:
            Status = WinInetStatus::Idle;
            return false;
        case ERROR_IO_PENDING:
            return false;
        default:
            Status = WinInetStatus::Idle;
            return false;
        }
    }

    static void CallbackFunction(HINTERNET hInternet, DWORD_PTR Context, DWORD dwInetStatus, LPVOID lpStatusInfo, DWORD dwStatusInfoLength) {
        AsyncWinInet& Inet = *(AsyncWinInet*)Context;

        switch (dwInetStatus) {
        case INTERNET_STATUS_HANDLE_CREATED:
        {
            INTERNET_ASYNC_RESULT& AsyncResult = *(INTERNET_ASYNC_RESULT*)lpStatusInfo;
            HINTERNET Handle = (HINTERNET)AsyncResult.dwResult;

            switch (Inet.Status) {
            case WinInetStatus::WaitUrlHandle:
                Inet.hOpenUrl = Handle;
                Inet.Status = WinInetStatus::WaitComplete;
                break;
            }
            break;
        }
        case INTERNET_STATUS_REQUEST_COMPLETE:
            Inet.Status = WinInetStatus::Idle;
            break;
        }
    }
};