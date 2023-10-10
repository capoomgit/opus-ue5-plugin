#include "GUI.h"
#include "GUIStyle.h"
#include "GUICommands.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h" 
#include "Serialization/JsonSerializer.h" 
#include "Async/Async.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Internationalization/Text.h"
#include "Modules/ModuleManager.h"
#include "Misc/DefaultValueHelper.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName GUITabName("GUI");

#define LOCTEXT_NAMESPACE "FGUIModule"

SGUIWidget::SGUIWidget()
    :
    bIsLoggedIn(false)
{
    FilteredSuggestions.Empty();

}

void SGUIWidget::Construct(const FArguments& InArgs)
{
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));

    if (FPaths::FileExists(SaveFilePath))
    {
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

        // If the file exists, consider the user as logged in and go directly to main GUI
        bIsLoggedIn = true;
    }
    if (!bIsLoggedIn) //Login screen
    {
        ChildSlot
        [
            SNew(SOverlay)

            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(-35, -150, 0, 0)  // Move the image up
            [
                SNew(SImage)
                .Image(FGUIStyle::Get().GetBrush("GUI.APILogo"))
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(0, 0, 50, 0)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                        .BorderBackgroundColor(FLinearColor::White)
                        .Padding(FMargin(5.0f))
                        [
                            SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("RapidKeyLabel", "Rapid Key: "))
                            ]

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SBox)
                                .WidthOverride(350)
                                [
                                    SAssignNew(KeyField, SEditableText)
                                    .HintText(LOCTEXT("KeyHint", "                              Please enter your rapid key"))
                                    .OnTextCommitted(this, &SGUIWidget::OnTextCommittedInKeyField)
                                ]
                            ]
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBox)
                        .WidthOverride(80)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("LoginButton", "Login"))
                            .OnClicked(this, &SGUIWidget::LoginButtonClicked)
                        ]
                    ]
                ]
            ]
        ];
    }
    else if (bIsQueueClicked)///Queue screen
    {
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);
        ChildSlot
        [
            SNew(SOverlay)

            + SOverlay::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Left)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("←")))
                .OnClicked(this, &SGUIWidget::ReturnButtonClicked)
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
                    .OnClicked(this, &SGUIWidget::EmptyQButtonClicked) // Add your EmptyListButtonClicked function
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(10, 0, 0, 0) // Add padding between buttons
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("↺ Empty Caching")))
                    .OnClicked(this, &SGUIWidget::RefreshCachingButtonClicked)
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
                            .OnGenerateRow(this, &SGUIWidget::OnGenerateRowForList)
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
    else //Main screen
    {
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

        CurrentItem = MakeShared<FString>(TEXT("Loading..."));
        FilteredSuggestions.Empty();
        ParameterSuggestions.Empty();

        ChildSlot
        [
            SNew(SOverlay)

            + SOverlay::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Right)
            [
                SNew(SButton)
                .Text(LOCTEXT("LogoutButton", "Logout 🚪"))
                .OnClicked(this, &SGUIWidget::LogoutButtonClicked)
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Right)
            .Padding(0, 0, 95, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("QueueButton", "Queue ↡"))
                .OnClicked(this, &SGUIWidget::QueueButtonClicked)
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Fill)
            .HAlign(HAlign_Fill)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 0, 200, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("SomeText", "Please select a component from the menu."))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 20, 0, 0)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(30, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(120)
                        [
                            SAssignNew(ComboBoxWidget, SComboBox<TSharedPtr<FString>>)
                            .OptionsSource(&ComboBoxOptions)
                            .OnGenerateWidget(this, &SGUIWidget::GenerateComboBoxItem)
                            .OnSelectionChanged(this, &SGUIWidget::ComboBoxSelectionChanged)
                            .InitiallySelectedItem(CurrentItem)
                            .ContentPadding(FMargin(2.0f))
                            [
                                SNew(STextBlock)
                                .Text(this, &SGUIWidget::GetCurrentItem)
                            ]
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(60, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(120)
                        [
                            SNew(SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("Button", "Create ⚙️"))
                            .OnClicked(this, &SGUIWidget::CreateButtonClicked)
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SSpacer)
                        .Size(FVector2D(95, 0))
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(0, 0, 100, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(300)
                        .HeightOverride(77)
                        [
                            SNew(SImage)
                            .Image(FGUIStyle::Get().GetBrush("GUI.MyImage"))
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("NewSearchLabel", "Search for Tags"))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    .VAlign(VAlign_Top)
                    [
                        SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                            .BorderBackgroundColor(FLinearColor::White)
                            .Padding(FMargin(5.0f))
                            [
                                SAssignNew(TagsSearchBox, SEditableText)
                                .OnTextChanged(this, &SGUIWidget::OnTagsSearchTextChanged)
                            ]
                        ]

                         + SVerticalBox::Slot()
                        .MaxHeight(111) // Set a maximum height for the suggestions box, adjust as needed
                        [
                            SNew(SScrollBox)

                            + SScrollBox::Slot()
                            [
                                SAssignNew(TagsSuggestionsListView, SListView<TSharedPtr<FString>>)
                                .ItemHeight(24)
                                .ListItemsSource(&TagFilteredSuggestions)
                                .OnGenerateRow(this, &SGUIWidget::GenerateTagSuggestionRow)
                                .Visibility(EVisibility::Collapsed)
                                .SelectionMode(ESelectionMode::Single)
                            ]
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("SearchLabel", "Search for Parameters"))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                    .BorderBackgroundColor(FLinearColor::White)
                    .Padding(FMargin(5.0f))
                    [
                        SAssignNew(SearchBox, SEditableText)
                        .OnTextChanged(this, &SGUIWidget::OnParamSearchTextChanged)
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SNew(SScrollBox)

                    + SScrollBox::Slot()
                    [
                        SNew(SBox)
                        .HeightOverride(111) // Adjust this based on your needs
                        [
                            SAssignNew(SuggestionsListView, SListView<TSharedPtr<FString>>)
                            .ItemHeight(24)
                            .ListItemsSource(&FilteredSuggestions)
                            .OnGenerateRow(this, &SGUIWidget::GenerateParamSuggestionRow)
                            .Visibility(EVisibility::Collapsed)
                            .SelectionMode(ESelectionMode::Single)
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 649, 0)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10, 0, 10, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(130)  // Adjusted width
                        .HeightOverride(80)  // Adjusted height
                        [
                            SAssignNew(ParamInputBox, SEditableTextBox)
                            .HintText(this, &SGUIWidget::GetParamHintText)
                            .Visibility(this, &SGUIWidget::GetParamInputBoxVisibility)
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(50, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(120)  // Adjusted button width
                        .HeightOverride(80)  // Adjusted button height
                        [
                            SNew(SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("ApplyFeatureButton", "Apply 🛠️"))
                            .OnClicked(this, &SGUIWidget::ApplyFeatureButtonClicked)
                            .Visibility(EVisibility::Visible) // Always visible
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(30, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(120)
                        .HeightOverride(80)
                        [
                            SNew(SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("ResetFeaturesButton", "Reset  🧹"))
                            .OnClicked(this, &SGUIWidget::ResetFeaturesButtonClicked)
                            .Visibility(EVisibility::Visible) // Always visible
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(110, 0, 10, 0)
                    [
                        SNew(SCheckBox)
                        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                        if (NewState == ECheckBoxState::Checked) {
                            bIsFBXSelected = true;
                            bIsGLTFSelected = false;
                        }
                        })
                        .IsChecked_Lambda([this]() -> ECheckBoxState {
                            return bIsFBXSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                        })
                        [
                            SNew(STextBlock).Text(LOCTEXT("FBX", "FBX"))
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(15, 0, 10, 0)
                    [
                        SNew(SCheckBox)
                        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                            if (NewState == ECheckBoxState::Checked) {
                                bIsGLTFSelected = true;
                                bIsFBXSelected = false;
                            }
                        })
                        .IsChecked_Lambda([this]() -> ECheckBoxState {
                            return bIsGLTFSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                        })
                        [
                            SNew(STextBlock).Text(LOCTEXT("GLTF", "GLTF"))
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SNew(SScrollBox)
                    + SScrollBox::Slot()
                    [
                        SNew(SBox)
                        .WidthOverride(100) // Adjust this based on your needs
                        .HeightOverride(130) // Adjust this based on your needs
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
                            .BorderBackgroundColor(FLinearColor::Black)
                            .Padding(FMargin(1))
                            [
                                SAssignNew(TableView, SListView<TSharedPtr<FKeywordTableRow>>)
                                .ItemHeight(24)
                                .ListItemsSource(&TableRows)
                                .OnGenerateRow(this, &SGUIWidget::GenerateTableRow)
                                .SelectionMode(ESelectionMode::Single)
                                .HeaderRow
                                (
                                    SNew(SHeaderRow)
                                    + SHeaderRow::Column("FeaturesColumn")
                                    .DefaultLabel(LOCTEXT("FeaturesColumnHeader", "Features"))
                                    .FillWidth(0.5f)

                                    + SHeaderRow::Column("InputColumn")
                                    .DefaultLabel(LOCTEXT("InputColumnHeader", "Input"))
                                    .FillWidth(0.5f)
                                )
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
    }
}

// ------------------------------
// --- BUTTON CALLBACK METHODS
// ------------------------------

FReply SGUIWidget::LoginButtonClicked()
{
    // Get the input from the UsernameField
    if (KeyField.IsValid())
    {
        FString UserKey = KeyField->GetText().ToString();

        // Make API call with Username
        FString Url = "https://opus5.p.rapidapi.com/get_model_names";
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
        HttpRequest->SetURL(Url);
        HttpRequest->SetVerb("GET");
        HttpRequest->SetHeader("Content-Type", "application/json");
        HttpRequest->SetHeader("X-RapidAPI-Key", UserKey);
        HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");

        HttpRequest->OnProcessRequestComplete().BindLambda([this, UserKey](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
            {
                if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
                {
                    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, UserKey]()
                        {
                            FPlatformProcess::Sleep(2);

                            AsyncTask(ENamedThreads::GameThread, [this, UserKey]()
                                {
                                    ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));

                                    // Save the API key to a file
                                    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
                                    FFileHelper::SaveStringToFile(UserKey, *SaveFilePath);
                                    FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

                                    bIsLoggedIn = true;
                                    this->RebuildWidget();
                                });
                        });
                }
                else
                {
                    ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                }
            });

        HttpRequest->ProcessRequest();
    }

    return FReply::Handled();
}

FReply SGUIWidget::LogoutButtonClicked()
{
    // Set logged in flag to false
    bIsLoggedIn = false;

    // Delete the APIKey.txt file
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
    if (FPaths::FileExists(SaveFilePath))
    {
        IFileManager::Get().Delete(*SaveFilePath);
    }

    // Rebuild the widget to go back to the login screen
    this->RebuildWidget();

    return FReply::Handled();
}

FReply SGUIWidget::CreateButtonClicked()
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
            //TODO: this text needs to be changed in order to be clear
            .Text(LOCTEXT("WarningText", "If you want to customize your component, It will not be possible after creating the component.\nYou can search and add features from below.\nThis warning will not be shown again.\nDo you want to proceed?"))
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
                //SUGGESTION: this button may direct the user to QUEUE screen
                SNew(SButton)
                .Text(LOCTEXT("YesButton", "Yes"))
                .OnClicked_Lambda([this, WarningWindowPtr]() {
                if (WarningWindowPtr.IsValid())
                {
                    WarningWindowPtr->RequestDestroyWindow();
                }

                SendAPIRequest_Create();  // Your existing API call logic

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

FReply SGUIWidget::ApplyFeatureButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("ApplyFeatureButtonClicked called!"));

    bool bAddNewRow = false;

    if (TagsSearchBox.IsValid())
    {
        FString TagBoxContent = TagsSearchBox->GetText().ToString();
        TArray<FString> ParsedStrings;

        bool bFound = false;

        if (TagBoxContent.ParseIntoArray(ParsedStrings, TEXT(" - "), true) == 2)
        {
            TSharedPtr<FString> MainCategory = MakeShared<FString>(ParsedStrings[0].TrimStartAndEnd());
            TSharedPtr<FString> CurrentTag = MakeShared<FString>(ParsedStrings[1].TrimStartAndEnd());

            for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
            {
                if (existingRow->Keyword->Equals(*MainCategory))
                {
                    if (IsParameter(*MainCategory))  // Assuming IsParameter checks if it's a parameter
                    {
                        existingRow->Value = CurrentTag;  // Update the value only
                        bFound = true;
                        break;
                    }
                    else // Check for non-parameter type
                    {
                        if (existingRow->Value->Equals(*CurrentTag))
                        {
                            // Skip adding if a row with same keyword and value already exists
                            bFound = true;
                            break;
                        }
                    }
                }
            }

            if (!bFound)
            {
                // Add new row for either parameter or non-parameter if not found
                TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
                newRow->Keyword = MainCategory;
                newRow->Value = CurrentTag;
                TableRows.Add(newRow);
                UE_LOG(LogTemp, Warning, TEXT("New Row Added! Main Category: %s, Tag: %s"), **newRow->Keyword, **newRow->Value);
            }

            TableView->RequestListRefresh();
            TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("The TagsSearchBox content format is invalid!"));
        }
    }

    bool bFoundParam = false;

    // Handling for ParamInputBox
    if (SelectedSuggestion.IsValid() && CurrentParameterRange != FVector2D::ZeroVector)
    {
        FString ParamInputContent = ParamInputBox->GetText().ToString();
        float InputValue;
        //TODO: this section must be revised to a proper try-catch structure
        if (FDefaultValueHelper::ParseFloat(ParamInputContent, InputValue))
        {
            if (InputValue >= CurrentParameterRange.X && InputValue <= CurrentParameterRange.Y)
            {
                for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
                {
                    if (existingRow->Keyword->Equals(*SelectedSuggestion))
                    {
                        existingRow->Value = MakeShared<FString>(ParamInputContent);
                        bFoundParam = true;
                        break;
                    }
                }

                if (!bFoundParam)
                {
                    TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
                    newRow->Keyword = SelectedSuggestion;
                    newRow->Value = MakeShared<FString>(ParamInputContent);
                    TableRows.Add(newRow);
                    UE_LOG(LogTemp, Warning, TEXT("New Parameter Row Added! Parameter: %s, Value: %s"), **newRow->Keyword, **newRow->Value);
                }
            }
            else
            {
                ShowNotificationFail(LOCTEXT("InvalidInputNotification", "The input is not in range!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("The input for the parameter is not a valid number!"));
        }
    }

    TableView->RequestListRefresh();
    TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);

    return FReply::Handled();
}

FReply SGUIWidget::ResetFeaturesButtonClicked()
{
    ResetTable();
    return FReply::Handled();
}

FReply SGUIWidget::QueueButtonClicked()
{
    bIsQueueClicked = !bIsQueueClicked;  // Toggle the state
    RebuildWidget();  // Rebuild the widget to reflect the new state
    return FReply::Handled();
}

FReply SGUIWidget::ReturnButtonClicked()
{
    bIsQueueClicked = false;  // Reset to show the main page
    RebuildWidget();  // Rebuild the widget to go back to the main page
    return FReply::Handled();
}

FReply SGUIWidget::RefreshCachingButtonClicked()
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
            .Text(LOCTEXT("WarningText", "You are now deleting the cached jobs you created!\nRefreshing now may interfere with any jobs that are currently being added.\nIf the job is not added into the UE yet, this could result in errors.\nAre you sure you want to proceed?"))
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

FReply SGUIWidget::EmptyQButtonClicked()
{
    // Construct the full path to the queue.txt file
    FString ProjectDir = FPaths::ProjectSavedDir();
    FString QueueFile = FPaths::Combine(ProjectDir, TEXT("queue.txt"));

    // Empty the queue.txt file
    FFileHelper::SaveStringToFile(TEXT(""), *QueueFile);

    return FReply::Handled();
}

// ------------------------------
// --- API REQUEST METHODS
// ------------------------------

void SGUIWidget::SendAPIRequest_Create()
{
    FString Url = "https://opus5.p.rapidapi.com/create_opus_component";

    for (const auto& structure : Structures)
    {
        if (structure->Equals(*CurrentItem))
        {
            // Change the URL for structures
            Url = "https://opus5.p.rapidapi.com/create_opus_structure";
            break;
        }
    }

    FString JsonData = ConstructJSONData();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", StoredAPIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->SetContentAsString(JsonData);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SGUIWidget::OnAPIRequestCreateCompleted);
    HttpRequest->ProcessRequest();
}

void SGUIWidget::OnAPIRequestCreateCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
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
            if (JsonObject->TryGetStringField("job_status", status))
            {
                UE_LOG(LogTemp, Warning, TEXT("Job status: %s"), *status);
                NewJobStatus = status;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to get job status"));
            }

            FString ResultID;
            if (JsonObject->TryGetStringField("result_id", ResultID))
            {
                APIResponseID = ResultID;

                UE_LOG(LogTemp, Warning, TEXT("HTTP Request successful. Response Code: %d, Response Data: %s"), ResponseCode, *ResponseStr);
                UE_LOG(LogTemp, Warning, TEXT("API Response ID: %s"), *APIResponseID);

                ShowNotificationSuccess(LOCTEXT("IDSuccess", "Job ID succesfully created!!"));

                // Write to file
                FString SaveDirectory = FPaths::ProjectSavedDir();
                FString FileName = FString(TEXT("queue.txt"));
                FString AbsolutePath = SaveDirectory + "/" + FileName;

                FString CurrentDateTime = FDateTime::Now().ToString(TEXT("%Y-%m-%d-%H-%M-%S"));
                FString TextToSave = *CurrentItem + TEXT(" ") + CurrentDateTime + TEXT(" ") + NewJobStatus + TEXT(" ") + *APIResponseID;

                // Append to file. This will create the file if it doesn't exist.
                FFileHelper::SaveStringToFile(TextToSave + LINE_TERMINATOR, *AbsolutePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("API Response does not contain \"result_id\" field."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
        }
    }
    else
    {
        ShowNotificationFail(LOCTEXT("Failed", "Failed to create ID. Please try again later."));
        UE_LOG(LogTemp, Error, TEXT("HTTP Request failed."));
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

void SGUIWidget::SendSecondAPIRequest_JobResult(FString jobID)
{
    FString EncodedJobID = FGenericPlatformHttp::UrlEncode(jobID.TrimStartAndEnd()); // URL encoding after trimming
    FString URL = "https://opus5.p.rapidapi.com/get_opus_job_result";
    FString Parameters = "?result_uid=" + EncodedJobID;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(URL + Parameters);
    HttpRequest->SetHeader("X-RapidAPI-Key", StoredAPIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindLambda([this, jobID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            OnSecondAPIRequestJobResultCompleted(Request, Response, bWasSuccessful, jobID);
        });
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Error, TEXT("Reached the end of request"));
    UE_LOG(LogTemp, Log, TEXT("Stored API Key: %s"), *StoredAPIKey);
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(URL + Parameters));
}

void SGUIWidget::OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID)
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
                        FString extensionKey = bIsFBXSelected ? "fbx" : "gltf";
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

void SGUIWidget::SendThirdAPIRequest_AttributeName()
{
    FString Url = "https://opus5.p.rapidapi.com/get_attributes_with_name";
    FString Parameters = "?name=" + *CurrentItem;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url + Parameters);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("X-RapidAPI-Key", StoredAPIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SGUIWidget::OnThirdAPIRequestAttributeNameCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url + Parameters));
}

void SGUIWidget::OnThirdAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

                                    FPair NewPair;
                                    NewPair.SubCategory = SubCategoryKey;
                                    NewPair.Tag = TagString;

                                    TagsList.Add(MakeShared<FPair>(NewPair));
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
                                    FPair NewPair;
                                    NewPair.SubCategory = SubCategoryKey;
                                    NewPair.Tag = TemplateString;  // Using Tag member variable to store the template

                                    TagsList.Add(MakeShared<FPair>(NewPair));
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
                                                ParameterRanges.Add(AssetParameterCombination, FVector2D(MinValue, MaxValue));

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
}

void SGUIWidget::SendForthAPIRequest_ModelNames()
{
    FString Url = "https://opus5.p.rapidapi.com/get_model_names";
    FString JsonData = "";

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", StoredAPIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->SetContentAsString(JsonData);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SGUIWidget::OnForthAPIRequestModelNamesCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url));
}

void SGUIWidget::OnForthAPIRequestModelNamesCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            ComboBoxOptions.Empty(); // Clear previous options
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
                        ComboBoxOptions.Add(structure);
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
                        ComboBoxOptions.Add(MakeShared<FString>(Value->AsString()));
                    }
                }
            }

            // Update the ComboBox
            if (ComboBoxWidget.IsValid())
            {
                ComboBoxWidget->RefreshOptions();
            }

            // Set the initial selected item
            if (ComboBoxOptions.Num() > 0)
            {
                CurrentItem = ComboBoxOptions[0];
                ComboBoxWidget->SetSelectedItem(CurrentItem);
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

// ------------------------------
// --- DOWNLOAD&UNZİP METHODS
// ------------------------------

void SGUIWidget::DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName)
{
    UE_LOG(LogTemp, Warning, TEXT("Initiating download from URL: %s"), *URL);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL(URL);

    //TODO: this section must be revised to a proper try-catch structure
    HttpRequest->OnProcessRequestComplete().BindLambda([this, DateTime, JobName](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response.IsValid())
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
                    if (ExtractWith7Zip(DownloadedZipFile, UnzipDirectory))
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

                        // Create a subfolder with the extracted name
                        if (!FPaths::DirectoryExists(DestinationSubFolder))
                        {
                            FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*DestinationSubFolder);
                        }

                        // Copy the unzipped files to the OPUS sub-directory
                        FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*DestinationSubFolder, *UnzipDirectory, true);

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

bool SGUIWidget::ExtractWith7Zip(const FString& ZipFile, const FString& DestinationDirectory)
{
    // Get the path to 7za.exe within the plugin's Binaries directory.
    FString PluginDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("GUI"));
    FString SevenZipExecutable = FPaths::Combine(PluginDir, TEXT("Binaries"), TEXT("7za.exe"));

    FString CommandArgs = FString::Printf(TEXT("e \"%s\" -o\"%s\" -y"), *ZipFile, *DestinationDirectory);

    FProcHandle Handle = FPlatformProcess::CreateProc(*SevenZipExecutable, *CommandArgs, true, false, false, nullptr, 0, nullptr, nullptr);
    if (Handle.IsValid())
    {
        FPlatformProcess::WaitForProc(Handle);
        return true;
    }
    return false;
}

// ------------------------------
// --- QUEUE TAB METHODS
// ------------------------------

TSharedRef<ITableRow> SGUIWidget::OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    FLinearColor TextColor;

    //TODO: this section must be revised to a switch structure
    if (InItem->Status == TEXT("IN_PROGRESS"))
    {
        TextColor = FLinearColor::Green;
    }
    else if (InItem->Status == TEXT("COMPLETED"))
    {
        TextColor = FLinearColor::Blue;
    }
    else if (InItem->Status == TEXT("FAILED"))
    {
        TextColor = FLinearColor::Red;
    }
    else if (InItem->Status == TEXT("SUSPENDED"))
    {
        TextColor = FLinearColor::Yellow;
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
                DownloadAndUnzipMethod(QueueData[CurrentIndex]->DownloadLink, QueueData[CurrentIndex]->DateTime, QueueData[CurrentIndex]->Job);
                ShowNotificationSuccess(LOCTEXT("Success", "The addition of the component is in progress. This might take some time varying the size of the job."));
            }
            else if (QueueData[CurrentIndex]->Status == TEXT("IN_PROGRESS")) {
                ShowNotificationPending(LOCTEXT("Pending", "Job is not ready yet. Please wait..."));
            }
            else if (QueueData[CurrentIndex]->Status == TEXT("FAILED")) {
                ShowNotificationFail(LOCTEXT("ErrorOccured", "Job failed. Please try again later."));
            }
            else if (QueueData[CurrentIndex]->Status == TEXT("SUSPENDED")) {
                ShowNotificationFail(LOCTEXT("OpusError", "Due to some reasons the job is suspended from the OPUS. Please try again later."));
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

void SGUIWidget::QueueLoop()
{
    if (bIsQueueClicked && QueueData.Num() > 0)
    {
        // Access the JobID of the current FQueueRow object
        FString currentJobID = QueueData[CurrentQueueIndex]->JobID;

        SendSecondAPIRequest_JobResult(currentJobID);

        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
            {
                FPlatformProcess::Sleep(2);

                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        if (!bIsQueueClicked || QueueData.Num() == 0)
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
    else if (!bIsQueueClicked)
    {
        UE_LOG(LogTemp, Display, TEXT("Successfully exited the loop!"));
        CurrentQueueIndex = 0;
    }
}

void SGUIWidget::ReadAndParseQueueFile()
{
    // Clear any existing data
    QueueData.Empty();

    FString SaveDirectory = FPaths::ProjectSavedDir();
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

void SGUIWidget::WriteQueueToFile()
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

// ------------------------------
// --- COMBOBOX METHODS
// ------------------------------

TSharedRef<SWidget> SGUIWidget::GenerateComboBoxItem(TSharedPtr<FString> Item)
{
    return SNew(STextBlock).Text(FText::FromString(*Item));
}

void SGUIWidget::ComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
    if (NewItem.IsValid())
    {
        CurrentItem = NewItem;

        // Clear the search boxes
        if (SearchBox.IsValid())
        {
            SearchBox->SetText(FText::GetEmpty());
        }

        if (TagsSearchBox.IsValid())
        {
            TagsSearchBox->SetText(FText::GetEmpty());
        }

        // Clear the filtered suggestion lists (assuming you still want this)
        FilteredSuggestions.Empty();
        TagFilteredSuggestions.Empty();

        ResetTable();

        // Make an API call with the new item
        SendThirdAPIRequest_AttributeName();

        // Refresh the list view
        SuggestionsListView->RequestListRefresh();
        SuggestionsListView->Invalidate(EInvalidateWidget::Layout);

        TagsSuggestionsListView->RequestListRefresh();
        TagsSuggestionsListView->Invalidate(EInvalidateWidget::Layout);
    }
}

FText SGUIWidget::GetCurrentItem() const
{
    if (CurrentItem.IsValid())
    {
        return FText::FromString(*CurrentItem);
    }
    return FText::GetEmpty();
}

// ------------------------------
// --- TABLE METHODS
// ------------------------------

TSharedRef<ITableRow> SGUIWidget::GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FKeywordTableRow>>, OwnerTable)
    [
        SNew(SHorizontalBox)

        // Keyword column
        + SHorizontalBox::Slot()
        .FillWidth(1.14)
        [
            SNew(STextBlock)
            .Text(FText::FromString(*(RowData->Keyword)))
        ]

        // Value column
        + SHorizontalBox::Slot()
        .FillWidth(1)
        [
            SNew(STextBlock)
            .Text(this, &SGUIWidget::GetKeywordValue, RowData)
        ]

        // Remove button
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("×")))
            .OnClicked_Lambda([this, RowData]() -> FReply {
            TableRows.Remove(RowData);
            TableView->RequestListRefresh();
            return FReply::Handled();
            })
        ]
    ];
}

