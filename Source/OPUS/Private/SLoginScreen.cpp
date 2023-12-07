// Fill out your copyright notice in the Description page of Project Settings.

// Source files
#include "SLoginScreen.h"
#include "SlateOptMacros.h"
#include "OPUSStyle.h"
#include "EditorNotificationHelper.h"
#include "URLHelper.h"

//Libraries 
#include "Http.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoginScreen::Construct(const FArguments& InArgs)
{
    // If api key exists in file, try to log in
    if (CheckStoredAPIKey()) 
    {
        ChildSlot
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 80, 30, 0)
            [
                SNew(SImage)
                    .Image(FOPUSStyle::Get().GetBrush("OPUS.APILogo"))
            ]

            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 80, 0, 0)
            [
                SNew(STextBlock)
                    .Text(LOCTEXT("Logging in", "Logging in from previous API key"))
            ]
        ];
    }
    // If can't log in or no stored API key exists, show login screen
    else 
    {
        ChildSlot
        [
            SNew(SScrollBox)

            + SScrollBox::Slot()
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Center)
            .Padding(0, 80, 0, 0)  // Move the image up
            [
                SNew(SImage)
                .Image(FOPUSStyle::Get().GetBrush("OPUS.APILogo"))
            ]

            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 60, 0, 0)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
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
                                    .IsPassword(true)
                                    .OnTextCommitted(this, &SLoginScreen::OnTextCommittedInKeyField)
                                ]
                            ]
                        ]
                    ]
                ]
            ]

            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 60, 0, 0)
            [
                SNew(SBox)
                [
                    SNew(SButton)
                    .ContentPadding(FMargin(10))
                    .Text(LOCTEXT("LoginButton", "Login"))
                    .OnClicked(this, &SLoginScreen::LoginButtonClicked)
                ]
            ]
        ];
    }
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
        LogIn(KeyField->GetText().ToString());
    }

    return FReply::Handled();
}

void SLoginScreen::LogIn(FString key)
{
    // Make API call with Username
    FString Url = URLHelper::GetModels;
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("X-RapidAPI-Key", key);
    HttpRequest->SetHeader("X-RapidAPI-Host", "opus5.p.rapidapi.com");

    HttpRequest->OnProcessRequestComplete().BindLambda([this, key](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
            {
                AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, key]()
                    {
                        FPlatformProcess::Sleep(2);

                        AsyncTask(ENamedThreads::GameThread, [this, key]()
                            {
                                SaveKeyToFile(key);
                                NotificationHelper.ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));
                                OnLoginSuccessfulDelegate.Broadcast(key);
                                
                            });
                    });
            }
            else
            {
                NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                RemoveKeyFile();
                RebuildWidget();
            }
        });

    HttpRequest->ProcessRequest();
}

bool SLoginScreen::CheckStoredAPIKey()
{
    // Find save file 
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("OPUSAPIKey.txt"));

    // check save file file != null
    if (FPaths::FileExists(SaveFilePath))
    {
        // string extracted from txt file
        // !! TODO  !! encrypt this key
        FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);
        LogIn(StoredAPIKey);
        return true;
    }

    return false;
}

void SLoginScreen::SaveKeyToFile(FString key) 
{
    // TODO dont save to a text file
    // Save the API key to a file
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("OPUSAPIKey.txt"));
    FFileHelper::SaveStringToFile(key, *SaveFilePath);
    FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);
}

void SLoginScreen::LogOut() 
{
    StoredAPIKey.Empty();
    RemoveKeyFile();
}

void SLoginScreen::RemoveKeyFile() 
{
    // Delete the APIKey.txt file
    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("OPUSAPIKey.txt"));
    if (FPaths::FileExists(SaveFilePath))
    {
        IFileManager::Get().Delete(*SaveFilePath);
    }
}

void SLoginScreen::RebuildWidget()
{
    Construct(FArguments());
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

