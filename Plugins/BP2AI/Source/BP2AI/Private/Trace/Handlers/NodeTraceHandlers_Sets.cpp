/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Sets.cpp


#include "NodeTraceHandlers_Sets.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h" // Assuming you might want LogDataTracer or a new LogSets
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan

FString FNodeTraceHandlers_Sets::HandleMakeSet(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // The set output pin
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext)
{
    // Essential checks for valid input parameters and node type
    check(Node.IsValid() && Node->NodeType == TEXT("MakeSet")); 
    check(Tracer);
    check(OutputPin.IsValid()); // Ensure the OutputPin pointer itself is valid

    // Corrected Check: The OutputPin for a "Make Set" node should have its ContainerType as "Set".
    // The OutputPin->Category will reflect the type of elements in the set (e.g., "int", "struct").
    check(OutputPin->ContainerType == TEXT("Set")); 

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: Processing node '%s' (GUID: %s) for output pin '%s' (PinCategory: '%s', PinContainerType: '%s'). Context: '%s'"), 
        *Node->Name, *Node->Guid, *OutputPin->Name, *OutputPin->Category, *OutputPin->ContainerType, *CurrentBlueprintContext);

    TArray<FString> ElementStrings;
    int32 ElementIndex = 0;
    while (true)
    {
        FString ElementPinName = FString::Printf(TEXT("[%d]"), ElementIndex);
        TSharedPtr<const FBlueprintPin> ElementPin = Node->GetPin(ElementPinName, TEXT("EGPD_Input"));

        if (!ElementPin.IsValid())
        {
            // If no numbered pins were found (ElementIndex is 0) AND there's more than just the output pin,
            // try to find a single non-standard named input pin as a fallback.
            // This handles a MakeSet with only one element that might not use the "[0]" naming.
            if (ElementIndex == 0 && Node->Pins.Num() > 1) { 
                bool bFoundOtherInput = false;
                for (const auto& Pair : Node->Pins) {
                    // Ensure it's an input, not the output pin itself, and not an exec pin
                    if (Pair.Value.IsValid() && Pair.Value->IsInput() && Pair.Value != OutputPin && !Pair.Value->IsExecution()) {
                        ElementPin = Pair.Value;
                        bFoundOtherInput = true;
                        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: Found non-standard single input pin '%s' for MakeSet."), *ElementPin->Name);
                        break;
                    }
                }
                if (!bFoundOtherInput || !ElementPin.IsValid()) { // If still no valid input pin, break
                    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: No more input pins found (checked standard and single fallback)."));
                    break;
                }
                // If we process this non-standard pin, make sure to break after this iteration
                ElementIndex = -1; // Signal to break after this iteration
            } else { // Broke out of numbered pins or it was an empty MakeSet
                UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: No more numbered input pins found or MakeSet is empty. Elements processed: %d"), ElementStrings.Num());
                break; 
            }
        }

        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: Tracing element pin '%s'."), *ElementPin->Name);
        FString ElementValue = Tracer->ResolvePinValueRecursive(
            ElementPin,
            CurrentNodesMap,
            Depth + 1, 
            VisitedPins,
            CallingNode,
            OuterNodesMap,
            bSymbolicTrace,
            CurrentBlueprintContext
        );
        ElementStrings.Add(ElementValue);

        if (ElementIndex == -1) { // Break if we processed a single non-standard named pin
             UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: Processed single non-standard input pin. Ending element search."));
             break;
        }
        ElementIndex++;
    }

    FString Result = FMarkdownSpan::LiteralContainer(TEXT("{")) + 
                     FString::Join(ElementStrings, TEXT(", ")) + 
                     FMarkdownSpan::LiteralContainer(TEXT("}"));

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeSet: Node %s finished. Returning literal set: %s"), *Node->Guid, *Result);
    return Result;
}