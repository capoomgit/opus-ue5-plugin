// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SLoginScreen.h"
#include "SCreationScreen.h"
#include "SQueueScreen.h"
#include "EditorNotificationHelper.h"

// Libraries
#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"

class FToolBarBuilder;
class FMenuBuilder;

enum OPUSScreenState 
{
	LOGIN = 0,
	CREATION = 1,
	QUEUE = 2
};

class FOPUSModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();

private:
	// Application
	void RegisterMenus();
	void CreateWindow();

	// Screens
	void ShowScreen(OPUSScreenState screen);
	void ShowLoginScreen();
	void ShowCreationScreen();
	void ShowQueueScreen();

	// Memory Management
	void CleanUp();

	// Register Screens
	void RegisterLoginScreen();
	void RegisterCreationScreen();
	void RegisterQueueScreen();

	//Delegate Events
	void OnLoginSuccessful(FString apiKey);
	void OnLogout();
	void OnCreationScreenEnabled();
	void OnQueueScreenEnabled();
	void OnMainWindowClosed(const TSharedRef<SWindow>& ClosedWindow);

private:
	// Application
	TSharedPtr<class FUICommandList> PluginCommands;

	// Screens
	TSharedPtr<SWindow> MainWindow;
	TSharedPtr<class SLoginScreen> LoginScreen;
	TSharedPtr<class SCreationScreen> CreationScreen;
	TSharedPtr<class SQueueScreen> QueueScreen;
	OPUSScreenState CurrentScreenState;

	// Login
	FString CurrentAPIKey;
	bool IsLoggedIn = false;

	// Editor Notifications
	EditorNotificationHelper NotificationHelper;

};
