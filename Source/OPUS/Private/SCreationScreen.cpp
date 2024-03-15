// Copyright Capoom Inc. All Rights Reserved.


#include "SCreationScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"
#include "SFilteredSelectionTextBox.h"
#include "URLHelper.h"
#include "SCustomizationTable.h"

// Libraries
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
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
    TagFilteredSuggestions.Empty();
    TemplateFilteredSuggestions.Empty();
    ParameterFilteredSuggestions.Empty();
    TagList.Empty();
    ParameterList.Empty();
    TemplateList.Empty();
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
                    .Padding(30, 0, 0, 0)
                    .MaxWidth(95)
                    [

                        SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("File Type", "File Type"))
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
                        .Padding(0, 5, 0, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("Texture Size", "Texture Size"))
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

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 5, 0, 0)
                        [
                            SNew(STextBlock)
                                .Text(LOCTEXT("Random seed", "Random Seed"))
                        ]

                        + SVerticalBox::Slot()
                        
                        [
                            /*
                            SNew(SNumericEntryBox<int32>)
                            .Value(this, &SCreationScreen::GetBatchCount)
                            .OnValueChanged(this, &SCreationScreen::OnBatchCountChanged)
                            .Delta(1)
                            */
                            
                            SNew(SNumericEntryBox<int32>)
                            .Value(this, &SCreationScreen::GetSeed)
                            .OnValueChanged(this, &SCreationScreen::OnSeedChanged)
                            .Delta(1)
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 5, 0, 0)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("Randomize", "Randomize"))
                            
                            .OnClicked_Lambda([this]()
                                {
                                    Seed = FMath::RandRange(0, 999999999);
                                    return FReply::Handled();
                                })
                        ]                       
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(260, 0, 0, 0)
                    [
                        SNew(SBox)
                        .WidthOverride(190)
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
                    .Text(LOCTEXT("Select tags", "Select a model tag"))
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
                        .Text(LOCTEXT("Select templates", "Select a template"))
                ]

                // Tag Search Box
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 10, 30, 0)
                [
                    SAssignNew(TemplateSearchBox, SFilteredSelectionTextBox)
                        .ListItemsSource(&TemplateFilteredSuggestions)
                        .OnTextChanged(this, &SCreationScreen::OnTemplateSearchTextChanged)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(30, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Parameter Customization", "Select a parameter to edit"))
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
                        .WidthOverride(160)  // Adjusted button width
                        .HeightOverride(50)  // Adjusted button height
                        [
                            SNew(SButton)
                            .VAlign(VAlign_Center)
                            .HAlign(HAlign_Center)
                            .Text(LOCTEXT("ResetFeaturesButton", "Reset Customization"))
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
                        .WidthOverride(160)
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
	
    TagSearchBox->OnSuggestionSelected.AddRaw(this, &SCreationScreen::OnTagSelected);
    TemplateSearchBox->OnSuggestionSelected.AddRaw(this, &SCreationScreen::OnTemplateSelected);
    ParameterSearchBox->OnSuggestionSelected.AddRaw(this, &SCreationScreen::OnParameterSelected);
    TagSearchBox->SelectionsLoadingStarted();
    TemplateSearchBox->SelectionsLoadingStarted();
    ParameterSearchBox->SelectionsLoadingStarted();

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

TOptional<int32> SCreationScreen::GetBatchCount() const
{
    return BatchCount;
}

TOptional<int32> SCreationScreen::GetSeed() const
{
    return Seed;
}

void SCreationScreen::OnBatchCountChanged(int32 NewValue)
{
    if (NewValue > 0 && NewValue <= MaxBatchCount) BatchCount = NewValue;
    else if (NewValue <= 0) BatchCount = 1;
    else if (NewValue > MaxBatchCount) BatchCount = MaxBatchCount;
}  

void SCreationScreen::OnSeedChanged(int32 NewValue)
{
    int32 maxSeed = 999999999;
    int32 minSeed = -1;

    if (NewValue >= minSeed && NewValue <= maxSeed) Seed = NewValue;
    else if (NewValue < minSeed) Seed = minSeed;
    else if (NewValue > maxSeed) Seed = maxSeed;
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

        // Give feedback
        TagSearchBox->SelectionsLoadingStarted();
        TemplateSearchBox->SelectionsLoadingStarted();
        ParameterSearchBox->SelectionsLoadingStarted();

        // Make an API call with the new item
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
            {
                FPlatformProcess::Sleep(1.5);

                AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        SendAPIRequest_AttributeName();
                    });
            });
        

        // Refresh the list view
        TagSearchBox->RequestListRefresh();
        TagSearchBox->Invalidate(EInvalidateWidget::Layout);

        TemplateSearchBox->RequestListRefresh();
        TemplateSearchBox->Invalidate(EInvalidateWidget::Layout);

        ParameterSearchBox->RequestListRefresh();
        ParameterSearchBox->Invalidate(EInvalidateWidget::Layout);
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
    for (const TSharedPtr<FAssetTag>& CurrentTag : TagList)
    {
        bool TagCategoryAlreadyAdded = false;
        if (CurrentTag->Tag.Contains(CurrentInput) || CurrentTag->ComponentName.Contains(CurrentInput) || CurrentInput.IsEmpty())
        {
            // Loop through rows in the customizations table
            for (const TSharedPtr<FKeywordTableRow> Row : CustomizationTable->TableRows)
            {
                // Check if subcategory tag already assigned
                if (Row->Customization->Equals(CurrentTag->ComponentName) && Row->Value->Equals(CurrentTag->Tag))
                {
                    TagCategoryAlreadyAdded = true;
                    break;
                }
            }

            if (!TagCategoryAlreadyAdded)
            {
                TSharedPtr<FString> TagDisplayString = MakeShared<FString>(CurrentTag->ComponentName + " - " + CurrentTag->Tag);
                TagFilteredSuggestions.Add(TagDisplayString);
            }
        }
    }
    // Sort array
    TagFilteredSuggestions.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B) { return *A < *B; });

    // Refresh the tags suggestion list view
    TagSearchBox->RequestListRefresh();
}

