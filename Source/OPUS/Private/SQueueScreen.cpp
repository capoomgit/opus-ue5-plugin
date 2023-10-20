// Fill out your copyright notice in the Description page of Project Settings.


#include "SQueueScreen.h"
#include "SlateOptMacros.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Serialization/JsonReader.h" 
#include "Serialization/JsonSerializer.h" 

#define LOCTEXT_NAMESPACE "FOPUSModule"
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQueueScreen::Construct(const FArguments& InArgs)
{
    // Store API key
    APIKey = InArgs._APIKey;
    IsFBXSelected = InArgs._IsFBXSelected;

	ChildSlot
	[
        SNew(SOverlay)

            + SOverlay::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Left)
            [
                SNew(SButton)
                    .Text(FText::FromString(TEXT("←")))
                    .OnClicked(this, &SQueueScreen::ReturnButtonClicked)
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Right)
            [
                SNew(SHorizontalBox) // Add a new SHorizontalBox here

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                            .Text(FText::FromString(TEXT("🗑️ Empty List"))) // Replace this with your desired text
                            .OnClicked(this, &SQueueScreen::EmptyQButtonClicked) // Add your EmptyListButtonClicked function
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10, 0, 0, 0) // Add padding between buttons
                    [
                        SNew(SButton)
                            .Text(FText::FromString(TEXT("↺ Empty Caching")))
                            .OnClicked(this, &SQueueScreen::RefreshCachingButtonClicked)
                    ]
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 0, 0, 0)
            [
                SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                    .BorderBackgroundColor(FLinearColor::Gray)
                    .Padding(FMargin(2))
                    [
                        SNew(SVerticalBox)

                            + SVerticalBox::Slot()
                            [
                                SNew(SBox)
                                    .WidthOverride(650)
                                    .HeightOverride(500)
                                    [
                                        SAssignNew(QueueListView, SListView<TSharedPtr<FQueueRow>>)
                                            .ItemHeight(24)
                                            .ListItemsSource(&QueueData)
                                            .OnGenerateRow(this, &SQueueScreen::OnGenerateRowForList)
                                            .HeaderRow
                                            (
                                                SNew(SHeaderRow)
                                                + SHeaderRow::Column("JobColumn")
                                                .DefaultLabel(LOCTEXT("JobColumnHeader", "Job"))
                                                .FillWidth(0.7f)

                                                + SHeaderRow::Column("DateTimeColumn")
                                                .DefaultLabel(LOCTEXT("DateTimeColumnHeader", "Date Time"))
                                                .FillWidth(1.0f)

                                                + SHeaderRow::Column("StatusColumn")
                                                .DefaultLabel(LOCTEXT("StatusColumnHeader", "Status"))
                                                .FillWidth(1.3f)
                                            )
                                    ]
                            ]
                    ]
            ]
	];
	
    ReadAndParseQueueFile();
    QueueLoop();
}

// --------------------------------
// --- QUEUE LOGIC METHODS --------
// --------------------------------

void SQueueScreen::QueueLoop()
{
    if (QueueData.Num() > 0)
    {
        // Access the JobID of the current FQueueRow object
        FString currentJobID = QueueData[CurrentQueueIndex]->JobID;

        SendSecondAPIRequest_JobResult(currentJobID);

        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
            {
                FPlatformProcess::Sleep(2);

                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        if (QueueData.Num() == 0)
                        {
                            return;
                        }

                        ReadAndParseQueueFile();
                        QueueListView->RequestListRefresh();

                        if (QueueData.Num() > 0) {
                            CurrentQueueIndex = (CurrentQueueIndex + 1) % QueueData.Num();
                        }
                        else {
                            CurrentQueueIndex = 0;
                        }

                        QueueLoop();
                    });
            });
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Successfully exited the loop!"));
        CurrentQueueIndex = 0;
    }
}

void SQueueScreen::ReadAndParseQueueFile()
{
    // Clear any existing data
    QueueData.Empty();

    FString SaveDirectory = FPaths::ProjectSavedDir();
    // TODO make the file name a constant
    FString FileName = FString(TEXT("queue.txt"));
    FString AbsolutePath = SaveDirectory + "/" + FileName;

    // File I/O
    IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
    if (file.FileExists(*AbsolutePath))
    {
        // Create File Handle to read the file
        TUniquePtr<IFileHandle> fileHandle(file.OpenRead(*AbsolutePath));
        if (fileHandle)
        {
            // Read the file into a buffer
            TArray<uint8> buffer;
            buffer.SetNumUninitialized(fileHandle->Size());
            fileHandle->Read(buffer.GetData(), buffer.Num());

            // Convert buffer to string
            FString fileContent;
            FFileHelper::BufferToString(fileContent, buffer.GetData(), buffer.Num());

            // Parse lines
            TArray<FString> lines;
            fileContent.ParseIntoArray(lines, TEXT("\n"), true);

            for (FString line : lines)
            {
                TArray<FString> tokens;
                line.ParseIntoArray(tokens, TEXT(" "), true);

                if (tokens.Num() >= 4)  // Ensure there are at least 4 tokens
                {
                    FString job = tokens[0];
                    FString dateTime = tokens[1];
                    FString status = tokens[2];
                    FString jobID = tokens[3];
                    FString downloadLink = (tokens.Num() >= 5) ? tokens[4] : TEXT("");  // Parsing the download link

                    QueueData.Add(MakeShareable(new FQueueRow(job, dateTime, status, jobID, downloadLink)));
                }
            }

            // Refresh the ListView to reflect the new data
            QueueListView->RequestListRefresh();
        }
    }
}

