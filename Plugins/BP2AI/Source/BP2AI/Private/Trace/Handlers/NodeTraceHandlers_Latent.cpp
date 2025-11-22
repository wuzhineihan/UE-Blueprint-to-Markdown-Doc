/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Latent.cpp


#include "NodeTraceHandlers_Latent.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h"


//----------------------------------------------------------------//
// Latent Action Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Latent::HandleLatentAction(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
  const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
  bool bSymbolicTrace,
  const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid()); // Node type might vary (Delay, MoveTo, etc.)
    check(Tracer);
    check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleLatentAction: Processing node %s (%s) for output pin %s"), *Node->NodeType, *Node->Guid, *OutputPin->Name);

    // For latent actions, we usually don't trace complex output values.
    // Return symbolic information based on the node type and pin.

    FString ActionName = Node->NodeType; // Start with the node type
    // Refine name based on common nodes
    if (Node->NodeType == TEXT("Delay")) ActionName = TEXT("Delay");
    else if (Node->NodeType == TEXT("MoveComponentTo")) ActionName = TEXT("MoveComponentTo");
    else if (Node->NodeType == TEXT("AIMoveTo")) ActionName = TEXT("AIMoveTo");
    // Could also check FunctionName property if it's a K2Node_LatentGameplayTaskCall

    // Get key parameters if available (e.g., Duration for Delay)
    FString ParamsStr = TEXT("");
    if (ActionName == TEXT("Delay"))
    {
        TSharedPtr<const FBlueprintPin> DurationPin = Node->GetPin(TEXT("Duration"));
        if (DurationPin.IsValid())
        {
            FString DurationVal = Tracer->ResolvePinValueRecursive(DurationPin, AllNodes, Depth + 1, VisitedPins);
            ParamsStr = FString::Printf(TEXT("(Duration=%s)"), *DurationVal);
        }
    }
    // Add similar logic for key params of other latent actions if needed

    // Format output pin symbolically
    FString Result = FMarkdownSpan::Info(TEXT("LatentResult"))
                     + FString::Printf(TEXT("(%s%s).%s"),
                        *FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *ActionName)),
                        *ParamsStr,
                        *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name))
                       );

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleLatentAction: Returning %s"), *Result);
    return Result;
}

FString FNodeTraceHandlers_Latent::HandleDelay(
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
	)
{
	check(Node.IsValid());
	check(Tracer);
	check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleDelay: Processing node %s (%s) for output pin %s"), *Node->NodeType, *Node->Guid, *OutputPin->Name);

	// --- ADDED: Declare and find DurationPin ---
	TSharedPtr<const FBlueprintPin> DurationPin = Node->GetPin(TEXT("Duration"));
	// --- END ADDED ---

	FString DurationValue = DurationPin.IsValid() ? Tracer->ResolvePinValueRecursive(DurationPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("[?Duration?]"));

	FString Result = FMarkdownSpan::Info(TEXT("DelayResult"))
					 + FString::Printf(TEXT("(Duration=%s).%s"),
						*DurationValue,
						*FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name))
					   );

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleDelay: Returning '%s'"), *Result);
	return Result;
}

//----------------------------------------------------------------//