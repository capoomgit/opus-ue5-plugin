// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SLoginScreen.h"
#include "SCreationScreen.h"
#include "SQueueScreen.h"

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

	void RegisterMenus();
	void RegisterScreens();
	void CreateWindow();
	void ShowLoginScreen();
	void ShowCreationScreen();
	void ShowQueueScreen();
	bool CheckSavedAPIKey();


	//Events
	void OnLoginSuccessful(FText apiKey);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	// Screens
	TSharedPtr<SWindow> MainWindow;
	TSharedPtr<class SLoginScreen> LoginScreen;
	TSharedPtr<class SCreationScreen> CreationScreen;
	TSharedPtr<class SQueueScreen> QueueScreen;

	FString CurrentAPIKey;
	bool IsLoggedIn = false;

	
};
