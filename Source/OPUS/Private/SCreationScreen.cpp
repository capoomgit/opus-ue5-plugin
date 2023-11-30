// Fill out your copyright notice in the Description page of Project Settings.


#include "SCreationScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"
#include "SFilteredSelectionTextBox.h"
#include "URLHelper.h"
#include "SCustomizationTable.h"

// Libraries
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Http.h"
#include "Async/Async.h"
#include "Misc/DefaultValueHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h" 
#include "Serialization/JsonSerializer.h" 

#define LOCTEXT_NAMESPACE "FOPUSModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCreationScreen::Construct(const FArguments& InArgs)
{
    APIKey = InArgs._APIKey;
    CurrentModel = MakeShared<FString>(TEXT("Loading..."));
    ParameterFilteredSuggestions.Empty();
    ParameterSuggestions.Empty();
    TagFilteredSuggestions.Empty();
    SetUpFileTypeComboBox();
    SetUpTextureSizeComboBox();

    ChildSlot
    [
        SNew(SScrollBox)

        // Logout button
        + SScrollBox::Slot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Right)
        .Padding(0, 0, 40, 0)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 0, 20, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("JobQueueButton", "Job Queue ↡"))
                .OnClicked(this, &SCreationScreen::QueueButtonClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("LogoutButton", "Logout 🚪"))
                .OnClicked(this, &SCreationScreen::LogoutButtonClicked)
            ]
        ]

        + SScrollBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        [     
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 0, 200, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Select Model", "Please select a model to generate."))
                ]

                // Component selection combo box
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 30, 0)
                [

                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBox)
                        .WidthOverride(120)
                        [
                            SAssignNew(ModelComboBox, SComboBox<TSharedPtr<FString>>)
                            .OptionsSource(&ModelOptions)
                            .OnGenerateWidget(this, &SCreationScreen::GenerateComboBoxItem)
                            .OnSelectionChanged(this, &SCreationScreen::ModelComboBoxSelectionChanged)
                            .InitiallySelectedItem(CurrentModel)
                            .ContentPadding(FMargin(2.0f))
                            [
                                SNew(STextBlock)
                                .Text(this, &SCreationScreen::GetCurrentModel)
                            ]
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(30, 0, 0, 0)
                    [

                        SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("File Type", "File type"))
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SAssignNew(FileTypeComboBox, SComboBox<TSharedPtr<FString>>)
                            .OptionsSource(&AvailableFileTypes)
                            .OnGenerateWidget(this, &SCreationScreen::GenerateComboBoxItem)
                            .OnSelectionChanged(this, &SCreationScreen::FileTypeComboBoxSelectionChanged)
                            .InitiallySelectedItem(CurrentFileType)
                            [
                                SNew(STextBlock)
                                .Text(this, &SCreationScreen::GetCurrentFileType)
                            ]
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 20, 0, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("Texture Size", "Texture size"))
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SAssignNew(TextureSizeComboBox, SComboBox<TSharedPtr<FString>>)
                            .OptionsSource(&AvailableTextureSizes)
                            .OnGenerateWidget(this, &SCreationScreen::GenerateComboBoxItem)
                            .OnSelectionChanged(this, &SCreationScreen::TextureSizeComboBoxSelectionChanged)
                            .InitiallySelectedItem(CurrentTextureSize)
                            [
                                SNew(STextBlock)
                                .Text(this, &SCreationScreen::GetCurrentTextureSize)
                            ]
                        ]
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(60, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(135)
                        .HeightOverride(100)
                        [
                            SNew(SImage)
                            .Image(FOPUSStyle::Get().GetBrush("OPUS.SmallLogo"))
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Select preset", "Select a model preset"))
                ]

                // Tag Search Box
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SAssignNew(TagSearchBox, SFilteredSelectionTextBox)
                    .ListItemsSource(&TagFilteredSuggestions)
                    .OnTextChanged(this, &SCreationScreen::OnTagsSearchTextChanged)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Parameter Customization", "Parameter customizaztion"))
                ]

                // Parameter Search Box
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SAssignNew(ParameterSearchBox, SFilteredSelectionTextBox)
                    .ListItemsSource(&ParameterFilteredSuggestions)
                    .OnTextChanged(this, &SCreationScreen::OnParamSearchTextChanged)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SAssignNew(CustomizationTable, SCustomizationTable)
                ]

                + SVerticalBox::Slot()
                .VAlign(VAlign_Bottom)
                .HAlign(HAlign_Center)
                .AutoHeight()
                .Padding(0, 10, 0, 0)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Left)
                    .AutoWidth()
                    .Padding(30, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(115)  // Adjusted button width
                        .HeightOverride(50)  // Adjusted button height
                        [
                            SNew(SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("ResetFeaturesButton", "Reset All"))
                            .OnClicked(this, &SCreationScreen::ResetFeaturesButtonClicked)
                            .ButtonColorAndOpacity(FLinearColor(1, 0.3, 0.3, 1))

                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .AutoWidth()
                    .Padding(30, 0, 30, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(130)
                        .HeightOverride(50)
                        [
                            SAssignNew(CreateButton, SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("Generate Model", "Generate Model"))
                            .OnClicked(this, &SCreationScreen::CreateButtonClicked)
                            .ButtonColorAndOpacity(FLinearColor(0.3, 1, 0.3, 1))
                        ]
                    ]
                ]
            ]
        ]
    ];

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
        {
            FPlatformProcess::Sleep(1);

            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    SendForthAPIRequest_ModelNames();
                });
        });

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
        {
            FPlatformProcess::Sleep(3);

            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    SendThirdAPIRequest_AttributeName();
                });
        });
	
    TagSearchBox->OnSuggestionSelected.AddRaw(this, &SCreationScreen::OnTagSelected);
    ParameterSearchBox->OnSuggestionSelected.AddRaw(this, &SCreationScreen::OnParameterSelected);
}

// ------------------------------
// --- BUTTON CALLBACK METHODS
// ------------------------------

FReply SCreationScreen::LogoutButtonClicked()
{
    APIKey = "";
    OnLogoutDelegate.Broadcast();
    return FReply::Handled();
}

FReply SCreationScreen::ResetFeaturesButtonClicked()
{
    ShowResetCustomizationWarning();
    return FReply::Handled();
}

FReply SCreationScreen::QueueButtonClicked()
{
    // Enable screen by broadcasting delegate
    OnQueueScreenEnabledDelegate.Broadcast();

    return FReply::Handled();
}

// TODO Make create button send request and give positive feedback.
FReply SCreationScreen::CreateButtonClicked()
{
    SendAPIRequest_Create();
    return FReply::Handled();
}


// ------------------------------
// --- COMBO BOX METHODS
// ------------------------------

TSharedRef<SWidget> SCreationScreen::GenerateComboBoxItem(TSharedPtr<FString> Item)
{
    return SNew(STextBlock).Text(FText::FromString(*Item));
}

void SCreationScreen::ModelComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
    if (NewItem.IsValid())
    {
        CurrentModel = NewItem;

        // Clear the filtered suggestion lists (assuming you still want this)
        ParameterFilteredSuggestions.Empty();
        TagFilteredSuggestions.Empty();

        CustomizationTable->ResetTable();

        // Make an API call with the new item
        SendThirdAPIRequest_AttributeName();

        // Refresh the list view
        ParameterSearchBox->RequestListRefresh();
        ParameterSearchBox->Invalidate(EInvalidateWidget::Layout);

        TagSearchBox->RequestListRefresh();
        TagSearchBox->Invalidate(EInvalidateWidget::Layout);
    }
}

void SCreationScreen::FileTypeComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
    CurrentFileType = NewItem;
}

void SCreationScreen::TextureSizeComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
    CurrentTextureSize = NewItem;
}

void SCreationScreen::SetUpFileTypeComboBox()
{
    AvailableFileTypes.Empty();
    AvailableFileTypes.Add(MakeShareable(new FString("fbx")));
    //AvailableFileTypes.Add(MakeShareable(new FString("gltf")));
    //AvailableFileTypes.Add(MakeShareable(new FString("usd")));
    CurrentFileType = AvailableFileTypes[0];
}

void SCreationScreen::SetUpTextureSizeComboBox()
{
    AvailableTextureSizes.Empty();
    AvailableTextureSizes.Add(MakeShareable(new FString("1024")));
    AvailableTextureSizes.Add(MakeShareable(new FString("2048")));
    CurrentTextureSize = AvailableTextureSizes[0];
}

// ------------------------------
// --- SEARCHBOX METHODS
// ------------------------------

