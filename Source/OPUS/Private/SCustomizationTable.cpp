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

                // Value slider
                + SHorizontalBox::Slot()
                .FillWidth(0.5)
                .Padding(0, 0, 5, 0)
                [
                    SAssignNew(RowData->AttachedSlider, SSlider)
                    .MinValue(RowData->ParameterRange.X)
                    .MaxValue(RowData->ParameterRange.Y)
                    .Value(RowData->ParameterRange.X)
                    .StepSize(RowData->ValueType->Equals("int") ? 1.0 : 0.001)
                    .Visibility(RowData->IsParameter ? EVisibility::Visible : EVisibility::Hidden)
                    .IsFocusable(true)
                    .OnValueChanged(this, &SCustomizationTable::OnSliderValueChanged, RowData)
                ]

                // Value box
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

void SCustomizationTable::AddTagToTable(TSharedPtr<FAssetTag> TagCustomization)
{
    if (!DoesCustomizationExist(TagCustomization->ComponentName, TagCustomization->Tag))
    {
        // Add new row if tag doesnt exist in table
        TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
        newRow->Customization = MakeShared<FString>(TagCustomization->ComponentName);
        newRow->Value = MakeShared<FString>(TagCustomization->Tag);
        newRow->ValueType = MakeShared<FString>("string");
        newRow->IsParameter = false;
        newRow->ParameterRange = FVector2D::Zero();
        TableRows.Add(newRow);
        UE_LOG(LogTemp, Warning, TEXT("New Row Added! Component: %s, Tag: %s"), **newRow->Customization, **newRow->Value);
    }

    // Refresh table
    TableView->RequestListRefresh();
    TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SCustomizationTable::AddTemplateToTable(TSharedPtr<FAssetTemplate> TemplateCustomization)
{
    if (!DoesCustomizationExist(TemplateCustomization->ComponentName, TemplateCustomization->TemplateName))
    {
        // Add new row if tag doesnt exist in table
        TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
        newRow->Customization = MakeShared<FString>(TemplateCustomization->ComponentName);
        newRow->Value = MakeShared<FString>(TemplateCustomization->TemplateName);
        newRow->ValueType = MakeShared<FString>("string");
        newRow->IsParameter = false;
        newRow->ParameterRange = FVector2D::Zero();
        TableRows.Add(newRow);
        UE_LOG(LogTemp, Warning, TEXT("New Row Added! Component: %s, Taemplate %s"), **newRow->Customization, **newRow->Value);
    }

    // Refresh table
    TableView->RequestListRefresh();
    TableView->Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SCustomizationTable::AddParameterToTable(TSharedPtr<FAssetParameter> ParameterCustomization)
{
    if (!DoesParameterCustomizationExist(ParameterCustomization->FullName))
    {
        TSharedPtr<FKeywordTableRow> newRow = MakeShared<FKeywordTableRow>();
        newRow->Customization = MakeShared<FString>(ParameterCustomization->FullName);
        newRow->IsParameter = true;
        newRow->ValueType = MakeShared<FString>(ParameterCustomization->Type);
        FVector2D Range = ParameterCustomization->Range;
        newRow->ParameterRange = Range;
        FString ParameterValue;

        if (newRow->ValueType->Equals("int"))
        {
            int32 IntValue = FMath::FloorToInt32(newRow->ParameterRange.X);
            ParameterValue = FString::FromInt(IntValue); 
        }
        else
        {
            ParameterValue = FString::SanitizeFloat(newRow->ParameterRange.X);
        }
        newRow->Value = MakeShared<FString>(ParameterValue);
        
        TableRows.Add(newRow);
        UE_LOG(LogTemp, Warning, TEXT("New Parameter Row Added! Parameter: %s, Value: %s"), **newRow->Customization, **newRow->Value);
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

bool SCustomizationTable::DoesCustomizationExist(FString Customization, FString Value)
{
    for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
    {
        if (existingRow->Customization->Equals(Customization) && existingRow->Value->Equals(Value))
        {
            return true;
        }
    }
    return false;
}

bool SCustomizationTable::DoesParameterCustomizationExist(FString Customization)
{
    for (TSharedPtr<FKeywordTableRow>& existingRow : TableRows)
    {
        if (existingRow->Customization->Equals(Customization))
        {
            return true;
        }
    }
    return false;
}

void SCustomizationTable::OnSliderValueChanged(float NewValue, TSharedPtr<FKeywordTableRow> RowData)
{
    if (RowData->ValueType->Equals("int"))
    {
        int32 IntValue = FMath::FloorToInt32(NewValue);
        RowData->Value = MakeShared<FString>(FString::FromInt(IntValue));
        RowData->AttachedTextBox->SetText(FText::FromString(*(RowData->Value)));
    }
    else 
    {
        RowData->Value = MakeShared<FString>(FString::SanitizeFloat(NewValue));
        RowData->AttachedTextBox->SetText(FText::FromString(*(RowData->Value)));
    }
    
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