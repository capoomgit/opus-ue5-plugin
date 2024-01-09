// Copyright Capoom Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "OPUSStyle.h"

class FOPUSCommands : public TCommands<FOPUSCommands>
{
public:

	FOPUSCommands()
		: TCommands<FOPUSCommands>(TEXT("OPUS"), NSLOCTEXT("Contexts", "OPUS", "OPUS Plugin"), NAME_None, FOPUSStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
