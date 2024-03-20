﻿// Copyright Capoom Inc. All Rights Reserved.


#include "SQueueScreen.h"
#include "SlateOptMacros.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Serialization/JsonReader.h" 
#include "Serialization/JsonSerializer.h"
#include "Misc/DefaultValueHelper.h"
#include "URLHelper.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Factories/FbxFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PackageTools.h"
#include "HAL/FileManagerGeneric.h"
//#include "FileUtilities/ZipArchiveReader.h"
//#include "miniz.h"

//#include "libzippp.h"

typedef int32(*_unzip)(const char* ArchivePath, const char* DestinationPath); // Declare a method to store the DLL method getInvertedBool.

_unzip m_getUnzipFromDLL;

#define LOCTEXT_NAMESPACE "FOPUSModule"
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQueueScreen::Construct(const FArguments& InArgs)
{
    // Store API key
    APIKey = InArgs._APIKey;
    SetUpFileTypes();
    LoadUnzipDLL();

	ChildSlot
	[
        SNew(SScrollBox)          

            + SScrollBox::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Left)
            .Padding(47, 10, 0, 0)
            [

                SNew(SButton)
                .Text(FText::FromString(TEXT("←")))
                .OnClicked(this, &SQueueScreen::ReturnButtonClicked)

            ]

            + SScrollBox::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Right)
            .Padding(0, -22, 47, 0)
            [

                SNew(SButton)
                .Text(FText::FromString(TEXT("Empty List 🗑️ "))) // Replace this with your desired text
                .OnClicked(this, &SQueueScreen::EmptyQButtonClicked) // Add your EmptyListButtonClicked function

            ]

            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 10, 0, 0)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                .BorderBackgroundColor(FLinearColor::Gray)
                .Padding(FMargin(2))
                [
                    SNew(SScrollBox)

                    + SScrollBox::Slot()
                    [
                        SNew(SBox)
                        .WidthOverride(550)
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
                                .FillWidth(1.2f)
                            )
                        ]
                    ]
                ]
            ]
	];
}

// --------------------------------
// --- QUEUE LOGIC METHODS --------
// --------------------------------
bool SQueueScreen::GetQueueLoopEnabled() { return IsQueueLoopEnabled; }
void SQueueScreen::SetQueueLoopEnabled(bool Enabled) { IsQueueLoopEnabled = Enabled; }
void SQueueScreen::QueueLoop()
{
    if (QueueData.Num() > 0)
    {

        // Access the JobID of the current FQueueRow object
        TSharedPtr<FQueueRow> CurrentJob = QueueData[CurrentQueueIndex];
        FString currentJobID = CurrentJob->JobID;
        FString currentJobStatus = CurrentJob->Status;

        if (currentJobStatus.Equals("PENDING") || currentJobStatus.Equals("IN_PROGRESS")) 
        {
            SendSecondAPIRequest_JobResult(currentJobID);
        }

        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
            {
                FPlatformProcess::Sleep(2);

                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        if (QueueData.Num() == 0 || !IsQueueLoopEnabled)
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

                
                FString job = tokens[0];
                FString dateTime = tokens[1];
                FString status = tokens[2];
                FString jobID = tokens[3];
                FString BatchCountString = tokens[4];

                int32 BatchCount;
                TArray<FString> DownloadLinks;
                if (FDefaultValueHelper::ParseInt(BatchCountString, BatchCount))
                {
                    for (int32 i = 0; i < BatchCount; i++)
                    {
                        DownloadLinks.Add(tokens[5 + i]);
                    }
                }
               

                QueueData.Add(MakeShareable(new FQueueRow(job, dateTime, status, jobID, BatchCount, DownloadLinks)));
                
            }

            // TODO Dont run loop when Queue screen not active
            // Refresh the ListView to reflect the new data
            QueueListView->RequestListRefresh();
        }
    }
}