FText SGUIWidget::GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const
{
    return FText::FromString(*RowData->Value);
}

void SGUIWidget::ResetTable()
{
    TableRows.Empty();
    if (TableView.IsValid())
    {
        TableView->RequestListRefresh();
    }
}

// ------------------------------
// --- SEARCHBOX METHODS
// ------------------------------

void SGUIWidget::OnTagsSearchTextChanged(const FText& NewText)
{
    // Clear previous suggestions
    TagFilteredSuggestions.Empty();

    FString CurrentInput = NewText.ToString();

    if (SuggestionsListView->GetVisibility() == EVisibility::Visible)
    {
        SuggestionsListView->SetVisibility(EVisibility::Collapsed);
    }

    for (const TSharedPtr<FPair>& CurrentPair : TagsList)
    {
        if (CurrentPair->Tag.Contains(CurrentInput))
        {
            TagFilteredSuggestions.Add(MakeShared<FString>(CurrentPair->SubCategory + " - " + CurrentPair->Tag));
        }
    }

    // Refresh the tags suggestion list view
    TagsSuggestionsListView->RequestListRefresh();

    if (TagFilteredSuggestions.Num() > 0)
    {
        TagsSuggestionsListView->SetVisibility(EVisibility::Visible);
    }
    else
    {
        TagsSuggestionsListView->SetVisibility(EVisibility::Collapsed);
    }
}