void SCreationScreen::OnTemplateSearchTextChanged(const FText& NewText)
{
    // Clear previous suggestions
    TemplateFilteredSuggestions.Empty();
    FString CurrentInput = NewText.ToString();

    // loop through tags list
    for (const TSharedPtr<FAssetTemplate>& CurrentTemplate : TemplateList)
    {
        bool CategoryAlreadyAdded = false;
        if (CurrentTemplate->TemplateName.Contains(CurrentInput) || CurrentTemplate->ComponentName.Contains(CurrentInput) || CurrentInput.IsEmpty())
        {
            // Loop through rows in the customizations table
            for (const TSharedPtr<FKeywordTableRow> Row : CustomizationTable->TableRows)
            {
                // Check if subcategory tag already assigned
                if (Row->Customization->Equals(CurrentTemplate->ComponentName) && Row->Value->Equals(CurrentTemplate->TemplateName))
                {
                    CategoryAlreadyAdded = true;
                    break;
                }
            }

            if (!CategoryAlreadyAdded)
            {
                TSharedPtr<FString> TemplateDisplayString = MakeShared<FString>(CurrentTemplate->ComponentName + " - " + CurrentTemplate->TemplateName);
                TemplateFilteredSuggestions.Add(TemplateDisplayString);
            }
        }
    }

    // Sort array
    TemplateFilteredSuggestions.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B) { return *A < *B; });

    // Refresh the tags suggestion list view
    TemplateSearchBox->RequestListRefresh();
}

void SCreationScreen::OnParamSearchTextChanged(const FText& NewText)
{
    ParameterFilteredSuggestions.Empty();
    bool TagCategoryAlreadyAdded = false;
    FString CurrentInput = NewText.ToString();
    
    for (const TSharedPtr<FAssetParameter>& CurrentParameter : ParameterList)
    {
        if (CurrentParameter->FullName.Contains(CurrentInput) || CurrentInput.IsEmpty())
        {
            // Loop through rows in the customizations table
            for (const TSharedPtr<FKeywordTableRow> Row : CustomizationTable->TableRows)
            {
                // Check if subcategory tag already assigned
                if (Row->Customization->Equals(CurrentParameter->FullName))
                {
                    TagCategoryAlreadyAdded = true;
                    break;
                }
            }

            if (!TagCategoryAlreadyAdded)
            {
                TSharedPtr<FString> ParameterFullName = MakeShared<FString>(CurrentParameter->FullName);
                ParameterFilteredSuggestions.Add(ParameterFullName);
            }
        }
    }

    // Sort array
    ParameterFilteredSuggestions.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B) { return *A < *B; });

    // Refresh the list view
    ParameterSearchBox->RequestListRefresh();
}

