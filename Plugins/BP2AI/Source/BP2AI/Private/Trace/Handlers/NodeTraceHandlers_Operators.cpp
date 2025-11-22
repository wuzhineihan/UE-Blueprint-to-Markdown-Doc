/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Operators.cpp


#include "NodeTraceHandlers_Operators.h"
#include "Trace/MarkdownDataTracer.h" // Include main tracer for helpers
#include "Models/BlueprintNode.h"    // Include node/pin models
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"       // For UE_LOG
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"


//----------------------------------------------------------------//
// Operator Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Operators::HandleOperator(
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
    UE_LOG(LogDataTracer, Error, TEXT("HandleOperator (OperatorHandler) ENTERED: Node=%s (%s), NodeType=%s. Context RECEIVED: '%s'. CallingNode=%s (%s), OuterMap=%p, Symbolic=%d"),
        *Node->Name,
        *Node->Guid.Left(8),
        *Node->NodeType, 
        *CurrentBlueprintContext, // Log received context
        CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL_NODE_NAME"),
        CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID"),
        OuterNodesMap,
        bSymbolicTrace);
    
    check(Node.IsValid() && Tracer && OutputPin.IsValid());
    check(Node->NodeType == TEXT("PromotableOperator") || Node->NodeType == TEXT("CommutativeAssociativeBinaryOperator"));

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleOperator: Processing node %s (%s)"), *Node->NodeType, *Node->Guid);

    const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
    const FString FuncName = FuncNamePtr ? **FuncNamePtr : FString(TEXT("")); // Ensure initialized

    if (FuncName.IsEmpty())
    {
        UE_LOG(LogDataTracer, Warning, TEXT("  HandleOperator: Node %s is missing FunctionName property! Cannot determine operation."), *Node->Guid);
        return FMarkdownSpan::Error(FString::Printf(TEXT("[Op FuncName Missing on %s]"), *Node->NodeType));
    }

    const FString NormalizedFuncName = MarkdownTracerUtils::NormalizeConversionName(FuncName, Tracer->GetTypeConversionMap());
    if (Tracer->GetTypeConversionMap().Contains(NormalizedFuncName))
    {
        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleOperator: Detected as TYPE CONVERSION (%s). Context to pass to FormatConversion: '%s'."), *FuncName, *CurrentBlueprintContext);
        // Pass context to FormatConversion
        return MarkdownFormattingUtils::FormatConversion(Node, Tracer, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    }
    else
    {
        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleOperator: Detected as MATH/LOGIC OP (%s). Context to pass to FormatOperator: '%s'."), *FuncName, *CurrentBlueprintContext);

        UE_LOG(LogDataTracer, Error, TEXT("HandleOperator (OperatorHandler) PRE-CALL FormatOperator: Node=%s (%s). Context about to be passed: '%s'. CallingNode=%s (%s), OuterMap=%p"),
            *Node->Name,
            *Node->Guid.Left(8),
            *CurrentBlueprintContext, // Log context to be passed
            CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL_NODE_NAME"),
            CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID"),
            OuterNodesMap);

        // Pass context to FormatOperator utility
        return MarkdownFormattingUtils::FormatOperator(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    }
}


FString FNodeTraceHandlers_Operators::HandleUnaryOperator(
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
    UE_LOG(LogDataTracer, Error, TEXT("HandleUnaryOperator (OperatorHandler) ENTERED: Node=%s (%s), NodeType=%s. Context RECEIVED: '%s'. CallingNode=%s (%s), OuterMap=%p, Symbolic=%d"),
        *Node->Name,
        *Node->Guid.Left(8),
        *Node->NodeType,
        *CurrentBlueprintContext, // Log received context
        CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL_NODE_NAME"),
        CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID"),
        OuterNodesMap,
        bSymbolicTrace);
    check(Node.IsValid());
    check(Tracer);
    check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleUnaryOperator: Processing node %s (%s)"), *Node->NodeType, *Node->Guid);

    UE_LOG(LogDataTracer, Log, TEXT("  HandleUnaryOperator: Context to pass to FormatUnaryOperator utility: '%s'"), *CurrentBlueprintContext);
    return MarkdownFormattingUtils::FormatUnaryOperator(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
}
