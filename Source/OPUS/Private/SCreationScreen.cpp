// Fill out your copyright notice in the Description page of Project Settings.


#include "SCreationScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"
#include "SFilteredSelectionTextBox.h"

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

    ChildSlot
        [
            SNew(SOverlay)

                // Logout button
                + SOverlay::Slot()
                .VAlign(VAlign_Top)
                .HAlign(HAlign_Right)
                .Padding(0, 0, 30, 0)
                [
                    SNew(SButton)
                        .Text(LOCTEXT("LogoutButton", "Logout 🚪"))
                        .OnClicked(this, &SCreationScreen::LogoutButtonClicked)
                ]

                // Queue Screen button
                + SOverlay::Slot()
                .VAlign(VAlign_Top)
                .HAlign(HAlign_Right)
                .Padding(0, 0, 140, 0)
                [
                    SNew(SButton)
                        .Text(LOCTEXT("JobQueueButton", "Job Queue ↡"))
                        .OnClicked(this, &SCreationScreen::QueueButtonClicked)
                ]

                + SOverlay::Slot()
                .VAlign(VAlign_Fill)
                .HAlign(HAlign_Fill)
                [

                    SNew(SHorizontalBox)


                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SVerticalBox)

                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(30, 40, 200, 0)
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
                                                        .OnSelectionChanged(this, &SCreationScreen::ComboBoxSelectionChanged)
                                                        .InitiallySelectedItem(CurrentModel)
                                                        .ContentPadding(FMargin(2.0f))
                                                        [
                                                            SNew(STextBlock)
                                                                .Text(this, &SCreationScreen::GetCurrentItem)
                                                        ]
                                                ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(30, 0, 0, 0)
                                        [
                                            SNew(SCheckBox)
                                                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                                                if (NewState == ECheckBoxState::Checked) {
                                                    IsFBXSelected = true;
                                                    IsGLTFSelected = false;
                                                }
                                                    })
                                                .IsChecked_Lambda([this]() -> ECheckBoxState {
                                                        return IsFBXSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                                                    })
                                                        [
                                                            SNew(STextBlock).Text(LOCTEXT("FBX", "FBX"))
                                                        ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(30, 0, 0, 0)
                                        [
                                            SNew(SCheckBox)
                                                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                                                if (NewState == ECheckBoxState::Checked) {
                                                    IsGLTFSelected = true;
                                                    IsFBXSelected = false;
                                                }
                                                    })
                                                .IsChecked_Lambda([this]() -> ECheckBoxState {
                                                        return IsGLTFSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                                                    })
                                                        [
                                                            SNew(STextBlock).Text(LOCTEXT("GLTF", "GLTF"))
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
                                    SNew(SHorizontalBox)

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        [
                                            SNew(SBox)
                                                .WidthOverride(130)  // Adjusted width
                                                .HeightOverride(50)  // Adjusted height
                                                [
                                                    SAssignNew(ParamInputBox, SEditableTextBox)
                                                        .HintText(this, &SCreationScreen::GetParamHintText)
                                                        //.Visibility(this, &SCreationScreen::GetParamInputBoxVisibility)
                                                ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .HAlign(HAlign_Right)
                                        .AutoWidth()
                                        .Padding(20, 0, 0, 0)
                                        [
                                            SNew(SBox)
                                                .WidthOverride(115)  // Adjusted button width
                                                .HeightOverride(50)  // Adjusted button height
                                                [
                                                    SNew(SButton)
                                                        .VAlign(VAlign_Center)
                                                        .HAlign(HAlign_Center)
                                                        .Text(LOCTEXT("ApplyFeatureButton", "Apply 🛠️"))
                                                        .OnClicked(this, &SCreationScreen::ApplyFeatureButtonClicked)
                                                        .Visibility(EVisibility::Visible) // Always visible
                                                ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .HAlign(HAlign_Right)
                                        .AutoWidth()
                                        .Padding(20, 0, 0, 0)
                                        [
                                            SNew(SBox)
                                                .WidthOverride(115)  // Adjusted button width
                                                .HeightOverride(50)  // Adjusted button height
                                                [
                                                    SNew(SButton)
                                                        .VAlign(VAlign_Center)
                                                        .HAlign(HAlign_Center)
                                                        .Text(LOCTEXT("ResetFeaturesButton", "Reset 🧹"))
                                                        .OnClicked(this, &SCreationScreen::ResetFeaturesButtonClicked)
                                                        .Visibility(EVisibility::Visible) // Always visible
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
                                                .WidthOverride(300) // Adjust this based on your needs
                                                .HeightOverride(200) // Adjust this based on your needs
                                                [
                                                    SNew(SBorder)
                                                        .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
                                                        .BorderBackgroundColor(FLinearColor::Black)
                                                        .Padding(FMargin(1))
                                                        [
                                                            SAssignNew(TableView, SListView<TSharedPtr<FKeywordTableRow>>)
                                                                .ItemHeight(24)
                                                                .ListItemsSource(&TableRows)
                                                                .OnGenerateRow(this, &SCreationScreen::GenerateTableRow)
                                                                .SelectionMode(ESelectionMode::Single)
                                                                .HeaderRow
                                                                (
                                                                    SNew(SHeaderRow)
                                                                    + SHeaderRow::Column("FeaturesColumn")
                                                                    .DefaultLabel(LOCTEXT("FeaturesColumnHeader", "Features"))
                                                                    .FillWidth(0.7f)

                                                                    + SHeaderRow::Column("InputColumn")
                                                                    .DefaultLabel(LOCTEXT("InputColumnHeader", "Input"))
                                                                    .FillWidth(0.3f)
                                                                )
                                                        ]
                                                ]
                                        ]
                                ]

                                + SVerticalBox::Slot()
                                .VAlign(VAlign_Bottom)
                                .HAlign(HAlign_Center)
                                .AutoHeight()
                                .Padding(0, 10, 0, 0)
                                [
                                    SNew(SBox)
                                        .WidthOverride(130)
                                        .HeightOverride(50)
                                        [
                                            SNew(SButton)
                                                .VAlign(VAlign_Center)
                                                .HAlign(HAlign_Center)
                                                .Text(LOCTEXT("Generate Model", "Generate Model"))
                                                .OnClicked(this, &SCreationScreen::CreateButtonClicked)
                                                .ButtonColorAndOpacity(FLinearColor(0.1,1,0.1,1))
                                        ]
                                ]
                        ]

                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SVerticalBox)

                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 70, 30, 0)
                                [

                                    SNew(SBorder)
                                        .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                                        .BorderBackgroundColor(FLinearColor::White)
                                        .Padding(FMargin(5.0f))
                                        [
                                            SNew(SBox)
                                            .WidthOverride(250)
                                            .HeightOverride(250)
                                                [
                                                    SNew(SImage)
                                                        .Image(FOPUSStyle::Get().GetBrush("OPUS.SmallLogo"))
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

// TODO Make create button send request and give positive feedback.
FReply SCreationScreen::CreateButtonClicked()
{
    SendAPIRequest_Create();  
    OnQueueScreenEnabledDelegate.Broadcast();
    return FReply::Handled();
}

void SCreationScreen::ParseTagInput(const FText& TagInputText)
{
    FString TagBoxContent = TagInputText.ToString();
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
            // Add new row if tag doesnt exist in table
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

void SCreationScreen::ParseParameterInput(const FText& ParameterInputText)
{
    bool bFoundParam = false;
    FString ParamInputContent = ParameterInputText.ToString();
    float InputValue;
    //TODO: this section must be revised to a proper try-catch structure
    if (FDefaultValueHelper::ParseFloat(ParamInputContent, InputValue))
    {
        if (InputValue >= CurrentParameterRange.X && InputValue <= CurrentParameterRange.Y)
        {
            for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
            {
                if (existingRow->Keyword->Equals(*SelectedParameterSuggestion))
                {
                    existingRow->Value = MakeShared<FString>(ParamInputContent);
                    bFoundParam = true;
                    break;
                }
            }

            if (!bFoundParam)
            {
                TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
                newRow->Keyword = SelectedParameterSuggestion;
                newRow->Value = MakeShared<FString>(ParamInputContent);
                TableRows.Add(newRow);
                UE_LOG(LogTemp, Warning, TEXT("New Parameter Row Added! Parameter: %s, Value: %s"), **newRow->Keyword, **newRow->Value);
            }
        }
        else
        {
            //NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidInputNotification", "The input is not in range!"));
            ShowWarningWindow("Parameter input is out of given range");
        }
    }
    else
    {
        ShowWarningWindow("Parameter input is not a number");
        UE_LOG(LogTemp, Warning, TEXT("The input for the parameter is not a valid number!"));
    }
}

FReply SCreationScreen::ApplyFeatureButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("ApplyFeatureButtonClicked called!"));

    if (TagSearchBox.IsValid())
    {
        ParseTagInput(TagSearchBox->GetText());
    }

    // Handling for ParamInputBox
    if (SelectedParameterSuggestion.IsValid() && CurrentParameterRange != FVector2D::ZeroVector)
    {
        ParseParameterInput(ParamInputBox->GetText());
    }

    // Clear selection
    SelectedTagSuggestion = nullptr;
    SelectedParameterSuggestion = nullptr;
    CurrentParameterRange = FVector2D::Zero();
    TagSearchBox->SetText(FText::GetEmpty());
    ParameterSearchBox->SetText(FText::GetEmpty());
    GetParamHintText();

    // Clear table
    TableView->RequestListRefresh();
    TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);

    return FReply::Handled();
}

FReply SCreationScreen::ResetFeaturesButtonClicked()
{
    ResetTable();
    return FReply::Handled();
}

FReply SCreationScreen::QueueButtonClicked()
{
    // Enable screen by broadcasting delegate
    OnQueueScreenEnabledDelegate.Broadcast();

    return FReply::Handled();
}

// ------------------------------
// --- SEARCHBOX METHODS
// ------------------------------

void SCreationScreen::OnTagsSearchTextChanged(const FText& NewText)
{
    // Clear previous suggestions
    TagFilteredSuggestions.Empty();

    FString CurrentInput = NewText.ToString();

    for (const TSharedPtr<FPair>& CurrentPair : TagsList)
    {
        if (CurrentPair->Tag.Contains(CurrentInput))
        {
            TagFilteredSuggestions.Add(MakeShared<FString>(CurrentPair->SubCategory + " - " + CurrentPair->Tag));
        }
    }

    // Refresh the tags suggestion list view
    TagSearchBox->RequestListRefresh();
}

void SCreationScreen::OnParamSearchTextChanged(const FText& NewText)
{
    ParameterFilteredSuggestions.Empty();

    FString CurrentInput = NewText.ToString();

    if (ParameterRanges.Contains(CurrentInput))
    {
        SelectedParameterSuggestion = MakeShared<FString>(CurrentInput);
    }
    else
    {
        SelectedParameterSuggestion.Reset();
    }

    ParamInputBox->SetText(FText::GetEmpty());
    
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
}

void SCreationScreen::OnParameterSelected(TSharedPtr<FString> ParameterSelection)
{
    SelectedParameterSuggestion = ParameterSelection;

    // Fetch the parameter range for the selected suggestion
    if (ParameterRanges.Contains(*ParameterSelection))
    {
        CurrentParameterRange = ParameterRanges[*ParameterSelection];
    }
    else
    {
        CurrentParameterRange = FVector2D::ZeroVector; // default value if not found
    }
}


// ------------------------------
// --- TABLE METHODS
// ------------------------------

TSharedRef<ITableRow> SCreationScreen::GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FKeywordTableRow>>, OwnerTable)
        [
            SNew(SHorizontalBox)

                // Keyword column
                + SHorizontalBox::Slot()
                .FillWidth(1.5)
                .Padding(5, 0, 0, 0)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString(*(RowData->Keyword)))
                ]

                // Value column
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 5, 0)
                [
                    SNew(STextBlock)
                        .Text(this, &SCreationScreen::GetKeywordValue, RowData)
                ]

                // Remove button
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("×")))
                        .OnClicked_Lambda([this, RowData]() -> FReply 
                            {
                                TableRows.Remove(RowData);
                                TableView->RequestListRefresh();
                                return FReply::Handled();
                            })
                ]
        ];
}