void SCreationScreen::OnTagSelected(TSharedPtr<FString> TagSelection)
{
    FString ComponentName;
    FString TagName;

    TagSelection->Split(TEXT("-"), &ComponentName, &TagName);

    ComponentName = ComponentName.TrimStartAndEnd();
    TagName = TagName.TrimStartAndEnd();

    for (const TSharedPtr<FAssetTag>& CurrentTag : TagList)
    {
        if (CurrentTag->Tag.Equals(TagName) && CurrentTag->ComponentName.Equals(ComponentName))
        {
            CustomizationTable->AddTagToTable(CurrentTag);
        }
    }    
}

void SCreationScreen::OnTemplateSelected(TSharedPtr<FString> TemplateSelection)
{
    FString ComponentName;
    FString TemplateName;

    TemplateSelection->Split(TEXT("-"), &ComponentName, &TemplateName);

    ComponentName = ComponentName.TrimStartAndEnd();
    TemplateName = TemplateName.TrimStartAndEnd();

    for (const TSharedPtr<FAssetTemplate>& CurrentTemplate : TemplateList)
    {
        bool TagCategoryAlreadyAdded = false;
        if (CurrentTemplate->TemplateName.Equals(TemplateName) && CurrentTemplate->ComponentName.Equals(ComponentName))
        {
            CustomizationTable->AddTemplateToTable(CurrentTemplate);
        }
    }
}

void SCreationScreen::OnParameterSelected(TSharedPtr<FString> ParameterSelection)
{
    FString AssetName;
    FString ParameterName;

    ParameterSelection->Split(TEXT("/"), &AssetName, &ParameterName);

    AssetName = AssetName.TrimStartAndEnd();
    ParameterName = ParameterName.TrimStartAndEnd();

    for (const TSharedPtr<FAssetParameter>& CurrentParameter : ParameterList)
    {
        bool TagCategoryAlreadyAdded = false;
        if (CurrentParameter->Name.Equals(ParameterName) && CurrentParameter->AssetName.Equals(AssetName))
        {
            CustomizationTable->AddParameterToTable(CurrentParameter);
        }
    }
}

// ------------------------------
// --- HELPER METHODS
// ------------------------------

void SCreationScreen::SetAPIKey(FString apiKey) { APIKey = apiKey; }
void SCreationScreen::SetModelOptions(TArray<TSharedPtr<FString>> newModelOptions)
{
    ModelOptions = newModelOptions;

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
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
            {
                FPlatformProcess::Sleep(3);
                SendAPIRequest_AttributeName();
            });
    }
}

int SCreationScreen::CompareStrings(FString Str1, FString Str2)
{
    int minSize = FMath::Min(Str1.Len(), Str2.Len());

    for (int i = 0; i < minSize; i++)
    {
        if (StringMap[Str1[i]] == StringMap[Str2[i]])
            continue;
        return StringMap[Str1[i]] - StringMap[Str2[i]];
    }

    return Str1.Len() - Str2.Len();
}

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

FReply SCreationScreen::ShowWarningWindow(FString WarningMessage)
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
            .Text(FText::FromString(WarningMessage))
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
                                    TagSearchBox->SetText(FText::GetEmpty());
                                    TemplateSearchBox->SetText(FText::GetEmpty());
                                    ParameterSearchBox->SetText(FText::GetEmpty());
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
    //JsonData += "    \"count\": \"" + FString::FromInt(BatchCount) + "\",\r\n";
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

        FString CurrentKey = pair.Key == "All" ? "*" : pair.Key;
        JsonData += "        \"" + CurrentKey + "\": [";
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

    JsonData += "\r\n    },\r\n";
    JsonData += "    \"seed\": " + FString::FromInt(Seed) + "\r\n";
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
    FString Url = URLHelper::CreateComponent;
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
                FString LinkPlaceholderString;
                for (int32 i = 0; i < BatchCount; i++) 
                {
                    LinkPlaceholderString += "link" + FString::FromInt(i) + " ";
                }
                FString TextToSave = *CurrentModel + TEXT(" ") + CurrentDateTime + TEXT(" ") + status + TEXT(" ") + *APIResponseID + TEXT(" ") + FString::FromInt(BatchCount) + TEXT(" ") + LinkPlaceholderString;

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



void SCreationScreen::SendAPIRequest_AttributeName()
{
    FString Url = URLHelper::GetAttributesWithName;
    FString Parameters = "?name=" + *CurrentModel;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url + Parameters);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("X-RapidAPI-Key", APIKey);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &SCreationScreen::OnAPIRequestAttributeNameCompleted);
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url + Parameters));
}