TSharedRef<ITableRow> SGUIWidget::GenerateTagSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
    [
        SNew(SButton)
        .Text(FText::FromString(*Suggestion))
        .OnClicked(this, &SGUIWidget::OnTagSuggestionRowClicked, Suggestion)
    ];
}

FReply SGUIWidget::OnTagSuggestionRowClicked(TSharedPtr<FString> Suggestion)
{
    if (TagsSearchBox.IsValid())
    {
        // Set the full suggestion (i.e., "SubCategory - Tag") in the search box
        TagsSearchBox->SetText(FText::FromString(*Suggestion));
    }

    // Set the selected tag suggestion
    SelectedTagSuggestion = Suggestion;

    // Clear the filtered tag suggestions
    TagFilteredSuggestions.Empty();

    // Refresh the tag suggestions list view
    TagsSuggestionsListView->RequestListRefresh();
    TagsSuggestionsListView->Invalidate(EInvalidateWidget::Layout);
    TagsSuggestionsListView->SetVisibility(EVisibility::Collapsed); // This will hide the tag suggestions view.

    return FReply::Handled();
}

void SGUIWidget::OnParamSearchTextChanged(const FText& NewText)
{
    FilteredSuggestions.Empty();

    FString CurrentInput = NewText.ToString();

    if (TagsSuggestionsListView->GetVisibility() == EVisibility::Visible)
    {
        TagsSuggestionsListView->SetVisibility(EVisibility::Collapsed);
    }

    if (ParameterRanges.Contains(CurrentInput))
    {
        SelectedSuggestion = MakeShared<FString>(CurrentInput);
    }
    else
    {
        SelectedSuggestion.Reset();
    }

    ParamInputBox->SetText(FText::GetEmpty());

    if (!CurrentInput.IsEmpty())
    {
        for (const TSharedPtr<FString>& Suggestion : ParameterSuggestions)
        {
            if (Suggestion->Contains(CurrentInput))
            {
                FilteredSuggestions.Add(Suggestion);
            }
        }
    }

    SuggestionsListView->SetVisibility(FilteredSuggestions.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed);

    // Refresh the list view
    SuggestionsListView->RequestListRefresh();
    SuggestionsListView->Invalidate(EInvalidateWidget::Layout);
    ParamInputBox->Invalidate(EInvalidateWidget::Visibility);
}