FText SCreationScreen::GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const
{
    return FText::FromString(*RowData->Value);
}

void SCreationScreen::ResetTable()
{
    TableRows.Empty();
    if (TableView.IsValid())
    {
        TableView->RequestListRefresh();
    }
}

// ------------------------------
// --- HELPER METHODS
// ------------------------------

void SCreationScreen::SetAPIKey(FString apiKey) { APIKey = apiKey; }

FText SCreationScreen::GetCurrentItem() const
{
    if (CurrentModel.IsValid())
    {
        return FText::FromString(*CurrentModel);
    }
    return FText::GetEmpty();
}

FText SCreationScreen::GetParamHintText() const
{
    if (CurrentParameterRange != FVector2D::ZeroVector)
    {
        FString RangeLow = FString::SanitizeFloat(CurrentParameterRange.X);
        FString RangeHigh = FString::SanitizeFloat(CurrentParameterRange.Y);
        return FText::Format(LOCTEXT("ParamRangeHint", "Range: {0} - {1}"), FText::FromString(RangeLow), FText::FromString(RangeHigh));
    }
    else
    {
        return LOCTEXT("Param Value", "Parameter value");
    }
}

void SCreationScreen::ComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
    if (NewItem.IsValid())
    {
        CurrentModel = NewItem;

        // Clear the search boxes
        if (ParameterSearchBox.IsValid())
        {
            //ParameterSearchBox->SetText(FText::GetEmpty());
        }

        if (TagSearchBox.IsValid())
        {
            //TagSearchBox->SetText(FText::GetEmpty());
        }

        // Clear the filtered suggestion lists (assuming you still want this)
        ParameterFilteredSuggestions.Empty();
        TagFilteredSuggestions.Empty();

        ResetTable();

        // Make an API call with the new item
        SendThirdAPIRequest_AttributeName();

        // Refresh the list view
        ParameterSearchBox->RequestListRefresh();
        ParameterSearchBox->Invalidate(EInvalidateWidget::Layout);

        TagSearchBox->RequestListRefresh();
        TagSearchBox->Invalidate(EInvalidateWidget::Layout);
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

    // dont make this come up every time
    FSlateApplication::Get().AddWindowAsNativeChild(WarningWindow, FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());

    WarningWindow->SetContent(
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(15)
        [
            SNew(STextBlock)
                //TODO: this text needs to be changed in order to be clear
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
                    //SUGGESTION: this button may direct the user to QUEUE screen
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

// Deprecated
EVisibility SCreationScreen::GetParamInputBoxVisibility() const
{
    // If SelectedSuggestion is valid, then it's considered valid parameter and input box should be visible
    return SelectedParameterSuggestion.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

TSharedRef<SWidget> SCreationScreen::GenerateComboBoxItem(TSharedPtr<FString> Item)
{
    return SNew(STextBlock).Text(FText::FromString(*Item));
}

bool SCreationScreen::IsParameter(const FString& Keyword)
{
    return Keyword.Contains("/");  // Adjust this if the criteria change
}

// ------------------------------
// --- API REQUEST METHODS
// ------------------------------

void SCreationScreen::SendAPIRequest_Create()
{
    FString Url = "https://opus5.p.rapidapi.com/create_opus_component";

    for (const auto& structure : Structures)
    {
        if (structure->Equals(*CurrentModel))
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
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->SetContentAsString(JsonData);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnAPIRequestCreateCompleted);
    HttpRequest->ProcessRequest();
}

FString SCreationScreen::ConstructJSONData()
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
    JsonData += "    \"name\": \"" + *CurrentModel + "\",\r\n";
    JsonData += "    \"texture_resolution\": \"1024\",\r\n";
    JsonData += FString::Printf(TEXT("    \"extensions\": [\r\n        \"%s\"\r\n    ],\r\n"), IsFBXSelected ? TEXT("fbx") : TEXT("gltf"));

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

void SCreationScreen::OnAPIRequestCreateCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

                NotificationHelper.ShowNotificationSuccess(LOCTEXT("IDSuccess", "Job ID succesfully created!!"));

                // Write to file
                FString SaveDirectory = FPaths::ProjectSavedDir();
                FString FileName = FString(TEXT("queue.txt"));
                FString AbsolutePath = SaveDirectory + "/" + FileName;

                FString CurrentDateTime = FDateTime::Now().ToString(TEXT("%Y-%m-%d-%H-%M-%S"));
                FString TextToSave = *CurrentModel + TEXT(" ") + CurrentDateTime + TEXT(" ") + NewJobStatus + TEXT(" ") + *APIResponseID;

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
        NotificationHelper.ShowNotificationFail(LOCTEXT("Failed", "Failed to create ID. Please try again later."));
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



void SCreationScreen::SendThirdAPIRequest_AttributeName()
{
    FString Url = "https://opus5.p.rapidapi.com/get_attributes_with_name";
    FString Parameters = "?name=" + *CurrentModel;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url + Parameters);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnThirdAPIRequestAttributeNameCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url + Parameters));
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

                                    FPair NewPair;
                                    NewPair.SubCategory = SubCategoryKey;
                                    NewPair.Tag = TagString;

                                    TagsList.Add(MakeShared<FPair>(NewPair));

                                    TagFilteredSuggestions.Add(MakeShared<FString>(NewPair.SubCategory + " - " + NewPair.Tag));
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
                                    TagFilteredSuggestions.Add(MakeShared<FString>(NewPair.SubCategory + " - " + NewPair.Tag));
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

void SCreationScreen::SendForthAPIRequest_ModelNames()
{
    FString Url = "https://opus5.p.rapidapi.com/get_model_names";
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
