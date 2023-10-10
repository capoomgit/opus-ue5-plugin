// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "GUIStyle.h"

class FGUICommands : public TCommands<FGUICommands>
{
public:

	FGUICommands()
		: TCommands<FGUICommands>(TEXT("GUI"), NSLOCTEXT("Contexts", "GUI", "GUI Plugin"), NAME_None, FGUIStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