void SCreationScreen::OnTagsSearchTextChanged(const FText& NewText)
{
    // Clear previous suggestions
    TagFilteredSuggestions.Empty();
    FString CurrentInput = NewText.ToString();
    
    // loop through tags list
    for (const TSharedPtr<FAssetTag>& CurrentAssetTag : TagsList)
    {
        bool TagCategoryAlreadyAdded = false;
        if (CurrentAssetTag->Tag.Contains(CurrentInput))
        {
            // Loop through rows in the customizations table
            for (const TSharedPtr<FKeywordTableRow> Row : CustomizationTable->TableRows)
            {
                // Check if subcategory tag already assigned
                if (Row->Customization->Equals(CurrentAssetTag->SubCategory))
                {
                    TagCategoryAlreadyAdded = true;
                    break;
                }
            }

            if (!TagCategoryAlreadyAdded)
            {
                TSharedPtr<FString> TagDisplayString = MakeShared<FString>(CurrentAssetTag->SubCategory + " - " + CurrentAssetTag->Tag);
                if (!TagExistsInFilteredList(TagDisplayString))
                {
                    TagFilteredSuggestions.Add(TagDisplayString);
                }
            }
        }
    }

    // Refresh the tags suggestion list view
    TagSearchBox->RequestListRefresh();
}

void SCreationScreen::OnParamSearchTextChanged(const FText& NewText)
{
    ParameterFilteredSuggestions.Empty();

    FString CurrentInput = NewText.ToString();
    
    for (const TSharedPtr<FString>& Suggestion : ParameterSuggestions)
    {
        if (Suggestion->Contains(CurrentInput))
        {
            ParameterFilteredSuggestions.Add(Suggestion);
        }
    }

    // Refresh the list view
    ParameterSearchBox->RequestListRefresh();
}

void SCreationScreen::OnTagSelected(TSharedPtr<FString> TagSelection)
{
    SelectedTagSuggestion = TagSelection;
    CustomizationTable->AddCustomizationToTable(TagSelection, false);
}

void SCreationScreen::OnParameterSelected(TSharedPtr<FString> ParameterSelection)
{
    SelectedParameterSuggestion = ParameterSelection;
    CustomizationTable->AddCustomizationToTable(ParameterSelection, true);
}

// ------------------------------
// --- HELPER METHODS
// ------------------------------

void SCreationScreen::SetAPIKey(FString apiKey) { APIKey = apiKey; }

FText SCreationScreen::GetCurrentModel() const
{
    if (CurrentModel.IsValid())
    {
        return FText::FromString(*CurrentModel);
    }
    return FText::GetEmpty();
}

FText SCreationScreen::GetCurrentFileType() const
{
    if (CurrentFileType.IsValid())
    {
        return FText::FromString(*CurrentFileType);
    }
    return FText::GetEmpty();
}

FText SCreationScreen::GetCurrentTextureSize() const
{
    if (CurrentTextureSize.IsValid())
    {
        return FText::FromString(*CurrentTextureSize);
    }
    return FText::GetEmpty();
}


FText SCreationScreen::GetParamHintText(FVector2D ParameterRange) const
{
    if (ParameterRange != FVector2D::ZeroVector)
    {
        FString RangeLow = FString::SanitizeFloat(ParameterRange.X);
        FString RangeHigh = FString::SanitizeFloat(ParameterRange.Y);
        return FText::Format(LOCTEXT("ParamRangeHint", "Range: {0} - {1}"), FText::FromString(RangeLow), FText::FromString(RangeHigh));
    }
    else
    {
        return LOCTEXT("Param Value", "Parameter value");
    }
}

FReply SCreationScreen::ShowWarningWindow(FString warningMessage)
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
            .Text(FText::FromString(warningMessage))
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
                    .Text(LOCTEXT("OKButton", "OK"))
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

FReply SCreationScreen::ShowResetCustomizationWarning()
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
                .Text(FText::FromString("You are reseting all current customizations, are you sure?"))
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
                                    CustomizationTable->ResetTable();
                                }
                                return FReply::Handled();
                            })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(LOCTEXT("NoButton", "No"))
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