TSharedRef<ITableRow> SGUIWidget::GenerateParamSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
    [
        SNew(SButton)
        .Text(FText::FromString(*Suggestion))
        .OnClicked(this, &SGUIWidget::OnParamSuggestionRowClicked, Suggestion)
    ];
}

FReply SGUIWidget::OnParamSuggestionRowClicked(TSharedPtr<FString> Suggestion)
{
    if (SearchBox.IsValid())
    {
        SearchBox->SetText(FText::FromString(*Suggestion));
    }

    // Set the selected suggestion
    SelectedSuggestion = Suggestion;

    ParamInputBox->SetText(FText::GetEmpty());

    // Clear the filtered suggestions
    FilteredSuggestions.Empty();

    // Fetch the parameter range for the selected suggestion
    if (ParameterRanges.Contains(*Suggestion))
    {
        CurrentParameterRange = ParameterRanges[*Suggestion];
    }
    else
    {
        CurrentParameterRange = FVector2D::ZeroVector; // default value if not found
    }

    // Refresh the list view
    SuggestionsListView->RequestListRefresh();
    SuggestionsListView->Invalidate(EInvalidateWidget::Layout);
    SuggestionsListView->SetVisibility(EVisibility::Collapsed); // This will hide the suggestions view.
    ParamInputBox->Invalidate(EInvalidateWidget::Visibility);

    return FReply::Handled();
}

