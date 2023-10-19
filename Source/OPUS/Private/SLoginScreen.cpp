// Fill out your copyright notice in the Description page of Project Settings.

// Source files
#include "SLoginScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"
#include "EditorNotificationHelper.h"

//Libraries 
//#include "Runtime/Online/HTTP/Public/Http.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Http.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoginScreen::Construct(const FArguments& InArgs)
{
    // Find save file 
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));

    // check save file file != null
    if (FPaths::FileExists(SaveFilePath))
    {
        // string extracted from txt file
        // !! TODO  !! encrypt this key
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

        // If the file exists, consider the user as logged in and go directly to main GUI
        // TODO log in as user
        IsLoggedIn = true;
    }

	ChildSlot
	[
        SNew(SOverlay)

            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(-35, -150, 0, 0)  // Move the image up
            [
                SNew(SImage)
                    .Image(FOPUSStyle::Get().GetBrush("OPUS.APILogo"))
            ]

            + SOverlay::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            [
                SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    [
                        SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 50, 0)
                            [
                                SNew(SBorder)
                                    .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                                    .BorderBackgroundColor(FLinearColor::White)
                                    .Padding(FMargin(5.0f))
                                    [
                                        SNew(SHorizontalBox)

                                            + SHorizontalBox::Slot()
                                            .AutoWidth()
                                            [
                                                SNew(STextBlock)
                                                    .Text(LOCTEXT("RapidKeyLabel", "Rapid Key: "))
                                            ]

                                            + SHorizontalBox::Slot()
                                            .AutoWidth()
                                            [
                                                SNew(SBox)
                                                    .WidthOverride(350)
                                                    [
                                                        SAssignNew(KeyField, SEditableText)
                                                            .HintText(LOCTEXT("KeyHint", "                              Please enter your rapid key"))
                                                            .OnTextCommitted(this, &SLoginScreen::OnTextCommittedInKeyField)
                                                    ]
                                            ]
                                    ]
                            ]

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SBox)
                                    .WidthOverride(80)
                                    [
                                        SNew(SButton)
                                            .Text(LOCTEXT("LoginButton", "Login"))
                                            .OnClicked(this, &SLoginScreen::LoginButtonClicked)
                                    ]
                            ]
                    ]
            ]
	];
	
}

void SLoginScreen::OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        LoginButtonClicked(); // You can call your login function directly here
    }
}

FReply SLoginScreen::LoginButtonClicked()
{
    // Get the input from the UsernameField
    if (KeyField.IsValid())
    {
        FString UserKey = KeyField->GetText().ToString();

        // Make API call with Username
        FString Url = "https://opus5.p.rapidapi.com/get_model_names";
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
        HttpRequest->SetURL(Url);
        HttpRequest->SetVerb("GET");
        HttpRequest->SetHeader("Content-Type", "application/json");
        HttpRequest->SetHeader("X-RapidAPI-Key", UserKey);
        HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");

        HttpRequest->OnProcessRequestComplete().BindLambda([this, UserKey](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
            {
                if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
                {
                    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, UserKey]()
                        {
                            FPlatformProcess::Sleep(2);

                            AsyncTask(ENamedThreads::GameThread, [this, UserKey]()
                                {
                                    NotificationHelper.ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));

                                    // TODO dont save to a text file
                                    // Save the API key to a file
                                    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
                                    FFileHelper::SaveStringToFile(UserKey, *SaveFilePath);
                                    FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

                                    IsLoggedIn = true;
                                    // Open creation window
                                    //this->RebuildWidget();
                                });
                        });
                }
                else
                {
                    NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                }
            });

        HttpRequest->ProcessRequest();
    }

    return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

/*
* // Source files
#include "SLoginWindow.h"
#include "SlateOptMacros.h"
#include "OPUS_APIStyle.h"
#include "EditorNotificationHelper.h"
#include "ToolMenus.h"


// Libraries
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"


#define LOCTEXT_NAMESPACE "FOPUS_APIModule"

void SLoginWindow::Construct(const FArguments& InArgs)
{
    // Find save file 
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));

    // check save file file != null
    if (FPaths::FileExists(SaveFilePath))
    {
        // string extracted from txt file
        // !! TODO  !! encrypt this key
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

        // If the file exists, consider the user as logged in and go directly to main GUI
        // TODO log in as user
        IsLoggedIn = true;
    }
    /*
    ChildSlot
       [
            SNew(SOverlay)

                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .Padding(-35, -150, 0, 0)  // Move the image up
                [
                    SNew(SImage)
                        .Image(FOPUS_APIStyle::Get().GetBrush("OPUS.APILogo"))
                ]

                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                [
                    SNew(SVerticalBox)

                        + SVerticalBox::Slot()
                        .VAlign(VAlign_Center)
                        .HAlign(HAlign_Center)
                        [
                            SNew(SHorizontalBox)

                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(0, 0, 50, 0)
                                [
                                    SNew(SBorder)
                                        .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                                        .BorderBackgroundColor(FLinearColor::White)
                                        .Padding(FMargin(5.0f))
                                        [
                                            SNew(SHorizontalBox)

                                                + SHorizontalBox::Slot()
                                                .AutoWidth()
                                                [
                                                    SNew(STextBlock)
                                                        .Text(LOCTEXT("RapidKeyLabel", "Rapid Key: "))
                                                ]

                                                + SHorizontalBox::Slot()
                                                .AutoWidth()
                                                [
                                                    SNew(SBox)
                                                        .WidthOverride(350)
                                                        [
                                                            SAssignNew(KeyField, SEditableText)
                                                                .HintText(LOCTEXT("KeyHint", "                              Please enter your rapid key"))
                                                                .OnTextCommitted(this, &SLoginWindow::OnTextCommittedInKeyField)
                                                        ]
                                                ]
                                        ]
                                ]

                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                [
                                    SNew(SBox)
                                        .WidthOverride(80)
                                        [
                                            SNew(SButton)
                                                .Text(LOCTEXT("LoginButton", "Login"))
                                                .OnClicked(this, &SLoginWindow::LoginButtonClicked)
                                        ]
                                ]
                        ]
                ]
        ];
        
}

// 
void SLoginWindow::OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        LoginButtonClicked(); // You can call your login function directly here
    }
}

FReply SLoginWindow::LoginButtonClicked()
{
    // Get the input from the UsernameField
    if (KeyField.IsValid())
    {
        FString UserKey = KeyField->GetText().ToString();

        // Make API call with Username
        FString Url = "https://opus5.p.rapidapi.com/get_model_names";
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
        HttpRequest->SetURL(Url);
        HttpRequest->SetVerb("GET");
        HttpRequest->SetHeader("Content-Type", "application/json");
        HttpRequest->SetHeader("X-RapidAPI-Key", UserKey);
        HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");

        HttpRequest->OnProcessRequestComplete().BindLambda([this, UserKey](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
            {
                if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
                {
                    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, UserKey]()
                        {
                            FPlatformProcess::Sleep(2);

                            AsyncTask(ENamedThreads::GameThread, [this, UserKey]()
                                {
                                    NotificationHelper.ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));

                                    // TODO dont save to a text file
                                    // Save the API key to a file
                                    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
                                    FFileHelper::SaveStringToFile(UserKey, *SaveFilePath);
                                    FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

                                    IsLoggedIn = true;
                                    // Open creation window
                                    //this->RebuildWidget();
                                });
                        });
                }
                else
                {
                    NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                }
            });

        HttpRequest->ProcessRequest();
    }

    return FReply::Handled();
}

void SLoginWindow::RebuildWidget()
{
    Construct(FArguments());
}
*/
