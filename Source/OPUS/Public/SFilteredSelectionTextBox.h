// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 *  Combined editable text box with scroll view
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FSuggestionSelectedDelegate, TSharedPtr<FString>);

class OPUS_API SFilteredSelectionTextBox : public SCompoundWidget
{
	using FOnGenerateRow = typename TSlateDelegates<FString>::FOnGenerateRow;

public:

	SLATE_BEGIN_ARGS(SFilteredSelectionTextBox)
		: _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _Padding(FMargin(2.0f))
		, _Text()
		, _ListItemsSource()
		, _OnTextChanged()
	{}
		
	SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
	SLATE_ARGUMENT(EVerticalAlignment, VAlign)

	SLATE_ATTRIBUTE(FMargin, Padding)
	SLATE_ATTRIBUTE(FText, Text)
	SLATE_ARGUMENT(const TArray<TSharedPtr<FString>>*, ListItemsSource)

	SLATE_EVENT(FOnTextChanged, OnTextChanged)
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	FText GetText() const;
	void SetText(const TAttribute<FText>& NewText);
	void RequestListRefresh();
	void SetListVisibility(EVisibility NewVisibility);
	void SelectionsLoadingStarted();
	void SelectionsLoadingComplete();

public:

	TSharedPtr<SEditableText> EditableTextBox;
	TSharedPtr<SListView<TSharedPtr<FString>>> SuggestionsListView;
	TArray<TSharedPtr<FString>> FilteredSuggestions;
	FSuggestionSelectedDelegate OnSuggestionSelected;

private:
	TSharedRef<ITableRow> GenerateSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable);
	FReply SelectionChanged(ECheckBoxState NewState);
	FReply OnSuggestionRowClicked(TSharedPtr<FString> Suggestion);
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
private:
	int32 KeyboardUserIndex;
	int LoadingIteration;
	const TArray<TSharedPtr<FString>>* ListItems;

};
