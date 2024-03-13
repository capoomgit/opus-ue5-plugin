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
		int32 BatchCount;
		TArray<FString> DownloadLinks;

		FQueueRow(FString InJob, FString InDateTime, FString InStatus, FString InJobID, int32 InBatchCount, TArray<FString> InDownloadLinks)
			: Job(InJob), DateTime(InDateTime), Status(InStatus), JobID(InJobID), BatchCount(InBatchCount), DownloadLinks(InDownloadLinks) {}
	};

public:
	bool GetQueueLoopEnabled();
	void SetQueueLoopEnabled(bool Enable);
	void ReadAndParseQueueFile();
	void QueueLoop();
	void FreeUnzipDLL();

private:
	// Queue logic methods
	void WriteQueueToFile();
	TSharedRef<ITableRow>OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// Button callbacks
	FReply ReturnButtonClicked();
	FReply EmptyQButtonClicked();

	// API methods
	void SendSecondAPIRequest_JobResult(FString jobID);
	void OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID);

	// Download and unzip methods
	bool ExtractZip(const FString& ZipFile, const FString& DestinationDirectory);
	void DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName, int32 BatchCount);
	void ImportFBX(const FString& DirectoryPath);

	// Helper methods
	void SetUpFileTypes();
	bool LoadUnzipDLL();
	FReply ShowRemoveWarningWindow(int32 QueueItemIndex);

private:
	TSharedPtr<SListView<TSharedPtr<FQueueRow>>> QueueListView;
	TArray<TSharedPtr<FQueueRow>> QueueData;
	int32 CurrentQueueIndex = 0;

	FString APIKey;
	FString SecondAPILink;
	bool IsQueueLoopEnabled = false;

	TArray<TSharedPtr<FString>> AvailableFileTypes;
	EditorNotificationHelper NotificationHelper;

	void* UnzipDLLHandle;
};
