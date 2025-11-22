/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Delegates.cpp


#include "NodeTraceHandlers_Delegates.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "Trace/Utils/MarkdownTracerUtils.h"  
#include "Trace/Utils/MarkdownTracerUtils.h"


//----------------------------------------------------------------//
// Delegate Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Delegates::HandleDelegate(
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
	const FString& CurrentBlueprintContext)
{
    check(Node.IsValid() && Node->NodeType == TEXT("CreateDelegate"));
    check(Tracer);
    check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCreateDelegate: Processing node %s"), *Node->Guid);

    // Find input pins for Object and FunctionName
    TSharedPtr<const FBlueprintPin> ObjectPin = Node->GetPin(TEXT("Object"));
    TSharedPtr<const FBlueprintPin> FuncNamePin = Node->GetPin(TEXT("FunctionName"));

    // Get the literal function name from the node properties if the pin isn't linked
    FString FunctionNameStr;
    if (FuncNamePin.IsValid() && FuncNamePin->SourcePinFor.Num() > 0)
    {
        FunctionNameStr = Tracer->ResolvePinValueRecursive(FuncNamePin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
    }
    else
    {
        const FString* FuncNameProp = Node->RawProperties.Find(TEXT("SelectedFunctionName"));
        FunctionNameStr = FMarkdownSpan::LiteralName(FString::Printf(TEXT("%s"), FuncNameProp ? **FuncNameProp : TEXT("?Func?")));
    }

    // Trace the target object pin
    FString TargetObjectStr = Tracer->ResolvePinValueRecursive(ObjectPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);

    FString Result = FMarkdownSpan::Keyword(TEXT("Delegate")) + FString::Printf(TEXT("(%s %s %s)"),
        *FunctionNameStr,
        *FMarkdownSpan::Keyword(TEXT("on")),
        *TargetObjectStr
    );

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCreateDelegate: Returning '%s'"), *Result);
    return Result;
}


FString FNodeTraceHandlers_Delegates::HandleCreateDelegate(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, // Added parameter
    TSharedPtr<const FBlueprintNode> CallingNode,   // Added parameter
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap // Added parameter
    , bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("CreateDelegate"));
    check(Tracer);
    check(OutputPin.IsValid()); // This should be the "ReturnValue" delegate pin
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCreateDelegate: Processing node %s"), *Node->Guid);

    // Find input pins for Object and FunctionName
    TSharedPtr<const FBlueprintPin> ObjectPin = Node->GetPin(TEXT("Object")); // Input pin specifying the target object
    TSharedPtr<const FBlueprintPin> FuncNameInputPin = Node->GetPin(TEXT("FunctionName")); // Input pin for the function name (if dynamic)

    FString FunctionNameStr;
    // If FunctionName pin is linked, trace its value
    if (FuncNameInputPin.IsValid() && FuncNameInputPin->SourcePinFor.Num() > 0)
    {
        // Pass the context when resolving
        FunctionNameStr = Tracer->ResolvePinValueRecursive(FuncNameInputPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
    }
    else
    {
        // Otherwise, get the literal function name stored on the node itself
        const FString* FuncNameProp = Node->RawProperties.Find(TEXT("SelectedFunctionName")); // Stored by Factory
        FunctionNameStr = FuncNameProp ? FMarkdownSpan::LiteralName(FString::Printf(TEXT("%s"), **FuncNameProp)) : FMarkdownSpan::Error(TEXT("?Func?"));
    }

    // Trace the target object pin using the current context
    FString TargetStr = ObjectPin.IsValid() ? Tracer->ResolvePinValueRecursive(ObjectPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Variable(TEXT("self"));

    FString Result = FMarkdownSpan::Keyword(TEXT("Delegate")) + FString::Printf(TEXT("(%s %s %s)"),
        *FunctionNameStr,
        *FMarkdownSpan::Keyword(TEXT("on")),
        *TargetStr
    );

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCreateDelegate: Returning '%s'"), *Result);
    return Result;
}