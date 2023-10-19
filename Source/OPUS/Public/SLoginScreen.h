// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class OPUS_API SLoginScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoginScreen)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	void OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod);
	FReply LoginButtonClicked();
	TSharedPtr<SEditableText> KeyField;
	bool IsLoggedIn = false;
	FString StoredAPIKey;
	EditorNotificationHelper NotificationHelper;

};
