// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSlider.h"


struct FKeywordTableRow
{
	TSharedPtr<FString> Customization;
	TSharedPtr<FString> Value;
	bool IsParameter;
	FVector2D ParameterRange;
	TSharedPtr<SSlider> AttachedSlider;
	TSharedPtr<SEditableTextBox> AttachedTextBox;
};

class OPUS_API SCustomizationTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomizationTable)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void AddCustomizationToTable(TSharedPtr<FString> Customization, bool IsParameter);
	void ResetTable();

public:

	TArray<TSharedPtr<FKeywordTableRow>> TableRows;
	TSharedPtr<SListView<TSharedPtr<FKeywordTableRow>>> TableView;
	TMap<FString, FVector2D> ParameterRanges;		// A map to hold parameter names and their ranges.

private:
	// Table logic
	TSharedRef<ITableRow> GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const;
	FVector2D GetParameterRange(TSharedPtr<FString> ParameterName);
	bool IsParameterRow(TSharedPtr<FKeywordTableRow> RowData) const;

	// Parsing input strings
	void ParseTagInput(TSharedPtr<FString> TagInput);
	void ParseParameterInput(TSharedPtr<FString> TagInput);
	
	// Event handling
	void OnSliderValueChanged(float NewValue, TSharedPtr<FKeywordTableRow> RowData);
	void OnValueTextCommitted(const FText& NewText, const ETextCommit::Type InTextAction, TSharedPtr<FKeywordTableRow> RowData);
};
