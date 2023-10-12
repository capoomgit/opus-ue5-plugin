// Copyright Epic Games, Inc. All Rights Reserved.

#include "OPUSCommands.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"

void FOPUSCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "OPUS", "Bring up OPUS window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
