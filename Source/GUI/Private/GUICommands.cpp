// Copyright Epic Games, Inc. All Rights Reserved.

#include "GUICommands.h"

#define LOCTEXT_NAMESPACE "FGUIModule"

void FGUICommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "GUI", "Open OPUS GUI", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