TSharedRef<ITableRow> SQueueScreen::OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    FLinearColor TextColor;

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
    else if (InItem->Status == TEXT("PENDING"))
    {
        TextColor = FLinearColor::White;
    }
    else
    {
        TextColor = FLinearColor::White; // Default color
    }
    int32 CurrentIndex = QueueData.Find(InItem);
    
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
                .FillWidth(0.8f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(InItem->Status))
                    .ColorAndOpacity(TextColor)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("↡")))
                    .IsEnabled_Lambda([this, CurrentIndex]() 
                        { 
                            if (CurrentIndex >= 0 && CurrentIndex < QueueData.Num()) 
                            {
                                return QueueData[CurrentIndex]->Status == TEXT("COMPLETED");
                            }
                            return false;
                        })
                    .OnClicked_Lambda([this, CurrentIndex]() 
                        {
                            if (CurrentIndex >= 0 && CurrentIndex < QueueData.Num())
                            {
                                if (QueueData[CurrentIndex]->Status == TEXT("COMPLETED")) {

                                    NotificationHelper.ShowNotificationSuccess(LOCTEXT("Success", "Downloading component! This might take some time varying the size of the job."));

                                    for (int32 i = 0; i < QueueData[CurrentIndex]->BatchCount; i++)
                                    {
                                        DownloadAndUnzipMethod(QueueData[CurrentIndex]->DownloadLinks[i], QueueData[CurrentIndex]->DateTime, QueueData[CurrentIndex]->Job, QueueData[CurrentIndex]->BatchCount);
                                    }
                                }
                            }
                            return FReply::Handled();
                        })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("×")))
                    .OnClicked_Lambda([this, CurrentIndex]() 
                        { 
                            ShowRemoveWarningWindow(CurrentIndex);
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
        FString DownloadLinksString;
        FString BatchCount = FString::FromInt(jobData->BatchCount);
        for (FString link : jobData->DownloadLinks)
        {
            DownloadLinksString += link + " ";
        }

        fileContent += jobData->Job + " " + jobData->DateTime + " " + jobData->Status + " " + jobData->JobID + " " + BatchCount + " " + DownloadLinksString + +"\n";
    }

    // Write to file
    FFileHelper::SaveStringToFile(fileContent, *AbsolutePath);
}

