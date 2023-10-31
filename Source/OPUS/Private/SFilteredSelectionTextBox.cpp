// Fill out your copyright notice in the Description page of Project Settings.


#include "SFilteredSelectionTextBox.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFilteredSelectionTextBox::Construct(const FArguments& InArgs)
{
	ChildSlot
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
                            SAssignNew(EditableTextBox, SEditableText)
                                .OnTextChanged(InArgs._OnTextChanged)

                        ]
                ]

                + SVerticalBox::Slot()
                .MaxHeight(111) // Set a maximum height for the suggestions box, adjust as needed
                [
                    SNew(SScrollBox)

                        + SScrollBox::Slot()
                        [
                            SAssignNew(SuggestionsListView, SListView<TSharedPtr<FString>>)
                                .ItemHeight(24)
                                .ListItemsSource(InArgs._ListItemsSource)
                                .OnGenerateRow(this, &SFilteredSelectionTextBox::GenerateSuggestionRow)
                                .Visibility(EVisibility::Collapsed)
                                .SelectionMode(ESelectionMode::Single)
                        ]
                ]
            ]
	];

    KeyboardUserIndex = FSlateApplication::Get().GetUserIndexForKeyboard();

}

FText SFilteredSelectionTextBox::GetText() const
{
    return EditableTextBox->GetText();
}

void SFilteredSelectionTextBox::SetText(const TAttribute<FText>& NewText) 
{
    EditableTextBox->SetText(NewText);
}

void SFilteredSelectionTextBox::RequestListRefresh()
{   
    SuggestionsListView->RequestListRefresh();
}

void SFilteredSelectionTextBox::SetListVisibility(EVisibility NewVisibility)
{
    SuggestionsListView->SetVisibility(NewVisibility);
}

TSharedRef<ITableRow> SFilteredSelectionTextBox::GenerateSuggestionRow(TSharedPtr<FString> Suggestion, const TSharedRef<STableViewBase>& OwnerTable)
{

    TSharedRef EditableTextBoxRef = EditableTextBox.ToSharedRef();
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(SButton)
                .Text(FText::FromString(*Suggestion))
                .OnClicked(this, &SFilteredSelectionTextBox::OnSuggestionRowClicked, Suggestion)
                //.OnClicked_Lambda([this, Suggestion]()
                //    {
                //        if (EditableTextBox.IsValid())
                //        {
                //            // Set the full suggestion (i.e., "SubCategory - Tag") in the search box
                //            EditableTextBox->SetText(FText::FromString(*Suggestion));
                //        }

                //        // Set the selected tag suggestion
                //        OnSuggestionSelected.Broadcast(Suggestion);

                //        // Clear the filtered tag suggestions
                //        FilteredSuggestions.Empty();

                //        // Refresh the tag suggestions list view
                //        SuggestionsListView->RequestListRefresh();
                //        SuggestionsListView->Invalidate(EInvalidateWidget::Layout);
                //        SuggestionsListView->SetVisibility(EVisibility::Collapsed); // This will hide the tag suggestions view.
                //        return FReply::Handled();
                //    })
                //.OnClicked_Raw(this, &SFilteredSelectionTextBox::OnSuggestionRowClicked, Suggestion)
        ];
}

FReply SFilteredSelectionTextBox::OnSuggestionRowClicked(TSharedPtr<FString> Suggestion)
{
    if (EditableTextBox.IsValid())
    {
        // Set the full suggestion (i.e., "SubCategory - Tag") in the search box
        EditableTextBox->SetText(FText::FromString(*Suggestion));
    }

    // Set the selected tag suggestion
    OnSuggestionSelected.Broadcast(Suggestion);

    // Clear the filtered tag suggestions
    FilteredSuggestions.Empty();

    // Refresh the tag suggestions list view
    SuggestionsListView->RequestListRefresh();
    SuggestionsListView->Invalidate(EInvalidateWidget::Layout);
    SuggestionsListView->SetVisibility(EVisibility::Collapsed); // This will hide the tag suggestions view.

    TSharedPtr<SWidget> FocusTarget = GetParentWidget();
    FSlateApplication::Get().SetUserFocus(KeyboardUserIndex, FocusTarget);

    return FReply::Handled();
}


void SFilteredSelectionTextBox::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    TSharedPtr<SWidget> ListWidgetPointer = SuggestionsListView;
    TSharedPtr<SWidget> TextBoxWidgetPointer = EditableTextBox;
    
    bool isTextBoxFocused = TextBoxWidgetPointer->HasKeyboardFocus();
    bool isListFocused = ListWidgetPointer->HasKeyboardFocus() || ListWidgetPointer->HasUserFocusedDescendants(KeyboardUserIndex);
    EVisibility ListVisibility = (isTextBoxFocused || isListFocused) ? EVisibility::Visible : EVisibility::Collapsed;

    SuggestionsListView->SetVisibility(ListVisibility);
        
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
