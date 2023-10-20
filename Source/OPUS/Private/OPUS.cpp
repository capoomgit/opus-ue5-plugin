// Copyright Epic Games, Inc. All Rights Reserved.

#include "OPUS.h"
#include "OPUSStyle.h"
#include "OPUSCommands.h"
#include "ToolMenus.h"
#include "SlateOptMacros.h"
#include "SLoginScreen.h"
#include "SCreationScreen.h"
#include "SQueueScreen.h"

// Libraries
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"

static const FName OPUSTabName("OPUS");

#define LOCTEXT_NAMESPACE "FOPUSModule"

void FOPUSModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FOPUSStyle::Initialize();
	FOPUSStyle::ReloadTextures();

	FOPUSCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FOPUSCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FOPUSModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOPUSModule::RegisterMenus));

}

void FOPUSModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FOPUSStyle::Shutdown();

	FOPUSCommands::Unregister();
}

void FOPUSModule::PluginButtonClicked()
{
	RegisterScreens();

	CreateWindow();
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FOPUSModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FOPUSCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FOPUSCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FOPUSModule::RegisterScreens()
{
	SAssignNew(LoginScreen, SLoginScreen);
	SAssignNew(CreationScreen, SCreationScreen)
		.APIKey(FText::FromString(CurrentAPIKey));
	SAssignNew(QueueScreen, SQueueScreen)
		.APIKey(FText::FromString(CurrentAPIKey));

	LoginScreen->OnLoginSuccessfulDelegate.AddRaw(this, &FOPUSModule::OnLoginSuccessful);
}

void FOPUSModule::CreateWindow()
{
	SAssignNew(MainWindow, SWindow)
		.Title(LOCTEXT("WindowTitle", "OPUS API"))
		.ClientSize(FVector2D(800, 650))
		.IsInitiallyMaximized(false);
	
	FSlateApplication::Get().AddWindow(MainWindow.ToSharedRef());

	// if there is no API key, then show login page
	if (!CheckSavedAPIKey())
	{
		ShowLoginScreen();
	}

	else
	{
		ShowCreationScreen();
	}
}

void FOPUSModule::ShowLoginScreen()
{
	MainWindow->SetContent(LoginScreen.ToSharedRef());
}

void FOPUSModule::ShowCreationScreen() 
{
	MainWindow->SetContent(CreationScreen.ToSharedRef());
}

void FOPUSModule::ShowQueueScreen() 
{
	MainWindow->SetContent(QueueScreen.ToSharedRef());
}

void FOPUSModule::OnLoginSuccessful(FText apiKey) 
{
	CurrentAPIKey = apiKey.ToString();
	ShowCreationScreen();
}

bool FOPUSModule::CheckSavedAPIKey()
{
	// Find save file 
	FString SaveFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("APIKey.txt"));

	// check save file file != null
	if (FPaths::FileExists(SaveFilePath) && CurrentAPIKey.IsEmpty())
	{
		// string extracted from txt file
		// !! TODO  !! encrypt this key
		FFileHelper::LoadFileToString(CurrentAPIKey, *SaveFilePath);

		// If the file exists, consider the user as logged in and go directly to main GUI
		// TODO log in as user
		IsLoggedIn = true;
	}

	return !CurrentAPIKey.IsEmpty();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOPUSModule, OPUS)