#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableText.h" // For the search bar
#include "Widgets/Views/SListView.h" // For the suggestions list view
#include "Runtime/Online/HTTP/Public/Http.h"




class SGUIWidget : public SCompoundWidget
{
public:
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

    //SLATE UI SETUP
    SGUIWidget();
    virtual ~SGUIWidget() = default;
    SLATE_BEGIN_ARGS(SGUIWidget) {}
    SLATE_END_ARGS()
        void Construct(const FArguments& InArgs);


    //DOWNLOAD&UNZIP METHOD DEFINITIONS
    void DownloadAndUnzipMethod(const FString& URL, const FString& DateTime, const FString& JobName);


    // DEFINITIONS FOR THE SEARCH FUNCTIONALITY
    void OnParamSearchTextChanged(const FText& NewText);
    TSharedRef<ITableRow> GenerateParamSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable);
    FReply OnParamSuggestionRowClicked(TSharedPtr<FString> Suggestion);


    // TABLE DEFINITIONS
    void ResetTable();


    //QUEUE TAB DEFINITIONS 
    bool bIsQueueClicked = false;


private:
    //STRUCT DEFINITIONS
    struct FKeywordTableRow
    {
        TSharedPtr<FString> Keyword;
        TSharedPtr<FString> Value;
    };
    struct FQueueRow
    {
        FString Job;
        FString DateTime;
        FString Status;
        FString JobID;
        FString DownloadLink;

        FQueueRow(FString InJob, FString InDateTime, FString InStatus, FString InJobID, FString InDownloadLink)
            : Job(InJob), DateTime(InDateTime), Status(InStatus), JobID(InJobID), DownloadLink(InDownloadLink) {}
    };


    //TABLE DEFINITIONS
    FReply TableRow_OnClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, TSharedPtr<FQueueRow> InItem);
    void OnTagsSearchTextChanged(const FText& NewText);
    TSharedRef<ITableRow> GenerateTagSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable);
    FReply OnTagSuggestionRowClicked(TSharedPtr<FString> Suggestion);

    //HELPER METHOD DEFINITIONS
    bool IsParameter(const FString& Keyword);
    EVisibility GetParamInputBoxVisibility() const;
    FText GetParamHintText() const;
    FString ConstructJSONData();
    void RebuildWidget();
    void OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod);


    //QUEUE TAB DEFINITIONS
    TSharedPtr<SListView<TSharedPtr<FQueueRow>>> QueueListView;
    TArray<TSharedPtr<FQueueRow>> QueueData;
    TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FQueueRow> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    void ReadAndParseQueueFile();
    void QueueLoop();
    void WriteQueueToFile();


    //NOTIFICATION DEFINITIONS
    void ShowNotificationSuccess(const FText& NotificationText);
    void ShowNotificationFail(const FText& NotificationText);
    void ShowNotificationPending(const FText& NotificationText);
    TSharedPtr<SNotificationItem> PendingNotificationItem;


    //DOWNLOAD&UNZIP METHOD DEFINITIONS
    bool ExtractWith7Zip(const FString& ZipFile, const FString& DestinationDirectory);


    //COMBOBOX DEFINITIONS
    TSharedRef<SWidget> GenerateComboBoxItem(TSharedPtr<FString> Item);
    void ComboBoxSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
    FText GetCurrentItem() const;
    void KeywordsComboBoxSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    TSharedPtr<FString> CurrentItem;
    TArray<TSharedPtr<FString>> ComboBoxOptions;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBoxWidget;
    TArray<TSharedPtr<FString>> Structures;


    //TABLE RELATED
    TSharedRef<ITableRow> GenerateTableRow(TSharedPtr<FKeywordTableRow> RowData, const TSharedRef<STableViewBase>& OwnerTable);
    TArray<TSharedPtr<FKeywordTableRow>> TableRows;
    TSharedPtr<SListView<TSharedPtr<FKeywordTableRow>>> TableView;
    FText GetKeywordValue(TSharedPtr<FKeywordTableRow> RowData) const;
    TSharedPtr<FString> SelectedSuggestion;


    // MEMBER DEFINITIONS FOR PARAM SEARCHBOX
    TArray<TSharedPtr<FString>> FilteredSuggestions;
    TSharedPtr<SEditableText> SearchBox;  // Search bar widget
    TSharedPtr<SListView<TSharedPtr<FString>>> SuggestionsListView; // Suggestions list view widget


    // API REQUEST DEFINITIONS
    void SendAPIRequest_Create();
    void OnAPIRequestCreateCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void SendSecondAPIRequest_JobResult(FString jobID);
    void OnSecondAPIRequestJobResultCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString jobID);
    void SendThirdAPIRequest_AttributeName();
    void OnThirdAPIRequestAttributeNameCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void SendForthAPIRequest_ModelNames();
    void OnForthAPIRequestModelNamesCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);


    // MEMBER VARIABLE DEFINITIONS
    TMap<FString, FVector2D> ParameterRanges;  // A map to hold parameter names and their ranges.
    FVector2D CurrentParameterRange;          // To store the range of the currently selected parameter.
    TSharedPtr<SEditableTextBox> ParamInputBox;
    TSharedPtr<SEditableText> KeyField;
    FString APIResponseID;
    FString JobStatus;
    FString NewJobStatus;
    FString SecondAPILink;
    FString SecondAPIResponse;
    FString ThirdAPIResponse;
    FString StoredAPIKey;
    int32 CurrentQueueIndex = 0;
    int32 SelectedJobIndex = -1;
    bool bIsFBXSelected = true;
    bool bIsGLTFSelected = false;
    bool bIsLoggedIn;


    // MEMBER DEFINITIONS FOR TAG SEARCHBOX 
    TArray<TSharedPtr<FString>> TagFilteredSuggestions;
    TSharedPtr<SEditableText> TagsSearchBox;  // Tags search bar widget
    TSharedPtr<SListView<TSharedPtr<FString>>> TagsSuggestionsListView; // Tags suggestions list view widget
    TSharedPtr<FString> SelectedTagSuggestion;
    TArray<TSharedPtr<FPair>> TagsList; // Source of possible tags
    TArray<TSharedPtr<FString>> MainCategoryKeysList;
    TArray<TSharedPtr<FString>> ParameterSuggestions;


    // BUTTON CALLBACK DEFINITIONS
    FReply CreateButtonClicked();
    FReply ApplyFeatureButtonClicked();
    FReply ResetFeaturesButtonClicked();
    FReply RefreshCachingButtonClicked();
    FReply EmptyQButtonClicked();
    FReply LogoutButtonClicked();
    FReply QueueButtonClicked();
    FReply ReturnButtonClicked();
    FReply LoginButtonClicked();

};

class FGUIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void PluginButtonClicked();

private:
    void RegisterMenus();
    void ShowWidget();

    TSharedPtr<class FUICommandList> PluginCommands;
    TSharedPtr<SGUIWidget> GUIWidget;
};
