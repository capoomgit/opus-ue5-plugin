// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Http.h"
#include "EditorNotificationHelper.h"

DECLARE_MULTICAST_DELEGATE(FOnCreationScreenEnabled);

class OPUS_API SQueueScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQueueScreen)
		: _APIKey("")
		{}
	SLATE_ARGUMENT(FString, APIKey)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void SetAPIKey(FString apiKey);

	// Delegate to show creation screen
	FOnCreationScreenEnabled OnCreationScreenEnabledDelegate;

public:
	// Struct Definitions
	struct FQueueRow
	{
		FString Job;
		FString DateTime;
		FString Status;
		FString JobID;
		FString DownloadLink;

		FQueueRow(FString InJob, FString InDateTime, FString InStatus, FString InJobID, FString InDownloadLink)
			: Job(InJob), DateTime(InDateTime), Status(InStatus), JobID(InJobID), DownloadLink(InDownloadLink) {}
	};

public:
	bool GetQueueLoopEnabled();
	void SetQueueLoopEnabled(bool Enable);
	void ReadAndParseQueueFile();
	void QueueLoop();

private:
	// Queue logic methods
	void WriteQueueToFile();
	TSharedRef<ITableRow>OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// Button callbacks
	FReply ReturnButtonClicked();
	FReply EmptyQButtonClicked();
	FReply RefreshCachingButtonClicked();

	// API methods
	void SendSecondAPIRequest_JobResult(FString jobID);
	void OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID);

	// Download and unzip methods
	bool ExtractWith7Zip(const FString& ZipFile, const FString& DestinationDirectory);
	void ZipProgressCallback();
	void DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName);
	void ImportFBX(const FString& DirectoryPath);

	// Helper methods
	void SetUpFileTypes();

private:
	TSharedPtr<SListView<TSharedPtr<FQueueRow>>> QueueListView;
	TArray<TSharedPtr<FQueueRow>> QueueData;
	int32 CurrentQueueIndex = 0;

	FString APIKey;
	FString SecondAPILink;
	bool IsQueueLoopEnabled = false;

	TArray<TSharedPtr<FString>> AvailableFileTypes;
	EditorNotificationHelper NotificationHelper;
};