FReply SQueueScreen::ShowRemoveWarningWindow(int32 QueueItemIndex)
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
                .Text(FText::FromString("Are you sure you want to remove " + QueueData[QueueItemIndex]->Job))
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
                .Text(LOCTEXT("OKButton", "Yes"))
                .OnClicked_Lambda([this, WarningWindowPtr, QueueItemIndex]()
                    {
                        if (WarningWindowPtr.IsValid())
                        {
                            QueueData.RemoveAt(QueueItemIndex);
                            WriteQueueToFile();
                            QueueListView->RequestListRefresh();
                            WarningWindowPtr->RequestDestroyWindow();
                        }
                        return FReply::Handled();
                    })
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                    .Text(LOCTEXT("OKButton", "No"))
                    .OnClicked_Lambda([this, WarningWindowPtr]()
                        {
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

// ------------------------------
// --- DOWNLOAD&UNZIP METHODS
// ------------------------------
// Returns the parent folder with all downloaded files
void SQueueScreen::DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName, int32 BatchCount)
{
    UE_LOG(LogTemp, Warning, TEXT("Initiating download from URL: %s"), *URL);
    FString DownloadedContentDir;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(URL);

    //TODO: this section must be revised to a proper try-catch structure
    HttpRequest->OnProcessRequestComplete().BindLambda([this, DateTime, JobName, BatchCount](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
#pragma push_macro("CreateDirectory")
#undef CreateDirectory

            int32 ResponseCode = Response->GetResponseCode();

            if (bWasSuccessful && Response.IsValid() && ResponseCode == 200)
            {
                // Create a "ZippedContents" folder if it does not exist
                FString ZippedContentsDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ZippedContents"));
                if (!FPaths::DirectoryExists(ZippedContentsDir))
                {
                    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*ZippedContentsDir);
                }

                // Generate dynamic folder name based on DateTime
                FString DynamicFolderName = FString::Printf(TEXT("%s-%s"), *JobName, *DateTime);

                // Define where you want to save the downloaded zip file
                FString DownloadedZipFile = FPaths::Combine(ZippedContentsDir, DynamicFolderName + TEXT(".zip"));

                UE_LOG(LogTemp, Warning, TEXT("Attempting to save downloaded file to: %s"), *DownloadedZipFile);

                // Save the downloaded zip data to the specified path
                if (FFileHelper::SaveArrayToFile(Response->GetContent(), *DownloadedZipFile))
                {
                    
                    UE_LOG(LogTemp, Warning, TEXT("File saved successfully to: %s"), *DownloadedZipFile);

                    // Define where you want to unzip the contents
                    FString UnzipDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UnzippedContents"));
                    UE_LOG(LogTemp, Warning, TEXT("Attempting to unzip to: %s"), *UnzipDirectory);

                    // Clear the UnzippedContents folder if it exists, then recreate it
                    if (FPaths::DirectoryExists(UnzipDirectory))
                    {
                        FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*UnzipDirectory);
                    }
                    FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*UnzipDirectory);


                    // Call the 7-Zip extraction function
                    if (ExtractZip(DownloadedZipFile, UnzipDirectory))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Unzip operation successful"));

                        // Once unzipped, copy the content to the Content/OPUS directory
                        FString ContentDir = FPaths::ProjectContentDir();
                        FString OPUSContentDirectory = FPaths::Combine(ContentDir, TEXT("OPUS"));

                        // Create OPUS folder if it doesn't exist
                        if (!FPaths::DirectoryExists(OPUSContentDirectory))
                        {
                            FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*OPUSContentDirectory);
                        }

                        // Extract a name from the downloaded file for the subfolder
                        FString SubFolderName = FPaths::GetBaseFilename(DownloadedZipFile);
                        FString DestinationSubFolder = FPaths::Combine(OPUSContentDirectory, SubFolderName);
                        FString ObjectSpecificDestinationDir = DestinationSubFolder;

                        // Create a subfolder with the extracted name
                        if (!FPaths::DirectoryExists(DestinationSubFolder))
                        {
                            FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*DestinationSubFolder);
                        }

                        if (BatchCount > 1)
                        {
                            TArray<FString> FbxFileNames;
                            FString FileExtension = "fbx";
                            FFileManagerGeneric::Get().FindFiles(FbxFileNames, *UnzipDirectory, *FileExtension);
                            if (FbxFileNames.Num() > 0)
                            {
                                FString FileName = FPaths::GetBaseFilename(FbxFileNames[0]);
                                ObjectSpecificDestinationDir = FPaths::Combine(DestinationSubFolder, FileName);
                            }
                        }
#pragma pop_macro("CreateDirectory")

                        // Copy the unzipped files to the OPUS sub-directory
                        FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*ObjectSpecificDestinationDir, *UnzipDirectory, true);

                        //ImportFBX(DestinationSubFolder);
                        UE_LOG(LogTemp, Warning, TEXT("Files copied to the Content/OPUS/%s directory"), *SubFolderName);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to initiate unzip operation"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to save the file to: %s"), *DownloadedZipFile);
                }
            }
            else
            {
                // Log an error
                UE_LOG(LogTemp, Error, TEXT("Failed to download file: %s"), *Response->GetContentAsString());
            }
        });

    HttpRequest->ProcessRequest();
}

bool SQueueScreen::LoadUnzipDLL()
{
    FString DLLPath = FPaths::Combine(FPaths::ProjectPluginsDir(), "OPUS/Source/UnzipDLL/unzip_library.dll");
    // If dll not in project, check engine plugins
    if (!FPaths::FileExists(DLLPath))
    {
        DLLPath = FPaths::Combine(FPaths::EnginePluginsDir(), "Marketplace/OPUS/Source/UnzipDLL/unzip_library.dll");
    }
    if (!FPaths::FileExists(DLLPath))
    {
        DLLPath = FPaths::Combine(FPaths::EnginePluginsDir(), "OPUS/Source/UnzipDLL/unzip_library.dll");
    }

    UnzipDLLHandle = FPlatformProcess::GetDllHandle(*DLLPath); // Retrieve the DLL.
    if (UnzipDLLHandle != NULL)
    {
        m_getUnzipFromDLL = NULL;
        FString procName = "extract_zip";    // Needs to be the exact name of the DLL method.
        m_getUnzipFromDLL = (_unzip)FPlatformProcess::GetDllExport(UnzipDLLHandle, *procName);
        if (m_getUnzipFromDLL != NULL)
        {
            return true;
        }
    }
    UE_LOG(LogTemp, Error, TEXT("Failed to load unzip_library.dll. This should be in the Source folder of this plugin"))
    return false;
}

