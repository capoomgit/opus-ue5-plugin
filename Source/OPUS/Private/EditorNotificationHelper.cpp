// Copyright Capoom Inc. All Rights Reserved.


#include "EditorNotificationHelper.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void EditorNotificationHelper::ShowNotificationFail(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;        // Makes the notification automatically disappear after a delay.
    Info.FadeOutDuration = 3.0f;       // Time for the fade out animation.
    Info.ExpireDuration = 5.0f;        // Time until the notification disappears.
    Info.bUseLargeFont = false;        // Adjust font.

    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Fail);
        NotificationItem->ExpireAndFadeout();
    }
}

void EditorNotificationHelper::ShowNotificationSuccess(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 3.0f;
    Info.ExpireDuration = 5.0f;
    Info.bUseLargeFont = false;

    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Success);
        NotificationItem->ExpireAndFadeout();
    }
}

void EditorNotificationHelper::ShowNotificationPending(const FText& NotificationText)
{
    FNotificationInfo Info(NotificationText);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 3.0f;
    Info.ExpireDuration = 5.0f;
    Info.bUseLargeFont = false;

    PendingNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

    if (PendingNotificationItem.IsValid())
    {
        PendingNotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Pending);
        PendingNotificationItem->ExpireAndFadeout();
    }
}