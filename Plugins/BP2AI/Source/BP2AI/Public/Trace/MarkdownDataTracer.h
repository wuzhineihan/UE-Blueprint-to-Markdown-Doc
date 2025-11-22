/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/Trace/MarkdownDataTracer.h
#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Templates/Function.h"
#include "Models/BlueprintPin.h"
#include "Models/BlueprintNode.h"
#include "Trace/FMarkdownPathTracer.h" // For FMarkdownPathTracer::EUserGraphType
#include "Trace/Generation/GenerationShared.h" // For FGenerationSettings

// Forward Declarations
class FMarkdownDataTracer;
// class FMarkdownPathTracer; // Now included above

class BP2AI_API FMarkdownDataTracer
{
public:
	using FNodeTraceHandlerFunc = TFunction<FString(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> PinToTrace,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	)>;

	FMarkdownDataTracer(const FBlueprintDataExtractor& InDataExtractor);
	const FBlueprintDataExtractor& GetDataExtractorRef() const { return DataExtractorRef; }
	
    // Method to access current generation settings
    const FGenerationSettings& GetSettings() const;

	FString TracePinValue(
		TSharedPtr<const FBlueprintPin> PinToResolve,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
		bool bSymbolicTrace = false,
		const FString& CurrentBlueprintContext = TEXT("")
	);

	void ClearCache();

	FString ResolvePinValueRecursive(
		TSharedPtr<const FBlueprintPin> PinToResolve,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		TSharedPtr<const FBlueprintNode> CallingNode = nullptr,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap = nullptr,
		bool bSymbolicTrace = false,
		const FString& CurrentBlueprintContext = TEXT("")
	);

	FString TraceTargetPin(
		TSharedPtr<const FBlueprintPin> TargetPin,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FString& CurrentBlueprintContext
	);

	const TMap<FString, FString>& GetMathOperatorMap() const { return MathOperatorMap; }
	const TMap<FString, FString>& GetTypeConversionMap() const { return TypeConversionMap; }

	const FBlueprintDataExtractor& DataExtractorRef;

	void StartTraceSession(
        // Updated TTuple type
		TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>* OutGraphsToDefineSeparatelyPtr,
		TSet<FString>* ProcessedSeparateGraphPathsPtr,
		bool bInShowTrivialDefaultParams,
        const FGenerationSettings* InSettings = nullptr // Added InSettings parameter
	);
	void EndTraceSession();
	
    // Updated TTuple type
	TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>* CurrentGraphsToDefineSeparatelyPtr = nullptr;
	TSet<FString>* CurrentProcessedSeparateGraphPathsPtr = nullptr;
	
	bool bCurrentShowTrivialDefaultParams = false; 
	
	const TMap<FName, FString>* GetCurrentCallsiteArguments() const { return CurrentCallsiteArgumentsPtr; }
	void SetCurrentCallsiteArguments(const TMap<FName, FString>* InCallsiteArgumentsPtr)
	{
		CurrentCallsiteArgumentsPtr = InCallsiteArgumentsPtr;
	}

	
	private:
	const int32 MaxTraceDepth; 
	
    const FGenerationSettings* CurrentSettings = nullptr; // Member to store settings
	
	const TMap<FName, FString>* CurrentCallsiteArgumentsPtr = nullptr;
	
	TMap<FString, FString> ResolvedPinCache; 
	TMap<FString, FString> MathOperatorMap; 
	TMap<FString, FString> TypeConversionMap; 
	TMap<FString, FNodeTraceHandlerFunc> NodeHandlers; 

	FString TraceSourceNode(
		TSharedPtr<const FBlueprintNode> SourceNode,
		TSharedPtr<const FBlueprintPin> SourcePin,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	);
};