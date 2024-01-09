// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "EditorNotificationHelper.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginSuccessful, FString);

class OPUS_API SLoginScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoginScreen)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void RebuildWidget();

	// Delegates
	FOnLoginSuccessful OnLoginSuccessfulDelegate;
	void LogOut();

private:
	void OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod);
	FReply LoginButtonClicked();
	void LogIn(FString key);
	void SaveKeyToFile(FString key);
	void RemoveKeyFile();
	bool CheckStoredAPIKey();

private:
	TSharedPtr<SEditableText> KeyField;
	FString StoredAPIKey;
	EditorNotificationHelper NotificationHelper;

};
