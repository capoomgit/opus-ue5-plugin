#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ToolMenus.h"
#include "LoginScreen.h"
#include "OPUSStyle.h"
#include "Async/Async.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"

TSharedRef<SDockTab> LoginScreen::ShowLoginScreen() {

	IsLoggedIn = false;

	FText loginText = FText::Format(
		LOCTEXT("Hello World", "Hello World"),
		FText::FromString(TEXT("FOPUSModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("OPUS.cpp"))
	);

    return
        
        SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            // Put your tab content here!
            SNew(SOverlay)
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                        .Text(loginText)
                ]

                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .Padding(-35, -150, 0, 0)  // Move the image up
                [
                   SNew(SImage)
                        .Image(FOPUSStyle::Get().GetBrush("OPUS.APILogo"))
                ]
        ];
        
    /*
        SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SOverlay)

                + SOverlay::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .Padding(-35, -150, 0, 0)  // Move the image up
                //[
                //   SNew(SImage)
                //        .Image(FOPUSStyle::Get().GetBrush("OPUS.APILogo"))
                //]

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
                                                                .OnTextCommitted(this, &LoginScreen::OnTextCommittedInKeyField)
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
                                                .OnClicked(this, &LoginScreen::LoginButtonClicked)
                                        ]
                                ]
                        ]
                ]
        ];
        */
        
}
;
FReply LoginScreen::LoginButtonClicked() { return FReply::Handled(); }

void LoginScreen::OnTextCommittedInKeyField(const FText& Text, ETextCommit::Type CommitMethod) {
    if (CommitMethod == ETextCommit::OnEnter)
    {
        LoginButtonClicked(); // You can call your login function directly here
    }
}
/*
FReply LoginScreen::LoginButtonClicked()
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
                                    ShowNotificationSuccess(LOCTEXT("ValidKeyNotification", "Logged in successfully!"));

                                    // TODO dont save to a text file
                                    // Save the API key to a file
                                    FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));
                                    FFileHelper::SaveStringToFile(UserKey, *SaveFilePath);
                                    FFileHelper::LoadFileToString(StoredAPIKey, *SaveFilePath);

                                    bIsLoggedIn = true;
                                    this->RebuildWidget();
                                });
                        });
                }
                else
                {
                    ShowNotificationFail(LOCTEXT("InvalidKeyNotification", "Key is not recognized!"));
                }
            });

        HttpRequest->ProcessRequest();
    }

    return FReply::Handled();
}
*/