TSharedRef<ITableRow> SQueueScreen::OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    FLinearColor TextColor;

    //TODO: this section must be revised to a switch structure
    if (InItem->Status == TEXT("IN_PROGRESS"))
    {
        TextColor = FLinearColor::Yellow;
    }
    else if (InItem->Status == TEXT("COMPLETED"))
    {
        TextColor = FLinearColor::Green;
    }
    else if (InItem->Status == TEXT("FAILED"))
    {
        TextColor = FLinearColor::Red;
    }
    else if (InItem->Status == TEXT("SUSPENDED"))
    {
        TextColor = FLinearColor::Gray;
    }
    else
    {
        TextColor = FLinearColor::White; // Default color
    }
    int32 CurrentIndex = QueueData.Find(InItem); // Find the index of the current item in QueueData

    return SNew(STableRow<TSharedPtr<FQueueRow>>, OwnerTable)
        [
            SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(0.7f)
                [
                    SNew(STextBlock).Text(FText::FromString(InItem->Job))
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(STextBlock).Text(FText::FromString(InItem->DateTime))
                ]

                + SHorizontalBox::Slot()
                .FillWidth(0.9f)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(InItem->Status))
                        .ColorAndOpacity(TextColor)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("+")))
                        .OnClicked_Lambda([this, CurrentIndex]() {
                        if (QueueData[CurrentIndex]->Status == TEXT("COMPLETED")) {
                            // TODO Download and unzip using 7zip
                            //DownloadAndUnzipMethod(QueueData[CurrentIndex]->DownloadLink, QueueData[CurrentIndex]->DateTime, QueueData[CurrentIndex]->Job);
                            NotificationHelper.ShowNotificationSuccess(LOCTEXT("Success", "The addition of the component is in progress. This might take some time varying the size of the job."));
                        }
                        else if (QueueData[CurrentIndex]->Status == TEXT("IN_PROGRESS")) {
                            NotificationHelper.ShowNotificationPending(LOCTEXT("Pending", "Job is not ready yet. Please wait..."));
                        }
                        else if (QueueData[CurrentIndex]->Status == TEXT("FAILED")) {
                            NotificationHelper.ShowNotificationFail(LOCTEXT("ErrorOccured", "Job failed. Please try again later."));
                        }
                        else if (QueueData[CurrentIndex]->Status == TEXT("SUSPENDED")) {
                            NotificationHelper.ShowNotificationFail(LOCTEXT("OpusError", "Due to some reasons the job is suspended from the OPUS. Please try again later."));
                        }
                        return FReply::Handled();
                            })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("×")))
                        .OnClicked_Lambda([this, CurrentIndex]() {
                        // Remove the item from QueueData
                        if (CurrentIndex >= 0 && CurrentIndex < QueueData.Num()) {
                            QueueData.RemoveAt(CurrentIndex);

                            WriteQueueToFile();
                            QueueListView->RequestListRefresh();
                        }
                        return FReply::Handled();
                            })
                ]
        ];
}

void SQueueScreen::WriteQueueToFile()
{
    FString SaveDirectory = FPaths::ProjectSavedDir();
    FString FileName = FString(TEXT("queue.txt"));
    FString AbsolutePath = SaveDirectory + "/" + FileName;

    FString fileContent;

    for (const auto& jobData : QueueData)
    {
        fileContent += jobData->Job + " " + jobData->DateTime + " " + jobData->Status + " " + jobData->JobID + " " + jobData->DownloadLink + "\n";
    }

    // Write to file
    FFileHelper::SaveStringToFile(fileContent, *AbsolutePath);
}

// --------------------------------
// --- BUTTON CALLBACK METHODS ----
// --------------------------------
FReply SQueueScreen::ReturnButtonClicked()
{
    // TODO Return to creation screen from queue screen
    //bIsQueueClicked = false;  // Reset to show the main page
    //RebuildWidget();  // Rebuild the widget to go back to the main page
    return FReply::Handled();
}