// ------------------------------
// --- HELPER METHODS
// ------------------------------

FString SGUIWidget::ConstructJSONData()
{
    TMap<FString, TArray<FString>> KeywordMap;

    // Populate the KeywordMap
    for (TSharedPtr<FKeywordTableRow>& row : TableRows)
    {
        if (!IsParameter(*row->Keyword))  // Check if it is not a parameter
        {
            KeywordMap.FindOrAdd(*row->Keyword).Add(*row->Value);
        }
    }

    // Start constructing the JSON data
    FString JsonData = "{\r\n";
    JsonData += "    \"name\": \"" + *CurrentItem + "\",\r\n";
    JsonData += "    \"texture_resolution\": \"1024\",\r\n";
    JsonData += FString::Printf(TEXT("    \"extensions\": [\r\n        \"%s\"\r\n    ],\r\n"), bIsFBXSelected ? TEXT("fbx") : TEXT("gltf"));

    // Constructing parameters (No changes here)
    JsonData += "    \"parameters\": {\r\n";
    bool isFirstParameter = true;
    for (TSharedPtr<FKeywordTableRow>& row : TableRows)
    {
        if (IsParameter(*row->Keyword))  // Check if it is a parameter
        {
            if (!isFirstParameter)
            {
                JsonData += ",\r\n";
            }
            isFirstParameter = false;
            JsonData += "        \"" + *row->Keyword + "\": " + *row->Value;
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

FText SGUIWidget::GetParamHintText() const
{
    if (CurrentParameterRange != FVector2D::ZeroVector)
    {
        return FText::Format(LOCTEXT("ParamRangeHint", "Range: {0} - {1}"), FText::AsNumber(CurrentParameterRange.X), FText::AsNumber(CurrentParameterRange.Y));
    }
    else
    {
        return LOCTEXT("NoRangeHint", "No range provided");
    }
}

EVisibility SGUIWidget::GetParamInputBoxVisibility() const
{
    // If SelectedSuggestion is valid, then it's considered valid parameter and input box should be visible
    return SelectedSuggestion.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SGUIWidget::IsParameter(const FString& Keyword)
{
    return Keyword.Contains("/");  // Adjust this if the criteria change
}

void SGUIWidget::RebuildWidget()
{
    Construct(FArguments());
}

void SGUIWidget::OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        LoginButtonClicked(); // You can call your login function directly here
    }
}

// ------------------------------
// --- NOTIFICATION METHODS
// ------------------------------

void SGUIWidget::ShowNotificationFail(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;        // Makes the notification automatically disappear after a delay.
    Info.FadeOutDuration = 3.0f;       // Time for the fade out animation.
    Info.ExpireDuration = 5.0f;        // Time until the notification disappears.
    Info.bUseLargeFont = false;        // Adjust font.

    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Fail);
        NotificationItem->ExpireAndFadeout();
    }
}

