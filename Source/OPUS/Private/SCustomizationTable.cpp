// Fill out your copyright notice in the Description page of Project Settings.


#include "SCustomizationTable.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SSlider.h"
#include "Misc/DefaultValueHelper.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCustomizationTable::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
        SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SBox)
                    .WidthOverride(500) // Adjust this based on your needs
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
                                    .OnGenerateRow(this, &SCustomizationTable::GenerateTableRow)
                                    .SelectionMode(ESelectionMode::Single)
                                    .HeaderRow
                                    (
                                        SNew(SHeaderRow)
                                        + SHeaderRow::Column("CustomizationsColumn")
                                        .DefaultLabel(LOCTEXT("CustomizationsHeader", "Current Model Customizations"))
                                        .FillWidth(0.7f)

                                        + SHeaderRow::Column("InputColumn")
                                        .DefaultLabel(LOCTEXT("InputColumnHeader", "Input"))
                                        .FillWidth(0.6f)
                                    )
                            ]
                    ]
            ]
	];
}

TSharedRef<ITableRow> SCustomizationTable::GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable)
{
    return 
        SNew(STableRow<TSharedPtr<FKeywordTableRow>>, OwnerTable)
        [
            SNew(SHorizontalBox)

                // Keyword column
                + SHorizontalBox::Slot()
                .FillWidth(1.5)
                .Padding(5, 0, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(*(RowData->Customization)))
                ]

                // Value column
                + SHorizontalBox::Slot()
                .FillWidth(0.5)
                .Padding(0, 0, 5, 0)
                [
                    SAssignNew(RowData->AttachedSlider, SSlider)
                    .MinValue(RowData->ParameterRange.X)
                    .MaxValue(RowData->ParameterRange.Y)
                    .Value(RowData->ParameterRange.X)
                    .Visibility(RowData->IsParameter ? EVisibility::Visible : EVisibility::Hidden)
                    .IsFocusable(true)
                    .OnValueChanged(this, &SCustomizationTable::OnSliderValueChanged, RowData)
                ]

                + SHorizontalBox::Slot()
                .FillWidth(0.6)
                .Padding(0, 0, 5, 0)
                [
                    SAssignNew(RowData->AttachedTextBox, SEditableTextBox)
                    .Text(FText::FromString(*(RowData->Value)))
                    .IsEnabled(RowData->IsParameter)
                    .OnTextCommitted(this, &SCustomizationTable::OnValueTextCommitted, RowData)
                ]

                // Remove button
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .OnClicked_Lambda([this, RowData] () -> FReply{
                        TableRows.Remove(RowData);
                        TableView->RequestListRefresh();
                        return FReply::Handled();
                    })
                    .Text(FText::FromString("x"))
                ]
        ];
}

void SCustomizationTable::AddCustomizationToTable(TSharedPtr<FString> Customization, bool IsParameter)
{
    if (IsParameter)
    {
        ParseParameterInput(Customization);
    }
    else
    {
        ParseTagInput(Customization);
    }

    // Refresh table
    TableView->RequestListRefresh();
    TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

FText SCustomizationTable::GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const
{
    return FText::FromString(*RowData->Value);
}

void SCustomizationTable::ResetTable()
{
    TableRows.Empty();
    if (TableView.IsValid())
    {
        TableView->RequestListRefresh();
    }
}

void SCustomizationTable::ParseTagInput(TSharedPtr<FString> TagInput)
{
    FString TagBoxContent = *TagInput;
    TArray<FString> ParsedStrings;

    bool bFound = false;

    if (TagBoxContent.ParseIntoArray(ParsedStrings, TEXT(" - "), true) == 2)
    {
        TSharedPtr<FString> MainCategory = MakeShared<FString>(ParsedStrings[0].TrimStartAndEnd());
        TSharedPtr<FString> CurrentTag = MakeShared<FString>(ParsedStrings[1].TrimStartAndEnd());

        for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
        {
            if (existingRow->Customization->Equals(*MainCategory))
            {
                existingRow->Value = CurrentTag;  // Update the value only
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            // Add new row if tag doesnt exist in table
            TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
            newRow->Customization = MainCategory;
            newRow->Value = CurrentTag;
            newRow->IsParameter = false;
            newRow->ParameterRange = FVector2D::Zero();
            TableRows.Add(newRow);
            UE_LOG(LogTemp, Warning, TEXT("New Row Added! Main Category: %s, Tag: %s"), **newRow->Customization, **newRow->Value);
        }

        TableView->RequestListRefresh();
        TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("The Tag input format is invalid!"));
    }
}

void SCustomizationTable::ParseParameterInput(TSharedPtr<FString> ParameterInput)
{
    bool bFoundParam = false;

    for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
    {
        if (existingRow->Customization->Equals(*ParameterInput))
        {
            bFoundParam = true;
            break;
        }
    }

    if (!bFoundParam)
    {
        TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
        newRow->Customization = ParameterInput;
        newRow->IsParameter = true;
        newRow->ParameterRange = GetParameterRange(ParameterInput);

        FString ParameterValue = FString::SanitizeFloat(newRow->ParameterRange.X);
        newRow->Value = MakeShared<FString>(ParameterValue);
        TableRows.Add(newRow);

        TableView->RequestListRefresh();
        TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
        UE_LOG(LogTemp, Warning, TEXT("New Parameter Row Added! Parameter: %s, Value: %s"), **newRow->Customization, **newRow->Value);
    }

}

FVector2D SCustomizationTable::GetParameterRange(TSharedPtr<FString> ParameterName)
{
    return *(ParameterRanges.Find(*ParameterName));
}

void SCustomizationTable::OnSliderValueChanged(float NewValue, TSharedPtr<FKeywordTableRow> RowData)
{
    RowData->Value = MakeShared<FString>(FString::SanitizeFloat(NewValue));
    RowData->AttachedTextBox->SetText(FText::FromString(*(RowData->Value)));
}

void SCustomizationTable::OnValueTextCommitted(const FText& NewText, const ETextCommit::Type InTextAction, TSharedPtr<FKeywordTableRow> RowData)
{
    FString ParamInputContent = NewText.ToString();
    float InputValue;

    // Handle input starting with decimal point, make ".2" into "0.2"
    if (ParamInputContent.StartsWith("."))
    {
        FString DecimalString = "0";
        DecimalString.Append(ParamInputContent);
        ParamInputContent = DecimalString;
    }

    // Parse Input
    if (FDefaultValueHelper::ParseFloat(ParamInputContent, InputValue))
    {
        if (InputValue >= RowData->ParameterRange.X && InputValue <= RowData->ParameterRange.Y)
        {
            //RowData->Value = MakeShared<FString>(FString::SanitizeFloat(InputValue));
            RowData->AttachedSlider->SetValue(InputValue);
            RowData->Value = MakeShared<FString>(FString::SanitizeFloat(InputValue));
        }
    }
    else 
    {
        RowData->AttachedSlider->SetValue(RowData->ParameterRange.X);
        RowData->Value = MakeShared<FString>(FString::SanitizeFloat(RowData->ParameterRange.X));
    }

    // Set cleaned text
    RowData->AttachedTextBox->SetText(FText::FromString(*(RowData->Value)));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE