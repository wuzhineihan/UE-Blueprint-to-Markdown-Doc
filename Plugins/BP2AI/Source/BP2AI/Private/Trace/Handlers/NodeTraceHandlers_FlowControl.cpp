/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_FlowControl.cpp


#include "NodeTraceHandlers_FlowControl.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h" 


//----------------------------------------------------------------//
// Flow Control Handlers (Value Tracing)
//----------------------------------------------------------------//

FString FNodeTraceHandlers_FlowControl::HandleSelect(
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
	check(Node.IsValid() && Node->NodeType == TEXT("Select"));
	check(Tracer); check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSelect: Processing node %s"), *Node->Guid);

	// K2Node_Select can be indexed by various types (Bool, Enum, Byte, Int).
	// Find the index/condition pin. Common names are "Index" or "Condition".
	TSharedPtr<const FBlueprintPin> IndexPin = Node->GetPin(TEXT("Index"));
	if (!IndexPin.IsValid()) { IndexPin = Node->GetPin(TEXT("Condition")); } // Fallback for boolean select

	FString IndexStr = IndexPin.IsValid() ? Tracer->ResolvePinValueRecursive(IndexPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("[?Index?]"));

	// Collect all non-trivial option input pins (e.g., "Option 0", "Option 1", or named pins like "True", "False").
	TArray<FString> OptionStrings;
	int32 OptionIndex = 0;
	bool FoundNumberedOptions = false;

	// Try numbered options first (common for integer/byte selects)
	while(true) {
		TSharedPtr<const FBlueprintPin> OptionPin = Node->GetPin(FString::Printf(TEXT("Option %d"), OptionIndex));
		if(OptionPin.IsValid() && OptionPin->IsInput()) {
			 FoundNumberedOptions = true;
			 // --- Use namespaced utility function ---
			 if(OptionPin->SourcePinFor.Num() > 0 || !MarkdownTracerUtils::IsTrivialDefault(OptionPin)) {
				 FString OptionValue = Tracer->ResolvePinValueRecursive(OptionPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
				 OptionStrings.Add(FString::Printf(TEXT("%s=%s"), *FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"), *OptionPin->Name)), *OptionValue));
			 }
			 OptionIndex++;
		} else { break; }
	}

	// If no numbered options were found, look for other named input data pins (common for boolean/enum selects)
	if (!FoundNumberedOptions) {
		for(const auto& Pair : Node->Pins) {
			const auto& Pin = Pair.Value;
			// Check if it's an input data pin that isn't the index/condition pin itself
			if(Pin.IsValid() && Pin->IsInput() && !Pin->IsExecution() && (!IndexPin.IsValid() || Pin != IndexPin)) {
				// --- Use namespaced utility function ---
				if(Pin->SourcePinFor.Num() > 0 || !MarkdownTracerUtils::IsTrivialDefault(Pin)) {
					// --- CORRECTED: Use CurrentNodesMap and pass context ---
					FString OptionValue = Tracer->ResolvePinValueRecursive(Pin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
					// --- END CORRECTION ---
					// Use pin name directly for boolean/enum cases (e.g., True=..., False=..., Forward=...)
					OptionStrings.Add(FString::Printf(TEXT("%s=%s"), *FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"), *Pin->Name)), *OptionValue));
				}
			}
		}
	}

	// --- Formatting Explanation ---
	// This handler uses a generic Select(Index=..., Options={...}) format.
	// This differs from the ternary format (Cond ? A : B) used for nodes like
	// "Select String", "Select Float", etc. Those specific nodes are typically
	// K2Node_CallFunction nodes handled by FormatPureFunctionCall_Internal (in NodeTraceHandlers_Functions.cpp),
	// which applies the ternary format when the condition pin 'bPickA' is present.
	// K2Node_Select is more general and can have multiple options based on integer/byte/enum indices,
	// so the generic format is more appropriate here.
	FString Result = FMarkdownSpan::FunctionName(TEXT("Select"))
					 + FString::Printf(TEXT("(%s=%s, Options={%s})"),
						*FMarkdownSpan::ParamName(IndexPin.IsValid() ? FString::Printf(TEXT("%s"), *IndexPin->Name) : TEXT("Index?")),
						*IndexStr,
						*FString::Join(OptionStrings, TEXT(", "))
					  );

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSelect: Returning '%s'"), *Result);
	return Result;
}
// --- END FULL FUNCTION: FNodeTraceHandlers_FlowControl::HandleSelect ---