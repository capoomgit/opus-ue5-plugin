// Copyright Capoom Inc. All Rights Reserved.

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
                    .ButtonColorAndOpacity(FLinearColor(0.3, 1, 0.3, 1))
                    .OnClicked(this, &SLoginScreen::LoginButtonClicked)
                ]
            ]
            + SScrollBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .Padding(0, 10, 0, 0)
            [
                SNew(SBox)
                    [
                        SNew(SButton)
                            .ContentPadding(FMargin(10))
                            .Text(LOCTEXT("Rapid API page", "Rapid API"))
                            .OnClicked(this, &SLoginScreen::RapidAPIButtonClicked)
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

FReply SLoginScreen::RapidAPIButtonClicked()
{
    FPlatformProcess::LaunchURL(TEXT("https://rapidapi.com/genel-gi78OM1rB/api/opus5"), NULL, NULL);
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
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    ModelOptions.Empty(); // Clear previous options

                    /*
                    // Parse Structures
                    const TArray<TSharedPtr<FJsonValue>>* StructuresArray;
                    if (JsonObject->TryGetArrayField("Structures", StructuresArray))
                    {
                        for (const auto& Value : *StructuresArray)
                        {
                            if (Value.IsValid() && Value->Type == EJson::String)
                            {
                                TSharedPtr<FString> structure = MakeShared<FString>(Value->AsString());
                                ModelOptions.Add(structure);
                            }
                        }
                    }
                    */

                    // Parse Components
                    const TArray<TSharedPtr<FJsonValue>>* ComponentsArray;
                    if (JsonObject->TryGetArrayField("Components", ComponentsArray))
                    {
                        for (const auto& Value : *ComponentsArray)
                        {
                            if (Value.IsValid() && Value->Type == EJson::String)
                            {
                                ModelOptions.Add(MakeShared<FString>(Value->AsString()));
                            }
                        }
                    }

                    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, key]()
                        {
                            FPlatformProcess::Sleep(2);

                            AsyncTask(ENamedThreads::GameThread, [this, key]()
                                {
                                    SaveKeyToFile(key);
                                    NotificationHelper.ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));
                                    //TODO dont send a new request, transfer model names to creation screen
                                    OnLoginSuccessfulDelegate.Broadcast(key, ModelOptions);

                                });
                        });
                }
                else
                {
                    NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                    ShowLoginFailedWindow();
                    RemoveKeyFile();
                    RebuildWidget();
                    UE_LOG(LogTemp, Error, TEXT("Failed to parse the JSON response."));
                }
            }
            else
            {
                NotificationHelper.ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                ShowLoginFailedWindow();
                RemoveKeyFile();
                RebuildWidget();
                UE_LOG(LogTemp, Error, TEXT("API request was not successful. Response code: %d, Error: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
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

FReply SLoginScreen::ShowLoginFailedWindow()
{
    TSharedPtr<SWindow> WarningWindowPtr; // Declare a shared pointer
    TSharedRef<SWindow> WarningWindow = SNew(SWindow)
        .Title(LOCTEXT("LoginFailed", "Login Failed"))
        .ClientSize(FVector2D(600, 150))
        .IsInitiallyMaximized(false);

    WarningWindowPtr = WarningWindow; // Store the window in the shared pointer

    FSlateApplication::Get().AddWindowAsNativeChild(WarningWindow, FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());

    WarningWindow->SetContent(
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(15)
        [
            SNew(STextBlock)
                .Text(FText::FromString("The key you entered is incorrect! If this is a vadil Rapid API key,\n make sure you are subscribed to OPUS"))
                .Justification(ETextJustify::Center)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        .Padding(15)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(15)
            [
                SNew(SButton)
                .Text(LOCTEXT("RetryButton", "Retry"))
                
                .OnClicked_Lambda([this, WarningWindowPtr]()
                    {
                        if (WarningWindowPtr.IsValid())
                        {
                            WarningWindowPtr->RequestDestroyWindow();
                        }
                        return FReply::Handled();
                    })
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(15)
            [
                SNew(SButton)
                    .Text(LOCTEXT("SubscribeButton", "Subscribe"))
                    .OnClicked_Lambda([this, WarningWindowPtr]()
                        {
                            if (WarningWindowPtr.IsValid())
                            {
                                FPlatformProcess::LaunchURL(TEXT("https://rapidapi.com/genel-gi78OM1rB/api/opus5/pricing"), NULL, NULL);
                                WarningWindowPtr->RequestDestroyWindow();
                            }
                            return FReply::Handled();
                        })
            ]
        ]);
    return FReply::Handled();
}

void SLoginScreen::RebuildWidget()
{
    Construct(FArguments());
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