FString SCreationScreen::ConstructCreateRequestBody()
{
    TMap<FString, TArray<FString>> KeywordMap;

    // Populate the KeywordMap
    for (TSharedPtr<FKeywordTableRow>& row : CustomizationTable->TableRows)
    {
        if (!IsCustomization(*row->Customization))  // Check if it is not a parameter
        {
            KeywordMap.FindOrAdd(*row->Customization).Add(*row->Value);
        }
    }

    // Start constructing the JSON data
    FString JsonData = "{\r\n";
    JsonData += "    \"name\": \"" + *CurrentModel + "\",\r\n";
    JsonData += "    \"texture_resolution\": \"" + *CurrentTextureSize + "\",\r\n";
    JsonData += "    \"extensions\": [\r\n        \"" + *CurrentFileType + "\"\r\n    ],\r\n";

    // Constructing parameters (No changes here)
    JsonData += "    \"parameters\": {\r\n";
    bool isFirstParameter = true;
    for (TSharedPtr<FKeywordTableRow>& row : CustomizationTable->TableRows)
    {
        if (IsCustomization(*row->Customization))  // Check if it is a parameter
        {
            if (!isFirstParameter)
            {
                JsonData += ",\r\n";
            }
            isFirstParameter = false;
            JsonData += "        \"" + *row->Customization + "\": " + *row->Value;
        }
    }
    JsonData += "\r\n    },\r\n";

    // Constructing keywords
    JsonData += "    \"keywords\": {\r\n";
    bool isFirstKey = true;

    for (auto& pair : KeywordMap)
    {
        if (!isFirstKey)
        {
            JsonData += ",\r\n";
        }
        isFirstKey = false;

        JsonData += "        \"" + pair.Key + "\": [";
        for (int i = 0; i < pair.Value.Num(); ++i)
        {
            JsonData += "\"" + pair.Value[i] + "\"";
            if (i < pair.Value.Num() - 1)
            {
                JsonData += ", ";
            }
        }
        JsonData += "]";
    }

    JsonData += "\r\n    }\r\n";
    JsonData += "}";
    return JsonData;
}

bool SCreationScreen::IsCustomization(const FString& Keyword)
{
    return Keyword.Contains("/");  // Adjust this if the criteria change
}

bool SCreationScreen::TagExistsInFilteredList(TSharedPtr<FString> TagString)
{
    for (const TSharedPtr<FString> CurrentTag : TagFilteredSuggestions)
    {
        FString TagInList = *CurrentTag;
        FString TagToSearch = *TagString;
        if (TagInList.Equals(TagToSearch)) 
        {
            return true;
        }
    }
    return false;
}

// ------------------------------
// --- API REQUEST METHODS
// ------------------------------

void SCreationScreen::SendAPIRequest_Create()
{
    // Disable create button
    CreateButton->SetEnabled(false);
    FString Url = URLHelper::CreateBatchComponent;
    // When you want to create more objects using create batch
    FString Parameters = "?count=1";
    Url += Parameters;

    for (const auto& structure : Structures)
    {
        if (structure->Equals(*CurrentModel))
        {
            // Change the URL for structures
            Url = URLHelper::CreateStructure;
            break;
        }
    }

    FString JsonData = ConstructCreateRequestBody();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->SetContentAsString(JsonData);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnAPIRequestCreateCompleted);
    HttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Sending Create HTTP request: %s"), *JsonData);
    NotificationHelper.ShowNotificationPending(FText::FromString("Creating Job"));
}

void SCreationScreen::OnAPIRequestCreateCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    // Enable create button
    CreateButton->SetEnabled(true);

    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        int32 ResponseCode = Response->GetResponseCode();
        FString ResponseStr = Response->GetContentAsString();

        // Parse the JSON response
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString status;
            if (JsonObject->TryGetStringField("overall_status", status))
            {
                UE_LOG(LogTemp, Warning, TEXT("Job status: %s"), *status);
                NewJobStatus = status;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to get job status"));
            }

            FString JobID;
            if (JsonObject->TryGetStringField("batch_job_id", JobID))
            {
                APIResponseID = JobID;

                UE_LOG(LogTemp, Warning, TEXT("HTTP Request successful. Response Code: %d, Response Data: %s"), ResponseCode, *ResponseStr);
                UE_LOG(LogTemp, Warning, TEXT("Response Job ID: %s"), *APIResponseID);

                NotificationHelper.ShowNotificationSuccess(LOCTEXT("IDSuccess", "Job ID succesfully created!!"));

                // Write to file
                FString SaveDirectory = FPaths::ProjectSavedDir();
                FString FileName = FString(TEXT("queue.txt"));
                FString AbsolutePath = SaveDirectory + "/" + FileName;

                FString CurrentDateTime = FDateTime::Now().ToString(TEXT("%Y-%m-%d-%H-%M-%S"));
                FString TextToSave = *CurrentModel + TEXT(" ") + CurrentDateTime + TEXT(" ") + NewJobStatus + TEXT(" ") + *APIResponseID;

                // Append to file. This will create the file if it doesn't exist.
                FFileHelper::SaveStringToFile(TextToSave + LINE_TERMINATOR, *AbsolutePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

                OnQueueScreenEnabledDelegate.Broadcast();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("API Response does not contain \"result_id\" field."));
                NotificationHelper.ShowNotificationFail(LOCTEXT("Failed", "Failed to create ID.  Please check logs and try again."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
            NotificationHelper.ShowNotificationFail(LOCTEXT("Failed", "Failed to create ID. Please check logs and try again."));
        }
    }
    else
    {
        NotificationHelper.ShowNotificationFail(LOCTEXT("Failed", "Failed to create ID. Please check logs and try again."));
        ShowWarningWindow("Failed to create ID. Please try again later.");
        UE_LOG(LogTemp, Error, TEXT("HTTP Create Model Request failed."));
        if (Response.IsValid())
        {
            int32 ResponseCode = Response->GetResponseCode();
            FString ResponseStr = Response->GetContentAsString();
            UE_LOG(LogTemp, Error, TEXT("HTTP Response Code: %d"), ResponseCode);
            UE_LOG(LogTemp, Error, TEXT("HTTP Response Data: %s"), *ResponseStr);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HTTP Response is invalid."));
        }
    }
}

