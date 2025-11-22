/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/CategoryAnalysisHelper.cpp

#include "CategoryAnalysisHelper.h"

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Trace/FMarkdownPathTracer.h"

FCategoryAnalysisHelper::FCategoryAnalysisHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FCategoryAnalysisHelper: Initialized"));
}

FCategoryAnalysisHelper::~FCategoryAnalysisHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FCategoryAnalysisHelper: Destroyed"));
}

FString FCategoryAnalysisHelper::CategorizeGraphByType(
    const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes)
{
    FMarkdownPathTracer::EUserGraphType GraphType = GraphInfo.Get<2>();
    const FString& GraphNameHint = GraphInfo.Get<0>();

    UE_LOG(LogPathTracer, Error, TEXT("🔍 CategorizeGraphByType DEBUG: GraphNameHint='%s', GraphType=%d"), 
           *GraphNameHint, static_cast<int32>(GraphType));

    switch (GraphType)
    {
        case FMarkdownPathTracer::EUserGraphType::Interface:
            {
                UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: 'Interfaces'"));
                return TEXT("Interfaces");
            }
            
        case FMarkdownPathTracer::EUserGraphType::Function:
        {
            bool bFunctionIsActuallyPure = true; // Assume pure unless proven otherwise
            TSharedPtr<const FBlueprintNode> FunctionEntryNode = nullptr;
            
            for (const auto& NodePair : GraphNodes)
            {
                if (NodePair.Value.IsValid() && NodePair.Value->NodeType == TEXT("FunctionEntry"))
                {
                    FunctionEntryNode = NodePair.Value;
                    // Check the 'Pure' flag on the FunctionEntry node itself if available (e.g. from UFunction metadata)
                    // FBlueprintNode::IsPure() might be useful if populated for FunctionEntry
                    const FString* PurityProp = FunctionEntryNode->RawProperties.Find(TEXT("bIsPureFunc")); // Common property name
                    if (PurityProp) {
                        bFunctionIsActuallyPure = PurityProp->ToBool();
                    } else {
                        // If property not found, try to infer. If it has exec output, it's not pure.
                        for(const auto& PinPair : FunctionEntryNode->Pins) {
                            if(PinPair.Value.IsValid() && PinPair.Value->IsExecution() && PinPair.Value->Direction == TEXT("EGPD_Output")) {
                                bFunctionIsActuallyPure = false;
                                break;
                            }
                        }
                    }
                    break; 
                }
            }
            
            // If still considered pure, double check by scanning other nodes for any non-pure node or exec pins (more robust)
            if (bFunctionIsActuallyPure)
            {
                for (const auto& NodePair : GraphNodes)
                {
                    if (NodePair.Value.IsValid() && NodePair.Value != FunctionEntryNode) // Exclude the entry node itself
                    {
                        // If any node in the function is not pure (and not a comment/knot/tunnel), the function is not pure.
                        if (!NodePair.Value->IsPure() && 
                            NodePair.Value->NodeType != TEXT("Comment") && 
                            NodePair.Value->NodeType != TEXT("Knot") &&
                            NodePair.Value->NodeType != TEXT("Tunnel") && /* Tunnels are for macros/collapsed */
                            NodePair.Value->NodeType != TEXT("FunctionResult")) /* Result node is fine */
                            {
                                bFunctionIsActuallyPure = false;
                                break;
                            }
                        // Additionally, if any node has an output execution pin (other than FunctionEntry's input exec if any), it implies non-pure.
                        // This is a bit more complex to check reliably without knowing all node types.
                        // Relying on Node->IsPure() for internal nodes is generally better.
                    }
                }
                  // if (!bFunctionIsActuallyPure) break; Break from outer loop if already found to be non-pure
            }
            
            FString Result = bFunctionIsActuallyPure ? TEXT("Pure Functions") : TEXT("Functions");
            UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: '%s'"), *Result);
            return Result;
        }
        
        case FMarkdownPathTracer::EUserGraphType::Macro:
        {
            // For macros, the IsPure flag on the MacroInstance node (caller side) is often a good indicator.
            // However, here we are categorizing the graph *definition*.
            // A macro graph is pure if all its constituent nodes are pure AND it has no execution pins on its Tunnel nodes.
            bool bIsPureBasedOnContent = true;
            bool bHasExecPinsOnTunnels = false;
            for (const auto& NodePair : GraphNodes)
            {
                if (NodePair.Value.IsValid())
                {
                    if (NodePair.Value->NodeType == TEXT("Tunnel"))
                    {
                        for (const auto& PinPair : NodePair.Value->Pins)
                        {
                            if (PinPair.Value.IsValid() && PinPair.Value->IsExecution())
                            {
                                bHasExecPinsOnTunnels = true;
                                break;
                            }
                        }
                        if (bHasExecPinsOnTunnels) {
                             bIsPureBasedOnContent = false; // Exec pins on tunnel means not pure
                             break;
                        }
                    }
                    else if (!NodePair.Value->IsPure() &&
                             NodePair.Value->NodeType != TEXT("Comment") &&
                             NodePair.Value->NodeType != TEXT("Knot"))
                    {
                        bIsPureBasedOnContent = false;
                        break;
                    }
                }
            }
            FString Result = bIsPureBasedOnContent ? TEXT("Pure Macros") : TEXT("Executable Macros");
            UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: '%s'"), *Result);
            return Result;
        }
        
        case FMarkdownPathTracer::EUserGraphType::CustomEventGraph: // Custom Events are inherently executable
        {
            UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: 'Callable Custom Events'"));
            return TEXT("Callable Custom Events");
        }
            
        case FMarkdownPathTracer::EUserGraphType::CollapsedGraph: // Collapsed graphs can be pure or exec
        {
            // Similar to macros, check content and tunnel pins
            bool bIsPure = true;
            bool bHasExecPinsOnTunnels = false;
            for (const auto& NodePair : GraphNodes) {
                if (NodePair.Value.IsValid()) {
                    if (NodePair.Value->NodeType == TEXT("Tunnel")) {
                        for (const auto& PinPair : NodePair.Value->Pins) {
                            if (PinPair.Value.IsValid() && PinPair.Value->IsExecution()) {
                                bHasExecPinsOnTunnels = true; break;
                            }
                        }
                        if (bHasExecPinsOnTunnels) { bIsPure = false; break; }
                    } else if (!NodePair.Value->IsPure() && NodePair.Value->NodeType != TEXT("Comment") && NodePair.Value->NodeType != TEXT("Knot")) {
                        bIsPure = false; break;
                    }
                }
            }
            FString Result = bIsPure ? TEXT("Pure Collapsed Graphs") : TEXT("Collapsed Graphs");
            UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: '%s'"), *Result);
            return Result;
        }
            
        default:
        {
            UE_LOG(LogPathTracer, Error, TEXT("  → CATEGORIZED as: 'Unknown' (DEFAULT CASE)"));
            return TEXT("Unknown");
        }
    }
}