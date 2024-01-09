// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSlider.h"
#include "SCreationScreen.h"


struct FKeywordTableRow
{
	TSharedPtr<FString> Customization;
	TSharedPtr<FString> Value;
	TSharedPtr<FString> ValueType;
	bool IsParameter;
	FVector2D ParameterRange;
	TSharedPtr<SSlider> AttachedSlider;
	TSharedPtr<SEditableTextBox> AttachedTextBox;
};

//STRUCT DEFINITIONS
struct FAssetParameter
{
	FString ComponentName;
	FString AssetName;
	FString Name;
	FString FullName;
	FVector2D Range;
	FString Type;
	FString Attribute;
};
struct FAssetTag
{
	FString ComponentName;
	FString AssetName;
	FString Tag;
};
struct FAssetTemplate
{
	FString ComponentName;
	FString AssetName;
	FString TemplateName;
};

class OPUS_API SCustomizationTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomizationTable)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void ResetTable();
	void AddTagToTable(TSharedPtr<FAssetTag> TagCustomization);
	void AddTemplateToTable(TSharedPtr<FAssetTemplate> TemplateCustomization);
	void AddParameterToTable(TSharedPtr<FAssetParameter> ParameterCustomization);

public:

	TArray<TSharedPtr<FKeywordTableRow>> TableRows;
	TSharedPtr<SListView<TSharedPtr<FKeywordTableRow>>> TableView;

private:
	// Table logic
	TSharedRef<ITableRow> GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const;
	bool IsParameterRow(TSharedPtr<FKeywordTableRow> RowData) const;
	bool DoesCustomizationExist(FString Customization, FString Value);
	bool DoesParameterCustomizationExist(FString Customization);

	// Parsing input strings
	void ParseParameterInput(TSharedPtr<FString> TagInput);
	
	// Event handling
	void OnSliderValueChanged(float NewValue, TSharedPtr<FKeywordTableRow> RowData);
	void OnValueTextCommitted(const FText& NewText, const ETextCommit::Type InTextAction, TSharedPtr<FKeywordTableRow> RowData);
};