void SCreationScreen::SendThirdAPIRequest_AttributeName()
{
    FString Url = URLHelper::GetAttributesWithName;
    FString Parameters = "?name=" + *CurrentModel;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url + Parameters);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnThirdAPIRequestAttributeNameCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url + Parameters));

    // Give feedback
    TagSearchBox->SelectionsLoadingStarted();
    ParameterSearchBox->SelectionsLoadingStarted();
}

void SCreationScreen::OnThirdAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("Inside OnThirdAPIRequestAttributeNameCompleted. bWasSuccessful: %s, Response Code: %d"), bWasSuccessful ? TEXT("True") : TEXT("False"), Response.IsValid() ? Response->GetResponseCode() : -1);

    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        // Parse JSON response
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        MainCategoryKeysList.Empty();
        TagsList.Empty();
        ParameterSuggestions.Empty();

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("JSON Deserialization successful."));

            // Loop over main categories like "Stair"
            for (const auto& MainCategoryPair : JsonObject->Values)
            {
                TSharedPtr<FJsonObject> SubCategories = MainCategoryPair.Value->AsObject();
                UE_LOG(LogTemp, Warning, TEXT("Main Category: %s"), *MainCategoryPair.Key);

                // Loop over sub-categories like "stair_upper"
                for (const auto& SubCategoryPair : SubCategories->Values)
                {
                    FString SubCategoryKey = SubCategoryPair.Key;
                    MainCategoryKeysList.Add(MakeShared<FString>(SubCategoryKey));
                    UE_LOG(LogTemp, Warning, TEXT("SubCategoryKey: %s"), *SubCategoryKey);

                    TSharedPtr<FJsonObject> CategoryObj = SubCategoryPair.Value->AsObject();
                    const TArray<TSharedPtr<FJsonValue>>* AssetsArray;
                    if (CategoryObj->TryGetArrayField("assets", AssetsArray))
                    {
                        for (const auto& AssetValue : *AssetsArray)
                        {
                            TSharedPtr<FJsonObject> AssetObj = AssetValue->AsObject();
                            FString AssetName;

                            if (AssetObj->HasField("name"))
                            {
                                AssetName = AssetObj->GetStringField("name");
                            }

                            // Access the asset tags
                            const TArray<TSharedPtr<FJsonValue>>* TagsArray;
                            if (AssetObj->TryGetArrayField("tags", TagsArray))
                            {
                                for (const auto& TagValue : *TagsArray)
                                {
                                    FString TagString = TagValue->AsString();

                                    FAssetTag NewAssetTag;
                                    NewAssetTag.SubCategory = SubCategoryKey;
                                    NewAssetTag.AssetName = AssetName;
                                    NewAssetTag.Tag = TagString;

                                    TagsList.Add(MakeShared<FAssetTag>(NewAssetTag));

                                    TSharedPtr<FString> TagDisplayString = MakeShared<FString>(NewAssetTag.SubCategory + " - " + NewAssetTag.Tag);
                                    if (!TagExistsInFilteredList(TagDisplayString))
                                    {
                                        TagFilteredSuggestions.Add(TagDisplayString);
                                    }
                                }
                            }

                            // Access the "templates"
                            const TArray<TSharedPtr<FJsonValue>>* TemplatesArray;
                            if (AssetObj->TryGetArrayField("templates", TemplatesArray))
                            {
                                for (const auto& TemplateValue : *TemplatesArray)
                                {
                                    FString TemplateString = TemplateValue->AsString();

                                    // Adding templates to the TagsList similar to how tags were added
                                    FAssetTag NewAssetTag;
                                    NewAssetTag.SubCategory = SubCategoryKey;
                                    NewAssetTag.AssetName = AssetName;
                                    NewAssetTag.Tag = TemplateString;  // Using Tag member variable to store the template

                                    TagsList.Add(MakeShared<FAssetTag>(NewAssetTag));

                                    TSharedPtr<FString> TagDisplayString = MakeShared<FString>(NewAssetTag.SubCategory + " - " + NewAssetTag.Tag);
                                    if (!TagExistsInFilteredList(TagDisplayString))
                                    {
                                        TagFilteredSuggestions.Add(TagDisplayString);
                                    }
                                }
                            }

                            // Access the "parameters"
                            const TArray<TSharedPtr<FJsonValue>>* ParametersArray;
                            if (AssetObj->TryGetArrayField("parameters", ParametersArray))
                            {
                                for (const auto& ParameterValue : *ParametersArray)
                                {
                                    TSharedPtr<FJsonObject> ParameterObj = ParameterValue->AsObject();
                                    if (ParameterObj->HasField("attribute"))
                                    {
                                        FString ParameterName = ParameterObj->GetStringField("name");
                                        FString AssetParameterCombination = FString::Printf(TEXT("%s/%s"), *AssetName, *ParameterName);
                                        ParameterSuggestions.Add(MakeShared<FString>(AssetParameterCombination));
                                        ParameterFilteredSuggestions.Add(MakeShared<FString>(AssetParameterCombination));

                                        // Extract the range of the parameters if available
                                        if (ParameterObj->HasField("range"))
                                        {
                                            const TArray<TSharedPtr<FJsonValue>>* RangeArray;
                                            if (ParameterObj->TryGetArrayField("range", RangeArray))
                                            {
                                                // Extract the first two range values
                                                float MinValue = (*RangeArray)[0]->AsNumber();
                                                float MaxValue = (*RangeArray)[1]->AsNumber();

                                                // Store this in the map
                                                CustomizationTable->ParameterRanges.Add(AssetParameterCombination, FVector2D(MinValue, MaxValue));

                                                for (const auto& RangeValue : *RangeArray)
                                                {
                                                    double Value = RangeValue->AsNumber();
                                                    UE_LOG(LogTemp, Warning, TEXT("Range Value: %f"), Value);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Give feedback
    TagSearchBox->SelectionsLoadingComplete();
    ParameterSearchBox->SelectionsLoadingComplete();
}

void SCreationScreen::SendForthAPIRequest_ModelNames()
{
    FString Url = URLHelper::GetModels;
    FString JsonData = "";

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->SetContentAsString(JsonData);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnForthAPIRequestModelNamesCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url));
}

void SCreationScreen::OnForthAPIRequestModelNamesCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            ModelOptions.Empty(); // Clear previous options
            Structures.Empty();     // Clear previous structures

            // Parse Structures
            const TArray<TSharedPtr<FJsonValue>>* StructuresArray;
            if (JsonObject->TryGetArrayField("Structures", StructuresArray))
            {
                for (const auto& Value : *StructuresArray)
                {
                    if (Value.IsValid() && Value->Type == EJson::String)
                    {
                        TSharedPtr<FString> structure = MakeShared<FString>(Value->AsString());
                        ModelOptions.Add(structure);
                        Structures.Add(structure); // Add to the Structures array
                    }
                }
            }

            // Parse Components
            const TArray<TSharedPtr<FJsonValue>>* ComponentsArray;
            if (JsonObject->TryGetArrayField("Components", ComponentsArray))
            {
                for (const auto& Value : *ComponentsArray)
                {
                    if (Value.IsValid() && Value->Type == EJson::String)
                    {
                        ModelOptions.Add(MakeShared<FString>(Value->AsString()));
                    }
                }
            }

            // Update the ComboBox
            if (ModelComboBox.IsValid())
            {
                ModelComboBox->RefreshOptions();
            }

            // Set the initial selected item
            if (ModelOptions.Num() > 0)
            {
                CurrentModel = ModelOptions[0];
                ModelComboBox->SetSelectedItem(CurrentModel);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse the JSON response."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("API request was not successful. Response code: %d, Error: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE
