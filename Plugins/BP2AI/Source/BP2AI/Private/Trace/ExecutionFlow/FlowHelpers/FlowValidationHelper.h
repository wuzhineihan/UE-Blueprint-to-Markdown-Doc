/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/FlowValidationHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/FMarkdownPathTracer.h" // For EUserGraphType

// Forward declarations
struct FTracingResults;

/**
 * Helper class for validating execution flow processes
 * Handles duplicate detection, phase validation, and result verification
 */
class BP2AI_API FFlowValidationHelper
{
public:
	FFlowValidationHelper();
	~FFlowValidationHelper();

	/**
	 * Duplicate detection report structure
	 */
	struct FDuplicateDetectionReport 
	{
		TMap<FString, TArray<FString>> DefKeyToHints;
		TMap<FString, TArray<FString>> GraphPathToHints;
		TMap<FString, TArray<FString>> HintToDefKeys;
        
		void LogReport() const;
		bool HasDuplicates() const;
		void RecordEntry(const FString& DefKey, const FString& GraphPath, const FString& GraphNameHint);
	};

	/**
	 * Validate discovered graphs for duplicates
	 */
	FDuplicateDetectionReport ValidateGraphDiscovery(
		const TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& DiscoveredGraphs
	) const;

	/**
	 * Validate Phase 1 fixes are working correctly
	 */
	void ValidatePhase1Fixes(const FTracingResults& Results);

private:
	void ValidateInterfaceSignatureExtraction(const FTracingResults& Results) const;
	void ValidateCustomEventsDiscovery(const FTracingResults& Results) const;
};