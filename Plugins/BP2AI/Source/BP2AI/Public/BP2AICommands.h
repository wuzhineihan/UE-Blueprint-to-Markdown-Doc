/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/BP2AICommands.h

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BP2AIStyle.h"

class FBP2AICommands : public TCommands<FBP2AICommands>
{
public:
	FBP2AICommands()
		: TCommands<FBP2AICommands>(
			TEXT("BlueprintMarkdown"),
			NSLOCTEXT("Contexts", "BlueprintMarkdown", "Blueprint Markdown Plugin"),
			NAME_None,
			FBP2AIStyle::GetStyleSetName())
	{}

	// TCommands interface
	virtual void RegisterCommands() override;

public:


	TSharedPtr<FUICommandInfo> GenerateExecFlowCommand;
};