/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Composite.cpp
#include "NodeTraceHandlers_Composite.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "Extractors/BlueprintDataExtractor.h"   // For Tracer->GetDataExtractorRef()
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_Composite::HandleCompositeOutputPinValue(
    TSharedPtr<const FBlueprintNode> Node, // The Composite node
    TSharedPtr<const FBlueprintPin> OutputPinFromComposite,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& OuterGraphNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractorRef,
    TSharedPtr<const FBlueprintNode> CallingNodeForOuterArgs,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMapForOuterArgs,
    bool bSymbolicTrace, // This flag will now more directly control if we do this symbolic representation
    const FString& OuterGraphContext
) {
    check(Node.IsValid() && Node->NodeType == TEXT("Composite"));
    check(OutputPinFromComposite.IsValid() && OutputPinFromComposite->IsOutput());
    check(Tracer);

    // If not doing a symbolic trace for THIS output pin, we might still want the old deep trace behavior.
    // However, for Goal 1, the request is to ALWAYS make this symbolic when a Composite's output is traced.
    // Let's assume for now this handler *always* produces the symbolic output.
    // If conditional deep-tracing is ever needed for Composite outputs, bSymbolicTrace could be used.

    const FString CompositeDisplayName = Node->Name.IsEmpty() ? Node->NodeType : Node->Name.Replace(TEXT("\n"), TEXT(" ")); // Use node name, clean newlines

    UE_LOG(LogDataTracer, Error, TEXT("HandleCompositeOutputPinValue (SYMBOLIC): Tracing output '%s' of Composite '%s'. OuterContext: '%s'"),
        *OutputPinFromComposite->Name, *CompositeDisplayName, *OuterGraphContext);

    // 1. Resolve Input Arguments of THIS Composite Node itself in the OuterGraphContext
    //    This uses MarkdownFormattingUtils::FormatArgumentsForTrace which internally calls RVR for each arg.
    TSet<FString> VisitedPinsForArgs; // Fresh set for formatting arguments
    TSet<FName> ExcludePinsForArgs;   // Typically empty or just "self" if relevant for composite node itself

    // Call FormatArgumentsForTrace to get a string like "Pin1=Val1, Pin2=Val2"
    // The `CurrentNodesMap` for FormatArgumentsForTrace should be OuterGraphNodesMap
    // The `Node` for FormatArgumentsForTrace is the Composite node itself.
    // The `CurrentBlueprintContext` for FormatArgumentsForTrace is OuterGraphContext.
    // `bSymbolicTrace` for arguments should be true if we want symbolic args, false for resolved.
    // Let's use the incoming 'bSymbolicTrace' for the arguments for consistency.
    FString FormattedArgs = MarkdownFormattingUtils::FormatArgumentsForTrace(
        Node,                       // The Composite Node whose inputs we are formatting
        Tracer,
        OuterGraphNodesMap,         // The map where 'Node' (the Composite) exists
        Depth + 1,                  // Depth for resolving arguments
        VisitedPinsForArgs,
        ExcludePinsForArgs,
        Node,                       // The "calling node" for its own inputs
        nullptr,                    // No "outer-outer" map for these argument resolutions
        bSymbolicTrace,             // How to trace the arguments themselves
        OuterGraphContext
    );
    
    FString ArgsParenthesized = FString::Printf(TEXT("(%s)"), *FormattedArgs);
    if (FormattedArgs.IsEmpty())
    {
        ArgsParenthesized = TEXT("()");
    }

    // 2. Construct the symbolic string
    FString SymbolicRepresentation = FString::Printf(TEXT("%s%s.%s"),
        *CompositeDisplayName,
        *ArgsParenthesized,
        *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPinFromComposite->Name))
    );

    // Wrap in ValueFrom to indicate it's not a direct literal
    FString Result = FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(%s)"), *SymbolicRepresentation);

    UE_LOG(LogDataTracer, Error, TEXT("  HandleCompositeOutputPinValue (SYMBOLIC): Composite '%s' output '%s' resolved to: %s"),
        *CompositeDisplayName, *OutputPinFromComposite->Name, *Result);
    return Result;
}
