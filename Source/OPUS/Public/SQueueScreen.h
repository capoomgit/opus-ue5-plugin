// Fill out your copyright notice in the Description page of Project Settings.

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
		, _IsFBXSelected(true)
		{}
	SLATE_ARGUMENT(FString, APIKey)
	SLATE_ARGUMENT(bool, IsFBXSelected)
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

private:
	// Queue logic methods
	void QueueLoop();
	void WriteQueueToFile();
	void ReadAndParseQueueFile();
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
	void DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName);

private:
	TSharedPtr<SListView<TSharedPtr<FQueueRow>>> QueueListView;
	TArray<TSharedPtr<FQueueRow>> QueueData;
	int32 CurrentQueueIndex = 0;

	FString APIKey;
	FString SecondAPILink;

	bool IsFBXSelected = true;
	EditorNotificationHelper NotificationHelper;
};