void SQueueScreen::FreeUnzipDLL()
{
    m_getUnzipFromDLL = NULL;
    if (UnzipDLLHandle != NULL)
    {
        FPlatformProcess::FreeDllHandle(UnzipDLLHandle);
    }
    UnzipDLLHandle = NULL;

}
bool SQueueScreen::ExtractZip(const FString& ZipFile, const FString& DestinationDirectory)
{
    FString ArchiveFullPath = FPaths::ConvertRelativePathToFull(ZipFile);
    FString DestinationFullPath = FPaths::ConvertRelativePathToFull(DestinationDirectory);

    std::string arch = std::string(TCHAR_TO_UTF8(*ArchiveFullPath));
    std::string dest = std::string(TCHAR_TO_UTF8(*DestinationFullPath));
    m_getUnzipFromDLL(arch.c_str(), dest.c_str());
    return true;


    /*
    // Get the path to 7za.exe within the plugin's Binaries directory.
    FString PluginDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("OPUS"));
    if (!FPaths::DirectoryExists(PluginDir)) 
    {
        PluginDir = FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("OPUS"));
    }
    FString SevenZipExecutable = FPaths::Combine(PluginDir,TEXT("Source"), TEXT("ThirdParty"), TEXT("7za.exe"));

    FString CommandArgs = FString::Printf(TEXT("e \"%s\" -o\"%s\" -y"), *ZipFile, *DestinationDirectory);

    FProcHandle Handle = FPlatformProcess::CreateProc(*SevenZipExecutable, *CommandArgs, true, false, false, nullptr, 0, nullptr, nullptr);
    if (Handle.IsValid())
    {
        FPlatformProcess::WaitForProc(Handle);
        return true;
    }
    return false;
    */
    
}

void SQueueScreen::ImportFBX(const FString& ContentDirectoryPath)
{
    TArray<FString> FbxFilesInDirectory;
    FbxFilesInDirectory.Empty();
    IFileManager::Get().FindFilesRecursive(FbxFilesInDirectory, *ContentDirectoryPath, TEXT("*.fbx"), true, false, false);

    if (FbxFilesInDirectory.Num() > 0)
    {
        UObject* ImportedObject = nullptr;
        UPackage* ModelPackage = nullptr;
        FString LogMessage;
        FString FbxFilePath = FbxFilesInDirectory[0];

        // Clean path
        FbxFilePath = FbxFilePath.Replace(TEXT("\""), TEXT("/"));

        UE_LOG(LogTemp, Warning, TEXT("Importing from fbx file: %s"), *FbxFilePath);
        // Create Fbx factory and asset registry
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
        FbxFactory->AddToRoot();
        FbxFactory->EnableShowOption();

        // Create strings for model package
        FString FbxFileName = FPaths::GetBaseFilename(FbxFilePath);
        FString ModelPackageName = FbxFilePath;
        FString GamePackageName = "/Game";
        ModelPackageName.Split(TEXT("Content"), nullptr, &ModelPackageName);
        ModelPackageName = GamePackageName.Append(ModelPackageName);
        ModelPackageName = PackageTools::SanitizePackageName(ModelPackageName);

        // Load if package exists
        if (FPackageName::DoesPackageExist(ModelPackageName))
        {
            ModelPackage = LoadPackage(nullptr, *ModelPackageName, LOAD_None);
            if (ModelPackage)
            {
                ModelPackage->FullyLoad();
            }

            LogMessage = ModelPackageName + FString(" already exists!");
            UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
        }
        else
        {
            // Create packeage if doesnt exist
            ModelPackage = CreatePackage(*ModelPackageName);
            ModelPackage->FullyLoad();
        }

        // Import settings
        bool bImportCancelled = false;

        ImportedObject = FbxFactory->ImportObject(UStaticMesh::StaticClass(),
            ModelPackage, 
            FName(*FbxFileName),
            EObjectFlags::RF_Standalone | EObjectFlags::RF_Public,
            FbxFilePath,
            nullptr,
            bImportCancelled);

        if (bImportCancelled)
        {
            if (ModelPackage)
            {
                // @todo clean up created package
                // ModelPackage->ConditionalBeginDestroy();
            }
        }

        // @todo save package
        FbxFactory->RemoveFromRoot();
        FbxFactory->ConditionalBeginDestroy();
        FbxFactory = nullptr;
    }
}

