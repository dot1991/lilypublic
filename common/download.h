#pragma once
#include "async_wininet.h"
#include <map>

enum class DownloadStatus {
	WaitOpenUrl,
	WaitRead,
	Done,
	Failed
};

class Download {
private:
	struct DownloadInfo {
		AsyncWinInet Inet;
		DownloadStatus Code = DownloadStatus::Failed;
		float TimeOut = 0.0f;
		float StartTime = 0.0f;
	};

	std::map<std::string, DownloadInfo> DownloadList;

public:
	DownloadStatus GetStatus(std::string Url) {
		return DownloadList[Url].Code;
	}

	std::vector<uint8_t> GetData(std::string Url) {
		return DownloadList[Url].Inet.GetData();
	}

	void RemoveData(std::string Url) {
		DownloadList.erase(Url);
	}

	void Add(std::string Url, float TimeOut = 2.0f) {
		RemoveData(Url);

		auto& Elem = DownloadList[Url] = { .TimeOut = TimeOut, .StartTime = GetTimeSeconds() };
		AsyncWinInet& Inet = Elem.Inet;
		DownloadStatus& Status = Elem.Code;

		Status = Inet.AddOpenUrl(Url.c_str()) ?
			DownloadStatus::WaitOpenUrl :
			DownloadStatus::Failed;
	}

	void Update() {
		float TimeSeconds = GetTimeSeconds();
		for (auto& Elem : DownloadList) {
			const std::string& Url = Elem.first;
			AsyncWinInet& Inet = Elem.second.Inet;
			DownloadStatus& Status = Elem.second.Code;
			float& TimeOut = Elem.second.TimeOut;
			float& StartTime = Elem.second.StartTime;

			if (TimeSeconds > StartTime + TimeOut)
				Status = DownloadStatus::Failed;

			switch (Status) {
			case DownloadStatus::WaitOpenUrl:
			{
				if (!Inet.IsIdle())
					break;

				Status = Inet.AddRead() ?
					DownloadStatus::WaitRead :
					DownloadStatus::Failed;
				break;
			}
			case DownloadStatus::WaitRead:
			{
				if (!Inet.UpdateRead())
					break;

				Status = DownloadStatus::Done;
			}
			case DownloadStatus::Done:
			case DownloadStatus::Failed:
				Inet.CloseConnect();
				break;
			}
		}
	}
};