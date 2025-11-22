/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/Formatters_Private.h
#pragma once

#include "CoreMinimal.h"
#include "Containers/UnrealString.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Widgets/SMarkdownOutputWindow.h" // Include to get FCapturedEventData definition
#include "Templates/SharedPointer.h" // Include for TSharedPtr

// Forward Declarations (Keep minimal)
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

// Use a namespace to group these internal static functions
namespace MarkdownNodeFormatters_Private
{
	// --- Common Helper Functions ---
	FString FormatArguments(
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins, 
		bool bSymbolicTraceForData,
		const TSet<FName>& ExcludePinNames = {},
		const FString& CurrentBlueprintContext = TEXT("") // New context parameter
	);

	FString FormatTarget(
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext = TEXT("") // New context parameter
	);
	
	FString FormatGeneric(
        TSharedPtr<const FBlueprintNode> Node
        // No context needed if it truly only uses Node's direct properties for display.
        // However, for consistency within this file where other formatters now take it:
        // const FString& CurrentBlueprintContext = TEXT("") 
        // For now, keeping it simple as it doesn't trace. If it ever needs to, context would be added.
    );

	// --- Specific Node Formatters (Declared Here, Defined in Separate CPPs) ---
	// All these will now take the CurrentBlueprintContext parameter

	// Formatters_Variables.cpp
	FString FormatVariableSet(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));

	// Formatters_Functions.cpp
	FString FormatCallFunction(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
	FString FormatMacroInstance(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
	FString FormatCallParentFunction(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatReturnNode(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatLatentAction(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatPlayMontage(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));

	// Formatters_ControlFlow.cpp
	FString FormatIfThenElse(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
	FString FormatSwitch(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatSequence(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatForEachLoop(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));

	// Formatters_Objects.cpp
    FString FormatSpawnActor(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatAddComponent(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatCreateWidget(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatGenericCreateObject(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatDynamicCast(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));

	// Formatters_Data.cpp
    FString FormatSetFieldsInStruct(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatCallArrayFunction(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatFormatText(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
	FString FormatGetDataTableRow_NodeDescription(TSharedPtr<const FBlueprintNode> Node,FMarkdownDataTracer& DataTracer,const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,TSet<FString>& VisitedDataPins,bool bSymbolicTraceForData,const FString& CurrentBlueprintContext);

	
	// Formatters_Events.cpp
	FString FormatEvent(TSharedPtr<const FBlueprintNode> Node, const TOptional<FCapturedEventData>& CapturedData, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
	FString FormatTimeline(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatDelegateBinding(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatClearDelegate(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatCallDelegate(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));
    FString FormatComposite(TSharedPtr<const FBlueprintNode> Node, FMarkdownDataTracer& DataTracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& VisitedDataPins, bool bSymbolicTraceForData, const FString& CurrentBlueprintContext = TEXT(""));

} // namespace MarkdownNodeFormatters_Private