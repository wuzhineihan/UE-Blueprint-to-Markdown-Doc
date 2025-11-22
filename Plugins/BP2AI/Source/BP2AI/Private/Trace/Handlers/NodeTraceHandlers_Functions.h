/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Functions.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for function call and macro node types.
 */
struct BP2AI_API FNodeTraceHandlers_Functions
{
	/** Handles K2Node_CallFunction and K2Node_CallParentFunction. */
	static FString HandleCallFunction(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	);
	static FString HandleFunctionEntryPin(
	   TSharedPtr<const FBlueprintNode> Node, // The FunctionEntry node
	   TSharedPtr<const FBlueprintPin> OutputPinFromEntry, // The specific output pin of FunctionEntry being traced
	   FMarkdownDataTracer* Tracer,
	   const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, // Nodes of the function graph
	   int32 Depth,
	   TSet<FString>& VisitedPins,
	   const FBlueprintDataExtractor& DataExtractor,
	   TSharedPtr<const FBlueprintNode> CallingNode, // Should be nullptr when this handler is invoked
	   const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap, // Should be nullptr
	   bool bSymbolicTrace,
	   const FString& CurrentBlueprintContext // Context of the function graph
   );
	/** Handles K2Node_CallMacroInstance. */
	static FString HandleCallMacro(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
		
		
	);

	/** Handles K2Node_PureAssignmentStatement (used in pure macro graphs). */
	static FString HandlePureAssignment(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	);

private:
	// Internal helper for pure function formatting - NOW includes Kismet specials
	static FString FormatPureFunctionCall_Internal( // <<< CORRECTED NAME
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor, // Removed as not used by this helper
		TSharedPtr<const FBlueprintNode> CurrentCallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* CurrentOuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	);

	static FString FormatPureMacroCall_Internal(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin,
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CurrentCallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* CurrentOuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext 
	);



	
    /* --- ADDED: Specific Kismet Formatter Helpers ---
    static FString FormatKismetMathFunction(const FString& FuncName, TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, int32 Depth, TSet<FString>& VisitedPins);
    static FString FormatKismetStringFunction(const FString& FuncName, TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, int32 Depth, TSet<FString>& VisitedPins);
    static FString FormatGameplayStaticsFunction(const FString& FuncName, TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, int32 Depth, TSet<FString>& VisitedPins);
    static FString FormatKismetSystemFunction(const FString& FuncName, TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, int32 Depth, TSet<FString>& VisitedPins);
    */ 
};