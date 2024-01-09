// Copyright Capoom Inc. All Rights Reserved.

#include "OPUSCommands.h"

#define LOCTEXT_NAMESPACE "FOPUSModule"

void FOPUSCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "OPUS", "Execute OPUS action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
