/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/Formatter/FMarkdownNodeFormatter.h
#pragma once

#include "CoreMinimal.h"
#include "Containers/UnrealString.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Widgets/SMarkdownOutputWindow.h" // Include to get FCapturedEventData definition
#include "Templates/SharedPointer.h"

// Forward Declarations (Use 'class' for consistency with engine/model types)
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;
// struct FMarkdownSpan; // Include MarkdownFormattingUtils.h where needed instead

/**
 * Utility for formatting FBlueprintNode descriptions for Markdown output,
 * incorporating data tracing results.
 */
struct BP2AI_API FMarkdownNodeFormatter // Defined as struct
{
public:
	/**
	 * Formats a node's description, including traced arguments/targets.
	 * Returns an empty string for nodes that should be visually skipped (Pure, Knot, Comment).
	 * Dispatches to specific internal formatters based on node type.
	 */
	static FString FormatNodeDescription(
			TSharedPtr<const FBlueprintNode> Node,
			const TOptional<FCapturedEventData>& CapturedData,
			FMarkdownDataTracer& DataTracer,
			const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
			TSet<FString>& VisitedDataPins,
			bool bGenerateLinkOnly = false,
			bool bSymbolicTraceForData = false,
			const FString& CurrentBlueprintContext = TEXT("") 
		);

	// NOTE: All other static function declarations removed from public header.
	// They are implementation details declared/defined privately.
};