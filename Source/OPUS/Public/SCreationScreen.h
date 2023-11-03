// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Http.h"
#include "EditorNotificationHelper.h"
#include "SFilteredSelectionTextBox.h"


DECLARE_MULTICAST_DELEGATE(FOnQueueScreenEnabled);
DECLARE_MULTICAST_DELEGATE(FOnLogout);
DECLARE_MULTICAST_DELEGATE(FOnJobCompleted);

class OPUS_API SCreationScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCreationScreen)
		: _APIKey("")
	{}
	SLATE_ARGUMENT(FString, APIKey)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void SetAPIKey(FString apiKey);

	// Delegate 
	FOnQueueScreenEnabled OnQueueScreenEnabledDelegate;
	FOnLogout OnLogoutDelegate;

private:
	//STRUCT DEFINITIONS
	struct Param {
		TArray<float> range;
		FString type;
		FString attribute;
	};
	struct FPair
	{
		FString SubCategory;
		FString Tag;
	};
	struct FKeywordTableRow
	{
		TSharedPtr<FString> Keyword;
		TSharedPtr<FString> Value;
	};
	
private: 

// ------------------------------
// --- BUTTON CALLBACK METHODS
// ------------------------------
	FReply LogoutButtonClicked();
	FReply CreateButtonClicked();
	FReply ApplyFeatureButtonClicked();
	FReply ResetFeaturesButtonClicked();
	FReply QueueButtonClicked();
	FReply ReturnButtonClicked();
	FReply RefreshCachingButtonClicked();
	FReply EmptyQButtonClicked();

// ------------------------------
// --- API REQUEST METHODS
// ------------------------------

	void SendAPIRequest_Create();
	void OnAPIRequestCreateCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SendSecondAPIRequest_JobResult(FString jobID);
	void SendThirdAPIRequest_AttributeName();
	void OnThirdAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SendForthAPIRequest_ModelNames();
	void OnForthAPIRequestModelNamesCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

// ------------------------------
// --- COMBOBOX METHODS ---------
// ------------------------------

	TSharedRef<SWidget> GenerateComboBoxItem(TSharedPtr<FString> Item);
	void ComboBoxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
	FText GetCurrentItem() const;
	void KeywordsComboBoxSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

// ------------------------------
// --- TABLE METHODS ------------
// ------------------------------

	TSharedRef<ITableRow> GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const;
	void ResetTable();

// ------------------------------
// --- SEARCHBOX METHODS --------
// ------------------------------

	void OnParamSearchTextChanged(const FText& NewText);
	void OnTagsSearchTextChanged(const FText& NewText);
	void ParseTagInput(const FText& TagInputText);
	void ParseParameterInput(const FText& TagInputText);

	void OnTagSelected(TSharedPtr<FString> SelectedTag);
	void OnParameterSelected(TSharedPtr<FString> ParameterSelection);
	void OnTextCommittedInParameterInput(const FText& Text, ETextCommit::Type CommitMethod);

// ------------------------------
// --- HELPER METHODS -----------
// ------------------------------

	FString ConstructJSONData();
	bool IsParameter(const FString& Keyword);
	FReply ShowWarningWindow(FString warningMessage);
	EVisibility GetParamInputBoxVisibility() const;
	FText GetParamHintText() const;

private:
	FString APIKey;
	EditorNotificationHelper NotificationHelper;

	// MEMBER VARIABLE DEFINITIONS
	TMap<FString, FVector2D> ParameterRanges;		// A map to hold parameter names and their ranges.
	FVector2D CurrentParameterRange;				// To store the range of the currently selected parameter.
	TSharedPtr<SEditableTextBox> ParamInputBox;
	FString APIResponseID;
	FString JobStatus;
	FString NewJobStatus;
	FString SecondAPIResponse;
	FString ThirdAPIResponse;
	int32 CurrentQueueIndex = 0;
	int32 SelectedJobIndex = -1;
	bool IsFBXSelected = true;
	bool IsGLTFSelected = false;

	//BUTTONS
	TSharedPtr<SButton> CreateButton;

	//COMBOBOX DEFINITIONS
	TSharedPtr<FString> CurrentModel;
	TArray<TSharedPtr<FString>> ModelOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ModelComboBox;
	TArray<TSharedPtr<FString>> Structures;

	//TABLE RELATED
	TArray<TSharedPtr<FKeywordTableRow>> TableRows;
	TSharedPtr<SListView<TSharedPtr<FKeywordTableRow>>> TableView;

	// MEMBER DEFINITIONS FOR PARAM SEARCHBOX
	TArray<TSharedPtr<FString>> ParameterFilteredSuggestions;
	TSharedPtr<FString> SelectedParameterSuggestion;

	// Tag searchbox members
	TArray<TSharedPtr<FString>> TagFilteredSuggestions;
	TSharedPtr<FString> SelectedTagSuggestion;
	TArray<TSharedPtr<FPair>> TagsList;											// Source of possible tags
	TArray<TSharedPtr<FString>> MainCategoryKeysList;
	TArray<TSharedPtr<FString>> ParameterSuggestions;

	// Search revamped
	TSharedPtr<SFilteredSelectionTextBox> TagSearchBox;
	TSharedPtr<SFilteredSelectionTextBox> ParameterSearchBox;					// Search bar widget
};
