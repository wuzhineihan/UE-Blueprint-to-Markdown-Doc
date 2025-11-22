/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Arrays.cpp


#include "NodeTraceHandlers_Arrays.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownTracerUtils.h"


#include "Internationalization/Regex.h" // For simple var check

//----------------------------------------------------------------//
// Array Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Arrays::HandleMakeArray(
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
    check(Node.IsValid() && Node->NodeType == TEXT("MakeArray"));
    check(Tracer);
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeArray: Processing node %s"), *Node->Guid);

    TArray<FString> ItemStrings;
    // Find input pins named "[0]", "[1]", etc.
    int32 Index = 0;
    while (true)
    {
        FString PinName = FString::Printf(TEXT("[%d]"), Index);
        TSharedPtr<const FBlueprintPin> ItemPin = Node->GetPin(PinName);
        if (!ItemPin.IsValid())
        {
            break; // No more numbered pins
        }
        // Trace each item pin's value recursively
        FString ElementValue = Tracer->ResolvePinValueRecursive(ItemPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace);
        ItemStrings.Add(ElementValue);
        Index++;
    }

    FString Result = FMarkdownSpan::LiteralContainer(TEXT("[")) + FString::Join(ItemStrings, TEXT(", ")) + FMarkdownSpan::LiteralContainer(TEXT("]"));
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeArray: Returning '%s'"), *Result);
    return Result;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Arrays::HandleGetArrayItem(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // This is the output item pin
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
    check(Node.IsValid() && Node->NodeType == TEXT("GetArrayItem"));
    check(Tracer);
    check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleGetArrayItem: Processing node %s for output pin %s"), *Node->Guid, *OutputPin->Name);

    // Find the array input pin (heuristic: first non-hidden input array)
    TSharedPtr<const FBlueprintPin> ArrayPin;
    TSharedPtr<const FBlueprintPin> IndexPin;

    for(const auto& Pair : Node->Pins)
    {
        if(Pair.Value.IsValid() && Pair.Value->IsInput())
        {
            if(Pair.Value->ContainerType == TEXT("Array")) // Simplified check
            {
                ArrayPin = Pair.Value;
            }
            // Common index pin names
            else if (Pair.Value->Name == TEXT("Dimension 1") || Pair.Value->Name == TEXT("Index"))
            {
                 IndexPin = Pair.Value;
            }
        }
    }

    // Ensure required pins were found
    if (!ArrayPin.IsValid()) { UE_LOG(LogDataTracer, Warning, TEXT("  HandleGetArrayItem: Cannot find Array input pin")); return FMarkdownSpan::Error(TEXT("[GetArrayItem Array Input Missing]")); }
    if (!IndexPin.IsValid()) { UE_LOG(LogDataTracer, Warning, TEXT("  HandleGetArrayItem: Cannot find Index input pin")); return FMarkdownSpan::Error(TEXT("[GetArrayItem Index Input Missing]")); }

    // Resolve values for the array and index pins recursively
    FString ArrayStr = Tracer->ResolvePinValueRecursive(ArrayPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace);
    FString IndexStr = Tracer->ResolvePinValueRecursive(IndexPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace);

    // Check if array string is a simple variable or needs wrapping for clarity
    static const FRegexPattern SimpleVarPattern(TEXT("^<span class=\"bp-var\">`[a-zAOL-Z_][a-zA-Z0-9_ ]*`</span>$")); // Corrected regex for variable names
    FRegexMatcher VarMatcher(SimpleVarPattern, ArrayStr);

    FString Result;
    if (VarMatcher.FindNext())
    {
        // If it's a simple variable, use direct array access syntax
        Result = FString::Printf(TEXT("%s%s%s%s"), *ArrayStr, *FMarkdownSpan::Operator(TEXT("[")), *IndexStr, *FMarkdownSpan::Operator(TEXT("]")));
    }
    else
    {
        // If it's a more complex expression, wrap it in parentheses
        Result = FString::Printf(TEXT("(%s)%s%s%s"), *ArrayStr, *FMarkdownSpan::Operator(TEXT("[")), *IndexStr, *FMarkdownSpan::Operator(TEXT("]")));
    }

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleGetArrayItem: Returning '%s'"), *Result);
    return Result;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Arrays::HandleCallArrayFunction(
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
    check(Node.IsValid() && Node->NodeType == TEXT("CallArrayFunction")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCallArrayFunction: Processing node %s for output pin %s"), *Node->Guid, *OutputPin->Name);

    const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
    const FString FuncName = FuncNamePtr ? **FuncNamePtr : TEXT("UnknownArrayOp");

    // Find Target Array pin
    TSharedPtr<const FBlueprintPin> ArrayPin;
    for(const auto& Pair : Node->Pins){ if(Pair.Value.IsValid() && Pair.Value->IsInput() && Pair.Value->ContainerType == TEXT("Array")){ ArrayPin = Pair.Value; break; }}
    FString ArrayStr = ArrayPin.IsValid() ? Tracer->ResolvePinValueRecursive(ArrayPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Array?]"));
    
    FString ArrayStrFmt = ArrayStr;
    static const FRegexPattern SimpleVarPattern(TEXT("^<span class=\"bp-var\">`[a-zA-Z_][a-zA-Z0-9_ ]*`</span>$"));
    FRegexMatcher VarMatcher(SimpleVarPattern, ArrayStr);
    if (!VarMatcher.FindNext()) { ArrayStrFmt = FString::Printf(TEXT("(%s)"), *ArrayStr); }

    // --- Handle common QUERYING functions symbolically ---
    if (OutputPin->Name == TEXT("ReturnValue")) // Most query functions output to ReturnValue
    {
        if (FuncName == TEXT("Length")) { return FString::Printf(TEXT("%s.%s()"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(TEXT("Length"))); }
        if (FuncName == TEXT("IsValidIndex")) {
            // --- FIX: Declare and trace IndexStr here ---
            TSharedPtr<const FBlueprintPin> IndexPin = Node->GetPin(TEXT("Index"));
            FString IndexStr = IndexPin.IsValid() ? Tracer->ResolvePinValueRecursive(IndexPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Index?]"));
            // --- End FIX ---
            return FString::Printf(TEXT("%s.%s(%s)"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(TEXT("IsValidIndex")), *IndexStr);
        }
        if (FuncName == TEXT("Contains")) {
            // --- FIX: Declare and trace ItemStr here ---
            TSharedPtr<const FBlueprintPin> ItemPin = Node->GetPin(TEXT("ItemToFind"));
            FString ItemStr = ItemPin.IsValid() ? Tracer->ResolvePinValueRecursive(ItemPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Item?]"));
             // --- End FIX ---
            return FString::Printf(TEXT("%s.%s(%s)"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(TEXT("Contains")), *ItemStr);
        }
        if (FuncName == TEXT("Find")) {
            // --- FIX: Declare and trace ItemStr here ---
            TSharedPtr<const FBlueprintPin> ItemPin = Node->GetPin(TEXT("ItemToFind"));
            FString ItemStr = ItemPin.IsValid() ? Tracer->ResolvePinValueRecursive(ItemPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Item?]"));
            // --- End FIX ---
            return FString::Printf(TEXT("%s.%s(%s)"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(TEXT("Find")), *ItemStr);
        }
        // Get is handled by GetArrayItem handler usually

        // Fallback for other ReturnValue outputs
        UE_LOG(LogDataTracer, Warning, TEXT("  HandleCallArrayFunction: Using generic ResultOf format for QUERYING function '%s'."), *FuncName);
        TSet<FName> Exclusions; if(ArrayPin.IsValid()) Exclusions.Add(FName(*ArrayPin->Name)); // Exclude Array itself
        FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace);
        return FMarkdownSpan::Info(TEXT("ResultOf")) + FString::Printf(TEXT("(%s.%s(%s))"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName)), *ArgsStr);
    }
    // --- Handle common MUTATING functions symbolically (when tracing output array) ---
    // Note: This assumes the output pin being traced IS the modified array pin
    else if (OutputPin->Name == TEXT("Target Array") || OutputPin == ArrayPin) // Check if it's the main array pin
    {
        // Symbolic representation of the *result* of the mutation
        TSet<FName> Exclusions; if(ArrayPin.IsValid()) Exclusions.Add(FName(*ArrayPin->Name));
        Exclusions.Add(FName(*OutputPin->Name)); // Exclude the output pin itself
        FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace);
        FString FuncNameSpan = FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName));

        // Use more generic "Modified" or "ResultOf" for common mutations
        if (FuncName == TEXT("SetArrayElem")) {
             TSharedPtr<const FBlueprintPin> IndexPin = Node->GetPin(TEXT("Index"));
             TSharedPtr<const FBlueprintPin> ItemPin = Node->GetPin(TEXT("Item"));
            FString IndexStr = IndexPin.IsValid() ? Tracer->ResolvePinValueRecursive(IndexPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Idx?]")); // Pass bSymbolicTrace
            FString ItemStr = ItemPin.IsValid() ? Tracer->ResolvePinValueRecursive(ItemPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace) : FMarkdownSpan::Error(TEXT("[?Item?]"));
            return FMarkdownSpan::Info(FString::Printf(TEXT("ResultOf(%s[%s] = %s)"), *ArrayStrFmt, *IndexStr, *ItemStr));
        } else {
            // Generic representation for Add, Remove, Insert, Clear, Resize etc.
            return FMarkdownSpan::Info(TEXT("Modified")) + FString::Printf(TEXT("(%s.%s(%s))"), *ArrayStrFmt, *FuncNameSpan, *ArgsStr);
        }
    }

    // Fallback for other output pins
    else {
        UE_LOG(LogDataTracer, Warning, TEXT("  HandleCallArrayFunction: Using generic fallback for function '%s' output pin '%s'."), *FuncName, *OutputPin->Name);
        return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(%s.%s.%s)"), *ArrayStrFmt, *FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName)), *FMarkdownSpan::PinName(FString::Printf(TEXT("`%s`"), *OutputPin->Name)));
    }
}
//----------------------------------------------------------------//
