﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "SCreationScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"

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
                        .OnClicked(this, &SCreationScreen::LogoutButtonClicked)
                ]

                + SOverlay::Slot()
                .VAlign(VAlign_Top)
                .HAlign(HAlign_Right)
                .Padding(0, 0, 95, 0)
                [
                    SNew(SButton)
                        .Text(LOCTEXT("QueueButton", "Queue ↡"))
                        .OnClicked(this, &SCreationScreen::QueueButtonClicked)
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
                                .Text(LOCTEXT("Select Component", "Please select a component from the menu."))
                        ]

                        // Component selection combo box
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
                                                .OnGenerateWidget(this, &SCreationScreen::GenerateComboBoxItem)
                                                .OnSelectionChanged(this, &SCreationScreen::ComboBoxSelectionChanged)
                                                .InitiallySelectedItem(CurrentItem)
                                                .ContentPadding(FMargin(2.0f))
                                                [
                                                    SNew(STextBlock)
                                                        .Text(this, &SCreationScreen::GetCurrentItem)
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
                                                .OnClicked(this, &SCreationScreen::CreateButtonClicked)
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
                                                .Image(FOPUSStyle::Get().GetBrush("OPUS.SmallLogo"))
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
                                                        .OnTextChanged(this, &SCreationScreen::OnTagsSearchTextChanged)
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
                                                        .OnGenerateRow(this, &SCreationScreen::GenerateTagSuggestionRow)
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
                                        .OnTextChanged(this, &SCreationScreen::OnParamSearchTextChanged)
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
                                                .OnGenerateRow(this, &SCreationScreen::GenerateParamSuggestionRow)
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
                                                .HintText(this, &SCreationScreen::GetParamHintText)
                                                .Visibility(this, &SCreationScreen::GetParamInputBoxVisibility)
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
                                                .OnClicked(this, &SCreationScreen::ApplyFeatureButtonClicked)
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
                                                .OnClicked(this, &SCreationScreen::ResetFeaturesButtonClicked)
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
                                .Padding(15, 0, 10, 0)
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
                                                        .OnGenerateRow(this, &SCreationScreen::GenerateTableRow)
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

// ------------------------------
// --- BUTTON CALLBACK METHODS
// ------------------------------

FReply SCreationScreen::LogoutButtonClicked()
{
    // TODO send logout event
    // Set logged in flag to false
    //bIsLoggedIn = false;


    // Delete the APIKey.txt file
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
    if (FPaths::FileExists(SaveFilePath))
    {
        IFileManager::Get().Delete(*SaveFilePath);
    }

    // Rebuild the widget to go back to the login screen
    //this->RebuildWidget();

    return FReply::Handled();
}

// TODO Make create button send request and give positive feedback.
FReply SCreationScreen::CreateButtonClicked()
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

FReply SCreationScreen::ApplyFeatureButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("ApplyFeatureButtonClicked called!"));

    bool bAddNewRow = false;

    // TODO Too many nested ifs, make it leaner

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
                NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidInputNotification", "The input is not in range!"));
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

FReply SCreationScreen::ResetFeaturesButtonClicked()
{
    ResetTable();
    return FReply::Handled();
}

FReply SCreationScreen::QueueButtonClicked()
{
    //bIsQueueClicked = !bIsQueueClicked;  // Toggle the state

    // TODO send to queue screen
    //RebuildWidget();  // Rebuild the widget to reflect the new state
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

TSharedRef<ITableRow> SCreationScreen::GenerateTagSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(SButton)
                .Text(FText::FromString(*Suggestion))
                .OnClicked(this, &SCreationScreen::OnTagSuggestionRowClicked, Suggestion)
        ];
}

FReply SCreationScreen::OnTagSuggestionRowClicked(TSharedPtr<FString> Suggestion)
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

void SCreationScreen::OnParamSearchTextChanged(const FText& NewText)
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

TSharedRef<ITableRow> SCreationScreen::GenerateParamSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(SButton)
                .Text(FText::FromString(*Suggestion))
                .OnClicked(this, &SCreationScreen::OnParamSuggestionRowClicked, Suggestion)
        ];
}

FReply SCreationScreen::OnParamSuggestionRowClicked(TSharedPtr<FString> Suggestion)
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
// --- TABLE METHODS
// ------------------------------

TSharedRef<ITableRow> SCreationScreen::GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable)
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
                        .Text(this, &SCreationScreen::GetKeywordValue, RowData)
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

FText SCreationScreen::GetCurrentItem() const
{
    if (CurrentItem.IsValid())
    {
        return FText::FromString(*CurrentItem);
    }
    return FText::GetEmpty();
}

FText SCreationScreen::GetParamHintText() const
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

void SCreationScreen::ComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
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

EVisibility SCreationScreen::GetParamInputBoxVisibility() const
{
    // If SelectedSuggestion is valid, then it's considered valid parameter and input box should be visible
    return SelectedSuggestion.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
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
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey.ToString());
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
    JsonData += "    \"name\": \"" + *CurrentItem + "\",\r\n";
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
    FString Parameters = "?name=" + *CurrentItem;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url + Parameters);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey.ToString());
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

void SCreationScreen::SendForthAPIRequest_ModelNames()
{
    FString Url = "https://opus5.p.rapidapi.com/get_model_names";
    FString JsonData = "";

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey.ToString());
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE
