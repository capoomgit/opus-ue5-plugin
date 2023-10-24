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
#include "Async/Async.h"

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
	// TODO Clean up memory and stop all background processes
	
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	NotificationHelper.ShowNotificationFail(FText::FromString("Shutting down module"));
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FOPUSStyle::Shutdown();

	FOPUSCommands::Unregister();
}

// This method is called when plugin button is clicked
void FOPUSModule::PluginButtonClicked()
{
	RegisterLoginScreen();
	CreateWindow();
	ShowScreen(CurrentScreenState);
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

// Register screen methods and assign delegates
void FOPUSModule::RegisterLoginScreen()
{
	if (LoginScreen == nullptr)
	{
		SAssignNew(LoginScreen, SLoginScreen);

		// Register delegate events
		LoginScreen->OnLoginSuccessfulDelegate.AddRaw(this, &FOPUSModule::OnLoginSuccessful);
	}
}

void FOPUSModule::RegisterCreationScreen()
{
	if (CreationScreen == nullptr) 
	{
		SAssignNew(CreationScreen, SCreationScreen)
			.APIKey(CurrentAPIKey);

		CreationScreen->OnLogoutDelegate.AddRaw(this, &FOPUSModule::OnLogout);
		CreationScreen->OnQueueScreenEnabledDelegate.AddRaw(this, &FOPUSModule::OnQueueScreenEnabled);
	}
}

void FOPUSModule::RegisterQueueScreen()
{
	if (QueueScreen == nullptr)
	{
		SAssignNew(QueueScreen, SQueueScreen)
			.APIKey(CurrentAPIKey);

		QueueScreen->OnCreationScreenEnabledDelegate.AddRaw(this, &FOPUSModule::OnCreationScreenEnabled);
	}
}

// Create plugin window to populate
void FOPUSModule::CreateWindow()
{
	SAssignNew(MainWindow, SWindow)
		.Title(LOCTEXT("WindowTitle", "OPUS API"))
		.ClientSize(FVector2D(800, 650))
		.IsInitiallyMaximized(false);
	
	FSlateApplication::Get().AddWindow(MainWindow.ToSharedRef());
}

void FOPUSModule::ShowScreen(OPUSScreenState screen)
{
	switch (screen)
	{
	case LOGIN:
		ShowLoginScreen();
		break;

	case CREATION:
		ShowCreationScreen();
		break;

	case QUEUE:
		ShowQueueScreen();
		break;

	default:
		ShowLoginScreen();
		break;
	}
}
void FOPUSModule::ShowLoginScreen()
{
	MainWindow->SetContent(LoginScreen.ToSharedRef());
	CurrentScreenState = LOGIN;
}

void FOPUSModule::ShowCreationScreen() 
{
	MainWindow->SetContent(CreationScreen.ToSharedRef());
	CurrentScreenState = CREATION;
}

void FOPUSModule::ShowQueueScreen() 
{
	MainWindow->SetContent(QueueScreen.ToSharedRef());
	CurrentScreenState = QUEUE;
}

// Delegate Events
void FOPUSModule::OnLoginSuccessful(FString apiKey) 
{
	CurrentAPIKey = apiKey;
	RegisterCreationScreen();
	ShowCreationScreen();
}

void FOPUSModule::OnLogout()
{
	CurrentAPIKey = "";
	QueueScreen->SetAPIKey("");
	LoginScreen->RebuildWidget();
	ShowLoginScreen();
}

void FOPUSModule::OnQueueScreenEnabled() 
{
	RegisterQueueScreen();
	ShowQueueScreen();
}

void FOPUSModule::OnCreationScreenEnabled()
{
	ShowCreationScreen();
}

void FOPUSModule::OnWindowClosed()
{

}

void FOPUSModule::CleanUp()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOPUSModule, OPUS)