// Copyright Epic Games, Inc. All Rights Reserved.

#include "OPUS.h"
#include "OPUSStyle.h"
#include "OPUSCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "LoginScreen.h"

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
		FOPUSCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FOPUSModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOPUSModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(OPUSTabName, FOnSpawnTab::CreateRaw(this, &FOPUSModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FOPUSTabTitle", "OPUS"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

}

void FOPUSModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FOPUSStyle::Shutdown();

	FOPUSCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(OPUSTabName);
}

TSharedRef<SDockTab> FOPUSModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	LoginScreen loginScreen;

	return loginScreen.ShowLoginScreen();
}

void FOPUSModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(OPUSTabName);
}

void FOPUSModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FOPUSCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FOPUSCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOPUSModule, OPUS)