FReply SQueueScreen::EmptyQButtonClicked()
{
    // Construct the full path to the queue.txt file
    FString ProjectDir = FPaths::ProjectSavedDir();
    FString QueueFile = FPaths::Combine(ProjectDir, TEXT("queue.txt"));

    // Empty the queue.txt file
    FFileHelper::SaveStringToFile(TEXT(""), *QueueFile);

    return FReply::Handled();
}

FReply SQueueScreen::RefreshCachingButtonClicked()
{
    TSharedPtr<SWindow> WarningWindowPtr; // Declare a shared pointer
    TSharedRef<SWindow> WarningWindow = SNew(SWindow)
        .Title(LOCTEXT("WarningTitle", "Warning"))
        .ClientSize(FVector2D(600, 150))
        .IsInitiallyMaximized(false);

    WarningWindowPtr = WarningWindow; // Store the window in the shared pointer

    FSlateApplication::Get().AddWindowAsNativeChild(WarningWindow, FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());

    WarningWindow->SetContent(
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(15)
        [
            SNew(STextBlock)
                .Text(LOCTEXT("WarningText", "You are now deleting all cached jobs you created!\n This may cause problems if you have jobs in progress\n Are you sure you want to proceed?"))
                .Justification(ETextJustify::Center)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(15)
        [
            SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(LOCTEXT("YesButton", "Yes"))
                        .OnClicked_Lambda([this, WarningWindowPtr]() {
                        if (WarningWindowPtr.IsValid())
                        {
                            WarningWindowPtr->RequestDestroyWindow();
                        }

                        // Construct the path to the UnzippedContents folder
                        FString ProjectDir = FPaths::ProjectSavedDir();
                        FString ZippedContents = FPaths::Combine(ProjectDir, TEXT("ZippedContents"));

                        // Get the platform file manager
                        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

                        // Check if the directory exists
                        if (PlatformFile.DirectoryExists(*ZippedContents))
                        {
                            // Delete all files and subdirectories in the UnzippedContents directory
                            PlatformFile.DeleteDirectoryRecursively(*ZippedContents);

                            // Create the directory again after deleting
                            PlatformFile.CreateDirectory(*ZippedContents);
                        }

                        return FReply::Handled();
                            })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(LOCTEXT("NoButton", "No"))
                        .OnClicked_Lambda([WarningWindowPtr]() {
                        if (WarningWindowPtr.IsValid())
                        {
                            WarningWindowPtr->RequestDestroyWindow();
                        }
                        return FReply::Handled();
                            })
                ]
        ]);

    return FReply::Handled();
}

// --------------------------------
// --- API METHODS ----------------
// --------------------------------
void SQueueScreen::SendSecondAPIRequest_JobResult(FString jobID)
{
    FString EncodedJobID = FGenericPlatformHttp::UrlEncode(jobID.TrimStartAndEnd()); // URL encoding after trimming
    FString URL = "https://opus5.p.rapidapi.com/get_opus_job_result";
    FString Parameters = "?result_uid=" + EncodedJobID;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(URL + Parameters);
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey.ToString());
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindLambda([this, jobID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            OnSecondAPIRequestJobResultCompleted(Request, Response, bWasSuccessful, jobID);
        });
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Error, TEXT("Reached the end of request"));
    UE_LOG(LogTemp, Log, TEXT("Stored API Key: %s"), *APIKey.ToString());
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(URL + Parameters));
}

void SQueueScreen::OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID)
{
    //TODO: this section must be revised to a proper try-catch structure
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        FString ResponseStr = Response->GetContentAsString();

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString status;
            if (JsonObject->TryGetStringField("job_status", status))
            {
                UE_LOG(LogTemp, Warning, TEXT("JobID from response: %s"), *jobID);

                // The existing loop and code for updating job data
                for (auto& jobData : QueueData)
                {
                    if (jobData->JobID == jobID)
                    {
                        jobData->Status = status;
                        break;
                    }
                }

                WriteQueueToFile();
                QueueListView->RequestListRefresh();

                if (status == "COMPLETED")
                {
                    TSharedPtr<FJsonObject> urlsObject = JsonObject->GetObjectField("urls");
                    if (urlsObject.IsValid())
                    {
                        FString extensionKey = IsFBXSelected ? "fbx" : "gltf";
                        FString link;

                        if (urlsObject->TryGetStringField(*extensionKey, link))
                        {
                            SecondAPILink = link;
                            UE_LOG(LogTemp, Warning, TEXT("API link: %s"), *SecondAPILink);

                            for (auto& jobData : QueueData)
                            {
                                if (jobData->JobID == jobID)
                                {
                                    jobData->DownloadLink = link;
                                    break;
                                }
                            }

                            WriteQueueToFile();
                            QueueListView->RequestListRefresh();
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Failed to get %s link"), *extensionKey);
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Urls object is not valid"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Job status is not completed"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to get job status"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize JSON"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Request failed: %s"), *Response->GetContentAsString());
    }
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE