/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/CategoryAnalysisHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/FMarkdownPathTracer.h" // For EUserGraphType

// Forward declarations
class FBlueprintNode;

/**
 * Helper class for analyzing and categorizing discovered graphs
 * Maps graphs to Phase 4 categories (Functions, Macros, Interfaces, etc.)
 */
class BP2AI_API FCategoryAnalysisHelper
{
public:
	FCategoryAnalysisHelper();
	~FCategoryAnalysisHelper();

	/**
	 * Categorize a graph by analyzing its type and content
	 */
	FString CategorizeGraphByType(
		const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes
	);

private:
	// Category analysis methods will be moved here
};