// --------------------------------
// --- BUTTON CALLBACK METHODS ----
// --------------------------------
FReply SQueueScreen::ReturnButtonClicked()
{
    OnCreationScreenEnabledDelegate.Broadcast();
    return FReply::Handled();
}

FReply SQueueScreen::EmptyQButtonClicked()
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
                .Text(LOCTEXT("Warning Deleting Jobs", "YOU ARE DELETING ALL THE JOBS.\nYou will not be able to download them again.\nAre you sure you want to continue?"))
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
                        .OnClicked_Lambda([this, WarningWindowPtr]() 
                            {
                                if (WarningWindowPtr.IsValid())
                                {
                                    WarningWindowPtr->RequestDestroyWindow();
                                }

                                // Construct the full path to the queue.txt file
                                FString ProjectDir = FPaths::ProjectSavedDir();
                                FString QueueFile = FPaths::Combine(ProjectDir, TEXT("queue.txt"));

                                // Empty the queue.txt file
                                FFileHelper::SaveStringToFile(TEXT(""), *QueueFile);

                                return FReply::Handled();
                            })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(LOCTEXT("NoButton", "No"))
                        .OnClicked_Lambda([WarningWindowPtr]() 
                            {
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
    FString URL = URLHelper::GetJobResults;
    FString Parameters = "?result_uid=" + EncodedJobID;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(URL + Parameters);
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindLambda([this, jobID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        OnSecondAPIRequestJobResultCompleted(Request, Response, bWasSuccessful, jobID);
    });
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Error, TEXT("Reached the end of request"));
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(URL + Parameters));
}

void SQueueScreen::OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID)
{
    int32 ResponseCode = Response->GetResponseCode();
    if (bWasSuccessful && Response.IsValid() && ResponseCode == 200)
    {
        FString ResponseStr = Response->GetContentAsString();

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString status;
            if (JsonObject->TryGetStringField("overall_status", status))
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
                    // When you create batch job implementatıon make this store all links cleanly
                    TArray<TSharedPtr<FJsonValue>> ResultsArray = JsonObject->GetArrayField("results");
                    if (ResultsArray.IsValidIndex(0))
                    {
                        for (int32 i = 0; i < ResultsArray.Num(); i++) 
                        {
                            TSharedPtr<FJsonValue> Result = ResultsArray[i];
                            TSharedPtr<FJsonObject> urlsObject = Result->AsObject()->GetObjectField("urls");
                            if (urlsObject.IsValid())
                            {

                                for (TSharedPtr<FString> CurrentFileType : AvailableFileTypes)
                                {
                                    FString link;

                                    if (urlsObject->TryGetStringField(*CurrentFileType, link))
                                    {
                                        SecondAPILink = link;
                                        UE_LOG(LogTemp, Warning, TEXT("API link: %s for file type %s"), *SecondAPILink, **CurrentFileType);

                                        for (auto& jobData : QueueData)
                                        {
                                            if (jobData->JobID == jobID)
                                            {
                                                jobData->DownloadLinks[i] = link;
                                                break;
                                            }
                                        }

                                        WriteQueueToFile();
                                        QueueListView->RequestListRefresh();
                                        // just gets link for one file type
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Urls object is not valid"));
                            }
                        }
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

void SQueueScreen::SetAPIKey(FString apiKey) { APIKey = apiKey; }

// Helper methods 

void SQueueScreen::SetUpFileTypes()
{
    AvailableFileTypes.Empty();
    AvailableFileTypes.Add(MakeShareable(new FString("fbx")));
    //AvailableFileTypes.Add(MakeShareable(new FString("gltf")));
    //AvailableFileTypes.Add(MakeShareable(new FString("usd")));
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE