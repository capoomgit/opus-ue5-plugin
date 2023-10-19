// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class OPUS_API EditorNotificationHelper
{
public:
	void ShowNotificationFail(const FText& NotificationText);
	void ShowNotificationSuccess(const FText& NotificationText);
	void ShowNotificationPending(const FText& NotificationText);

private:
	TSharedPtr<SNotificationItem> PendingNotificationItem;
};