void SGUIWidget::ShowNotificationSuccess(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 3.0f;
    Info.ExpireDuration = 5.0f;
    Info.bUseLargeFont = false;

    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Success);
        NotificationItem->ExpireAndFadeout();
    }
}

void SGUIWidget::ShowNotificationPending(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 3.0f;
    Info.ExpireDuration = 5.0f;
    Info.bUseLargeFont = false;

    PendingNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (PendingNotificationItem.IsValid())
    {
        PendingNotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Pending);
        PendingNotificationItem->ExpireAndFadeout();
    }
}

// ------------------------------
// --- MODULE LIFECYCLE METHODS
// ------------------------------

void FGUIModule::StartupModule()
{
    FGUIStyle::Initialize();
    FGUIStyle::ReloadTextures();

    FGUICommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FGUICommands::Get().PluginAction,
        FExecuteAction::CreateRaw(this, &FGUIModule::PluginButtonClicked),
        FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGUIModule::RegisterMenus));
}

void FGUIModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);

    UToolMenus::UnregisterOwner(this);

    FGUIStyle::Shutdown();

    FGUICommands::Unregister();
}

// ------------------------------
// --- USER INTERFACE METHODS
// ------------------------------

void FGUIModule::PluginButtonClicked()
{
    if (GUIWidget.IsValid()) {
        GUIWidget->bIsQueueClicked = false;
    }

    ShowWidget();
}

void FGUIModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
            FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
            Section.AddMenuEntryWithCommandList(FGUICommands::Get().PluginAction, PluginCommands);
        }
    }

    {
        UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
        {
            FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
            {
                FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGUICommands::Get().PluginAction));
                Entry.SetCommandList(PluginCommands);
            }
        }
    }
}

void FGUIModule::ShowWidget()
{
    SAssignNew(GUIWidget, SGUIWidget);

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("WindowTitle", "OPUS API"))
        .ClientSize(FVector2D(800, 650))
        .IsInitiallyMaximized(false);

    Window->SetContent(GUIWidget.ToSharedRef());
    FSlateApplication::Get().AddWindow(Window);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGUIModule, GUI)