void SCreationScreen::OnAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("Inside OnAPIRequestAttributeNameCompleted. bWasSuccessful: %s, Response Code: %d"), bWasSuccessful ? TEXT("True") : TEXT("False"), Response.IsValid() ? Response->GetResponseCode() : -1);

    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        // Parse JSON response
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        ModelComponentKeysList.Empty();
        TagList.Empty();
        TemplateList.Empty();
        ParameterList.Empty();

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("JSON Deserialization successful."));

            // Loop over main categories like "Stair"
            for (const auto& Model : JsonObject->Values)
            {
                TSharedPtr<FJsonObject> Components = Model.Value->AsObject();

                // Loop over sub-categories like "stair_upper"
                for (const auto& ModelComponent : Components->Values)
                {
                    FString ModelComponentKey = ModelComponent.Key;
                    ModelComponentKeysList.Add(MakeShared<FString>(ModelComponentKey));

                    TSharedPtr<FJsonObject> ModelComponentObj = ModelComponent.Value->AsObject();
                    const TArray<TSharedPtr<FJsonValue>>* AssetsArray;
                    if (ModelComponentObj->TryGetArrayField("assets", AssetsArray))
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
                                    bool bSameTagExists = false;
                                    bool bSameComponentTagExists = false;
                                    bool bStarComponentForTagExists = false;
                                    FString TagString = TagValue->AsString();

                                    FAssetTag NewAssetTag;
                                    NewAssetTag.ComponentName = ModelComponentKey;
                                    NewAssetTag.AssetName = AssetName;
                                    NewAssetTag.Tag = TagString;

                                    // Search for existing tags to generate * option
                                    for (TSharedPtr<FAssetTag> ExistingTag : TagList) 
                                    {
                                        if (ExistingTag->Tag == NewAssetTag.Tag)
                                        {
                                            bSameTagExists = true;
                                            if (ExistingTag->ComponentName == NewAssetTag.ComponentName) bSameComponentTagExists = true;
                                            if (ExistingTag->ComponentName == "All") bStarComponentForTagExists = true;
                                        }
                                    }

                                    // Add new tag if it doesnt exist for current component
                                    if (!bSameComponentTagExists)
                                    {
                                        TagList.Add(MakeShared<FAssetTag>(NewAssetTag));
                                        TSharedPtr<FString> TagDisplayString = MakeShared<FString>(NewAssetTag.ComponentName + " - " + NewAssetTag.Tag);
                                        TagFilteredSuggestions.Add(TagDisplayString);
                                    }

                                    /* Uncomment this to add All (*) tags
                                    // Add star component tag to pick this tag for every component
                                    if (!bStarComponentForTagExists && bSameTagExists)
                                    {
                                        FAssetTag StarAssetTag;
                                        StarAssetTag.ComponentName = "All";
                                        StarAssetTag.AssetName = "All";
                                        StarAssetTag.Tag = TagString;

                                        TagList.Add(MakeShared<FAssetTag>(StarAssetTag));
                                        TSharedPtr<FString> StarTagDisplayString = MakeShared<FString>(StarAssetTag.ComponentName + " - " + StarAssetTag.Tag);
                                        TagFilteredSuggestions.Add(StarTagDisplayString);
                                    }
                                    */
                                }
                            }

                            // Access the "templates"
                            const TArray<TSharedPtr<FJsonValue>>* TemplatesArray;
                            if (AssetObj->TryGetArrayField("templates", TemplatesArray))
                            {
                                for (const auto& TemplateValue : *TemplatesArray)
                                {
                                    bool bSameTemplateExists = false;
                                    bool bSameComponentTemplateExists = false;
                                    bool bStarComponentForTemplateExists = false;
                                    FString TemplateString = TemplateValue->AsString();

                                    FAssetTemplate NewAssetTemplate;
                                    NewAssetTemplate.ComponentName = ModelComponentKey;
                                    NewAssetTemplate.AssetName = AssetName;
                                    NewAssetTemplate.TemplateName = TemplateString;

                                    for (TSharedPtr<FAssetTemplate> ExistingTemplate : TemplateList)
                                    {
                                        if (ExistingTemplate->TemplateName == NewAssetTemplate.TemplateName)
                                        {
                                            bSameTemplateExists = true;
                                            if (ExistingTemplate->ComponentName == NewAssetTemplate.ComponentName) bSameComponentTemplateExists = true;
                                            if (ExistingTemplate->ComponentName == "All") bStarComponentForTemplateExists = true;
                                        }
                                    }

                                    // Add new template if it doesnt exist for current component
                                    if (!bSameComponentTemplateExists)
                                    {
                                        TemplateList.Add(MakeShared<FAssetTemplate>(NewAssetTemplate));
                                        TSharedPtr<FString> TemplateDisplayString = MakeShared<FString>(NewAssetTemplate.ComponentName + " - " + NewAssetTemplate.TemplateName);
                                        TemplateFilteredSuggestions.Add(TemplateDisplayString);
                                    }

                                    /* Uncomment this to add All (*) templates
                                    // Add star component template to pick this template for every component
                                    if (!bStarComponentForTemplateExists && bSameTemplateExists) 
                                    {
                                        FAssetTemplate StarAssetTemplate;
                                        StarAssetTemplate.ComponentName = "All";
                                        StarAssetTemplate.AssetName = "All";
                                        StarAssetTemplate.TemplateName = TemplateString;

                                        TemplateList.Add(MakeShared<FAssetTemplate>(StarAssetTemplate));
                                        TSharedPtr<FString> StarTemplateDisplayString = MakeShared<FString>(StarAssetTemplate.ComponentName + " - " + StarAssetTemplate.TemplateName);
                                        TemplateFilteredSuggestions.Add(StarTemplateDisplayString);
                                    }
                                    */
                                }
                            }

                            // Access the "parameters"
                            const TArray<TSharedPtr<FJsonValue>>* ParametersArray;
                            if (AssetObj->TryGetArrayField("parameters", ParametersArray))
                            {
                                for (const auto& ParameterValue : *ParametersArray)
                                {
                                    TSharedPtr<FJsonObject> ParameterObj = ParameterValue->AsObject();
                                    FAssetParameter NewParameter;
                                        
                                    NewParameter.ComponentName = ModelComponentKey;
                                    NewParameter.AssetName = AssetName;
                                    NewParameter.Name = ParameterObj->GetStringField("name");
                                    NewParameter.Type = ParameterObj->GetStringField("type");

                                    // not needed for now
                                    //NewParameter.Attribute = ParameterObj->GetStringField("attribute");
                                    NewParameter.FullName = NewParameter.AssetName + "/" + NewParameter.Name;;
                                    // Extract the range of the parameters if available

                                    const TArray<TSharedPtr<FJsonValue>>* RangeArray;
                                    if (ParameterObj->TryGetArrayField("range", RangeArray))
                                    {
                                        // Extract the first two range values
                                        float MinValue = (*RangeArray)[0]->AsNumber();
                                        float MaxValue = (*RangeArray)[1]->AsNumber();

                                        NewParameter.Range = FVector2D(MinValue, MaxValue);

                                        if (MaxValue - MinValue != 0)
                                        {
                                            ParameterList.Add(MakeShared<FAssetParameter>(NewParameter));
                                            ParameterFilteredSuggestions.Add(MakeShared<FString>(NewParameter.FullName));
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
    else
    {
        ShowConnectionErrorWarning();
        UE_LOG(LogTemp, Error, TEXT("API request was not successful. Response code: %d, Error: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
    }

    // Give feedback
    TagSearchBox->SelectionsLoadingComplete();
    TemplateSearchBox->SelectionsLoadingComplete();
    ParameterSearchBox->SelectionsLoadingComplete();
}

void SCreationScreen::SendAPIRequest_ModelNames()
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
    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
            {
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    ModelOptions.Empty(); // Clear previous options

                    /*
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
                            }
                        }
                    }
                    */

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
        });
    HttpRequest->ProcessRequest();
    UE_LOG(LogTemp, Log, TEXT("Sending request to URL: %s"), *(Url));
}

FReply SCreationScreen::ShowConnectionErrorWarning()
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
                .Text(FText::FromString("Couldn't connect to OPUS API \n Make sure you have a stable internet connection"))
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
                        .Text(LOCTEXT("OkButton", "Ok"))
                        .OnClicked_Lambda([this, WarningWindowPtr]()
                            {
                                if (WarningWindowPtr.IsValid())
                                {
                                    WarningWindowPtr->RequestDestroyWindow();
                                    
                                }
                                SendAPIRequest_AttributeName();
                                return FReply::Handled();
                            })
                ]
        ]);
    return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE
