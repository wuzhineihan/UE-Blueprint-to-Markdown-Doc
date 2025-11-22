/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/BP2AICommands.cpp


#include "BP2AICommands.h"
#include "Logging/BP2AILog.h"

#define LOCTEXT_NAMESPACE "FBlueprintMarkdownModule"

void FBP2AICommands::RegisterCommands()
{
	/* --- COMMENTED OUT LEGACY COMMANDS ---
	UI_COMMAND(
		GenerateMarkdownCommand, 
		"Generate Markdown", 
		"Generate Markdown from selected Blueprint nodes", 
		EUserInterfaceActionType::Button, 
		FInputChord()
	);
	
	UI_COMMAND(
		GenerateTextMarkdownCommand, 
		"Generate Text-Based Markdown", 
		"Generate Markdown from selected Blueprint nodes using text export", 
		EUserInterfaceActionType::Button, 
		FInputChord()
	);
	
	UI_COMMAND(
		ShowExportDebugCommand, 
		"Show Export Debug", 
		"Show raw text export of selected Blueprint nodes for debugging", 
		EUserInterfaceActionType::Button, 
		FInputChord()
	);
	*/
	
	// --- KEPT: The one remaining command ---
	UI_COMMAND(
		GenerateExecFlowCommand, // New variable name
		"Trace Execution Flow", // Text for NEW menu entry
		"Generate Markdown showing execution flow from selected entry points", // Tooltip for NEW entry
		EUserInterfaceActionType::Button,
		FInputChord()
	);
	
}

#undef LOCTEXT_NAMESPACE