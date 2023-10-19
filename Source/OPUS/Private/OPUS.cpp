// Copyright Epic Games, Inc. All Rights Reserved.

#include "OPUS.h"
#include "OPUSStyle.h"
#include "OPUSCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SlateOptMacros.h"

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
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FOPUSModule::PluginButtonClicked()")),
							FText::FromString(TEXT("OPUS.cpp"))
					   );

	ShowLoginWidget();
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

void FOPUSModule::ShowLoginWidget()
{
	SAssignNew(LoginScreen, SLoginScreen);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("WindowTitle", "OPUS API"))
		.ClientSize(FVector2D(800, 650))
		.IsInitiallyMaximized(false);

	Window->SetContent(LoginScreen.ToSharedRef());
	FSlateApplication::Get().AddWindow(Window);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOPUSModule, OPUS)