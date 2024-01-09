// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Http.h"
#include "EditorNotificationHelper.h"
#include "SFilteredSelectionTextBox.h"
#include"SCustomizationTable.h"

DECLARE_MULTICAST_DELEGATE(FOnQueueScreenEnabled)
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

// ------------------------------
// --- BUTTON CALLBACK METHODS
// ------------------------------
	FReply LogoutButtonClicked();
	FReply CreateButtonClicked();
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
	void SendAPIRequest_AttributeName();
	void OnAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SendAPIRequest_ModelNames();
	void OnAPIRequestModelNamesCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

// ------------------------------
// --- COMBOBOX METHODS ---------
// ------------------------------

	TSharedRef<SWidget> GenerateComboBoxItem(TSharedPtr<FString> Item);
	void ModelComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo);
	void FileTypeComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo);
	void TextureSizeComboBoxSelectionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo);
	void SetUpTextureSizeComboBox();
	void SetUpFileTypeComboBox();


// ------------------------------
// --- SEARCHBOX METHODS --------
// ------------------------------

	void OnParamSearchTextChanged(const FText& NewText);
	void OnTagsSearchTextChanged(const FText& NewText);
	void OnTemplateSearchTextChanged(const FText& NewText);
	void OnTagSelected(TSharedPtr<FString> SelectedTag);
	void OnTemplateSelected(TSharedPtr<FString> SelectedTemplate);
	void OnParameterSelected(TSharedPtr<FString> ParameterSelection);
	void OnTextCommittedInParameterInput(const FText& Text, ETextCommit::Type CommitMethod);

// ------------------------------
// --- HELPER METHODS -----------
// ------------------------------

	int CompareStrings(FString Str1, FString Str2);
	FString ConstructCreateRequestBody();
	bool IsCustomization(const FString& Keyword);
	FReply ShowWarningWindow(FString WarningMessage);
	FReply ShowResetCustomizationWarning();
	FReply ShowConnectionErrorWarning();
	EVisibility GetParamInputBoxVisibility() const;
	FText GetParamHintText(FVector2D ParameterRange) const;
	FText GetCurrentModel() const;
	FText GetCurrentFileType() const;
	FText GetCurrentTextureSize() const;
	bool TagExistsInFilteredList(TSharedPtr<FString> TagString);

private:
	FString APIKey;
	EditorNotificationHelper NotificationHelper;
	TMap<char, int> StringMap;

	// MEMBER VARIABLE DEFINITIONS
	FVector2D CurrentParameterRange;				// To store the range of the currently selected parameter.
	FString APIResponseID;
	FString SecondAPIResponse;
	FString ThirdAPIResponse;

	//BUTTONS
	TSharedPtr<SButton> CreateButton;

	// COMBOBOX DEFINITIONS
	// Model Box
	TSharedPtr<FString> CurrentModel;
	TArray<TSharedPtr<FString>> ModelOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ModelComboBox;
	TArray<TSharedPtr<FString>> Structures;

	// File Type Box
	TSharedPtr<SComboBox<TSharedPtr<FString>>> FileTypeComboBox;
	TArray<TSharedPtr<FString>> AvailableFileTypes;
	TSharedPtr<FString> CurrentFileType;

	// Texture Size Box
	TSharedPtr<SComboBox<TSharedPtr<FString>>> TextureSizeComboBox;
	TArray<TSharedPtr<FString>> AvailableTextureSizes;
	TSharedPtr<FString> CurrentTextureSize;

	// TABLE RELATED
	TSharedPtr<SCustomizationTable> CustomizationTable;

	// Search boxes
	TSharedPtr<SFilteredSelectionTextBox> TagSearchBox;
	TSharedPtr<SFilteredSelectionTextBox> TemplateSearchBox;
	TSharedPtr<SFilteredSelectionTextBox> ParameterSearchBox;

	// Filtered search box suggestions
	TArray<TSharedPtr<FString>> TagFilteredSuggestions;
	TArray<TSharedPtr<FString>> TemplateFilteredSuggestions;
	TArray<TSharedPtr<FString>> ParameterFilteredSuggestions;

	// Tags, Templates, Parameters list
	TArray<TSharedPtr<FAssetTag>> TagList;
	TArray<TSharedPtr<FAssetTemplate>> TemplateList;
	TArray<TSharedPtr<FAssetParameter>> ParameterList;
	TArray<TSharedPtr<FString>> ModelComponentKeysList;
};
