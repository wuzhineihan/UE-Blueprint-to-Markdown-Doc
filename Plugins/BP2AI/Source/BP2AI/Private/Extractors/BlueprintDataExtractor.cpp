/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Extractors/BlueprintDataExtractor.cpp


#include "Extractors/BlueprintDataExtractor.h"
#include "Logging/BP2AILog.h"
#include "Models/BlueprintNodeFactory.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MacroInstance.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditorModule.h"
#include "EdGraphSchema_K2.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Logging/BP2AILog.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/TopLevelAssetPath.h"
#include "EdGraph/EdGraphSchema.h"
#include "Editor.h"

FBlueprintDataExtractor::FBlueprintDataExtractor()
{
}

FBlueprintDataExtractor::~FBlueprintDataExtractor()
{
}

TMap<FString, TSharedPtr<FBlueprintNode>> FBlueprintDataExtractor::ExtractFromSelectedNodes(
    const TArray<UEdGraphNode*>& SelectedNodes,
    bool bIncludeNestedFunctions) const
{
    TMap<FString, TSharedPtr<FBlueprintNode>> BlueprintNodes;

    if (SelectedNodes.Num() == 0)
    {
        return BlueprintNodes;
    }

    TArray<UEdGraphNode*> NodesToProcess = SelectedNodes;

    if (bIncludeNestedFunctions)
    {
        UE_LOG(LogExtractor, Warning, TEXT("FBlueprintDataExtractor: Nested function expansion is not yet implemented."));
    }

    UE_LOG(LogExtractor, Log, TEXT("FBlueprintDataExtractor: Starting node data extraction..."));
    for (UEdGraphNode* Node : NodesToProcess)
    {
        if (!Node)
        {
            continue;
        }

        TSharedPtr<FBlueprintNode> ExtractedNode = ExtractNodeData(Node);

        if (ExtractedNode.IsValid()) {
            if (!ExtractedNode->Guid.IsEmpty()) {
                BlueprintNodes.Add(ExtractedNode->Guid, ExtractedNode);
            } else {
                UE_LOG(LogExtractor, Error, TEXT("FBlueprintDataExtractor: Extracted node is missing GUID! Node Title: %s"), 
                       Node->GetNodeTitle(ENodeTitleType::ListView).IsEmpty() ? TEXT("[NoTitle]") : *Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
            }
        }
    }
    UE_LOG(LogExtractor, Log, TEXT("FBlueprintDataExtractor: Finished node data extraction. %d nodes processed."), BlueprintNodes.Num());

    FinalizeExtractedNodes(BlueprintNodes);

    UE_LOG(LogExtractor, Error, TEXT("DEBUG Post-ResolveLinks: Inspecting SourcePinFor for GetDataTableRow input pins:"));
    for (const auto& NodePair : BlueprintNodes)
    {
        const TSharedPtr<FBlueprintNode>& Node = NodePair.Value;
        if (Node.IsValid() && Node->NodeType == TEXT("GetDataTableRow"))
        {
            UE_LOG(LogExtractor, Error, TEXT("  Node: %s (GUID: %s)"), *Node->Name, *Node->Guid.Left(8));
            TSharedPtr<FBlueprintPin> dtPin = Node->GetPin(TEXT("DataTable"), TEXT("EGPD_Input"));
            if (dtPin.IsValid())
            {
                UE_LOG(LogExtractor, Error, TEXT("    Pin: '%s' (ID:%s), SourcePinFor.Num: %d, DefaultObject: '%s'"),
                    *dtPin->Name, *dtPin->Id.Left(8), dtPin->SourcePinFor.Num(), *dtPin->DefaultObject);
                for (int32 i = 0; i < dtPin->SourcePinFor.Num(); ++i) {
                    if (dtPin->SourcePinFor[i].IsValid()) {
                        UE_LOG(LogExtractor, Error, TEXT("      -> Linked from Node GUID: %s, Pin Name: '%s' (ID:%s)"),
                            *dtPin->SourcePinFor[i]->NodeGuid.Left(8), *dtPin->SourcePinFor[i]->Name, *dtPin->SourcePinFor[i]->Id.Left(8));
                    } else {
                        UE_LOG(LogExtractor, Error, TEXT("      -> SourcePinFor[%d] is INVALID"), i);
                    }
                }
            }
            TSharedPtr<FBlueprintPin> rnPin = Node->GetPin(TEXT("RowName"), TEXT("EGPD_Input"));
            if (rnPin.IsValid())
            {
                UE_LOG(LogExtractor, Error, TEXT("    Pin: '%s' (ID:%s), SourcePinFor.Num: %d, DefaultValue: '%s'"),
                    *rnPin->Name, *rnPin->Id.Left(8), rnPin->SourcePinFor.Num(), *rnPin->DefaultValue);
                for (int32 i = 0; i < rnPin->SourcePinFor.Num(); ++i) {
                    if (rnPin->SourcePinFor[i].IsValid()) {
                        UE_LOG(LogExtractor, Error, TEXT("      -> Linked from Node GUID: %s, Pin Name: '%s' (ID:%s)"),
                            *rnPin->SourcePinFor[i]->NodeGuid.Left(8), *rnPin->SourcePinFor[i]->Name, *rnPin->SourcePinFor[i]->Id.Left(8));
                    } else {
                        UE_LOG(LogExtractor, Error, TEXT("      -> SourcePinFor[%d] is INVALID"), i);
                    }
                }
            }
        }
    }

    return BlueprintNodes;
}

// =============================================================================
// REFACTORED ExtractNodesFromGraph - Main orchestrator
// =============================================================================

bool FBlueprintDataExtractor::ExtractNodesFromGraph(const FString& GraphPath, TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const
{
    OutNodes.Empty();

    UE_LOG(LogExtractor, Error, TEXT("ExtractNodesFromGraph: ENTRY - Attempting to extract from GraphPath: '%s'"), *GraphPath);

    // Phase 1: Resolve the target graph
    UEdGraph* TargetGraph = ResolveTargetGraph(GraphPath);
    if (!TargetGraph)
    {
        UE_LOG(LogExtractor, Error, TEXT("ExtractNodesFromGraph: CRITICAL FAILURE - Could not resolve UEdGraph for path: '%s'"), *GraphPath);
        return false;
    }

    UE_LOG(LogExtractor, Error, TEXT("ExtractNodesFromGraph: Graph resolved for Path '%s'. Graph Name: '%s', PathName: '%s'. Contains %d nodes."), 
           *GraphPath, *TargetGraph->GetName(), *TargetGraph->GetPathName(), TargetGraph->Nodes.Num());

    // Phase 2: Handle empty graphs (diagnostics + interventions)
    if (TargetGraph->Nodes.Num() == 0)
    {
        HandleEmptyGraph(TargetGraph, GraphPath);
        
        if (TargetGraph->Nodes.Num() == 0)
        {
            UE_LOG(LogExtractor, Log, TEXT("ExtractNodesFromGraph: Graph '%s' remains empty after interventions. Returning true with empty OutNodes."), *TargetGraph->GetName());
            return true; // Empty but valid
        }
        
        UE_LOG(LogExtractor, Error, TEXT("ExtractNodesFromGraph: Graph '%s' NOW HAS %d NODES after interventions! Proceeding to process them."), 
               *TargetGraph->GetName(), TargetGraph->Nodes.Num());
    }

    // Phase 3: Process the graph nodes
    bool bSuccess = ProcessGraphNodes(TargetGraph, OutNodes);

    UE_LOG(LogExtractor, Error, TEXT("ExtractNodesFromGraph: EXIT - Finished extraction for graph '%s' (Path: '%s'). Final OutNodes count: %d. Success: %s"),
           *TargetGraph->GetName(), *GraphPath, OutNodes.Num(), bSuccess ? TEXT("true") : TEXT("false"));

    return bSuccess;
}

// =============================================================================
// Graph Resolution Helpers
// =============================================================================

UEdGraph* FBlueprintDataExtractor::ResolveTargetGraph(const FString& GraphPath) const
{
    // Try direct lookup first
    UEdGraph* TargetGraph = TryFindGraph(GraphPath);
    if (TargetGraph)
    {
        return TargetGraph;
    }

    // Try loading from package
    return LoadGraphFromPackage(GraphPath);
}

UEdGraph* FBlueprintDataExtractor::TryFindGraph(const FString& GraphPath) const
{
    return FindObject<UEdGraph>(nullptr, *GraphPath);
}

UEdGraph* FBlueprintDataExtractor::LoadGraphFromPackage(const FString& GraphPath) const
{
    UE_LOG(LogExtractor, Warning, TEXT("ResolveTargetGraph: Direct lookup failed for '%s'. Attempting package load..."), *GraphPath);

    FString PackageNameStr = FPackageName::ObjectPathToPackageName(GraphPath);
    if (PackageNameStr.IsEmpty())
    {
        UE_LOG(LogExtractor, Warning, TEXT("ResolveTargetGraph: Could not determine package name from graph path: '%s'"), *GraphPath);
        return nullptr;
    }

    UPackage* Package = LoadPackage(nullptr, *PackageNameStr, LOAD_None);
    if (!Package)
    {
        UE_LOG(LogExtractor, Warning, TEXT("ResolveTargetGraph: Could not load package '%s' for graph path: '%s'"), *PackageNameStr, *GraphPath);
        return nullptr;
    }

    Package->FullyLoad();
    UEdGraph* TargetGraph = FindObject<UEdGraph>(nullptr, *GraphPath);
    
    UE_LOG(LogExtractor, Log, TEXT("ResolveTargetGraph: Loaded package '%s' to find graph '%s'. Success: %s"), 
           *PackageNameStr, *GraphPath, TargetGraph ? TEXT("Yes") : TEXT("No"));

    return TargetGraph;
}

// =============================================================================
// Empty Graph Handling
// =============================================================================

void FBlueprintDataExtractor::HandleEmptyGraph(UEdGraph* Graph, const FString& GraphPath) const
{
    UBlueprint* OwningBP = FindOwningBlueprint(Graph, GraphPath);
    
    // Perform diagnostics
    DiagnoseEmptyGraph(Graph, GraphPath, OwningBP);
    
    // Attempt interventions if this is a problematic case
    if (IsProblematicMainEventGraph(Graph, OwningBP))
    {
        UE_LOG(LogExtractor, Error, TEXT("HandleEmptyGraph: Identified problematic case. Attempting interventions..."));
        bool bInterventionSuccess = AttemptGraphInterventions(Graph, OwningBP);
        UE_LOG(LogExtractor, Error, TEXT("HandleEmptyGraph: Interventions %s. Final node count: %d"), 
               bInterventionSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), Graph->Nodes.Num());
    }
}

void FBlueprintDataExtractor::DiagnoseEmptyGraph(UEdGraph* Graph, const FString& GraphPath, UBlueprint* OwningBP) const
{
    UE_LOG(LogExtractor, Error, TEXT("DiagnoseEmptyGraph: Graph '%s' (Path: '%s') reported 0 UEdGraphNodes. Starting detailed investigation:"), 
           *Graph->GetName(), *GraphPath);

    LogDiagnosticInfo(Graph, OwningBP, GraphPath);
}

void FBlueprintDataExtractor::LogDiagnosticInfo(UEdGraph* Graph, UBlueprint* OwningBP, const FString& GraphPath) const
{
    // Log graph info
    FString GraphFlagsHex = FString::Printf(TEXT("0x%08X"), Graph->GetFlags());
    UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Graph Class: '%s', Schema: '%s', ObjectFlags: %s"), 
           Graph->GetClass() ? *Graph->GetClass()->GetName() : TEXT("NULL_CLASS"),
           Graph->Schema ? *Graph->Schema->GetClass()->GetName() : TEXT("NULL_SCHEMA"),
           *GraphFlagsHex);

    UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Graph Outer Class: '%s', Outer Path: '%s'"),
           Graph->GetOuter() ? *Graph->GetOuter()->GetClass()->GetName() : TEXT("NULL_OUTER_CLASS"),
           Graph->GetOuter() ? *Graph->GetOuter()->GetPathName() : TEXT("NULL_OUTER_PATH"));

    // Log blueprint info if available
    if (OwningBP)
    {
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Owning Blueprint: '%s' (Class: '%s')"), 
               *OwningBP->GetPathName(), *OwningBP->GetClass()->GetName());
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Blueprint ParentClass: '%s'"), 
               OwningBP->ParentClass ? *OwningBP->ParentClass->GetPathName() : TEXT("NULL_PARENT_CLASS"));

        bool bIsParentNative = OwningBP->ParentClass && OwningBP->ParentClass->GetClassPathName().ToString().StartsWith(TEXT("/Script/"));
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Is ParentClass Native? %s"), bIsParentNative ? TEXT("Yes") : TEXT("No"));

        // Log blueprint status
        const UEnum* BPTypeEnum = StaticEnum<EBlueprintType>();
        FString BPTypeStr = BPTypeEnum ? BPTypeEnum->GetNameStringByValue(static_cast<int64>(OwningBP->BlueprintType)) : 
                           FString::Printf(TEXT("Unknown (%d)"), static_cast<int32>(OwningBP->BlueprintType));
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Blueprint Type: %s"), *BPTypeStr);

        const UEnum* BPStatusEnum = StaticEnum<EBlueprintStatus>();
        FString BPStatusStr = BPStatusEnum ? BPStatusEnum->GetNameStringByValue(static_cast<int64>(OwningBP->Status)) : 
                              FString::Printf(TEXT("Unknown (%d)"), static_cast<int32>(OwningBP->Status));
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Blueprint Status: %s"), *BPStatusStr);

        // Log UbergraphPages
        if (!OwningBP->UbergraphPages.IsEmpty()) {
            for (int32 i = 0; i < OwningBP->UbergraphPages.Num(); ++i) {
                UEdGraph* UberPage = OwningBP->UbergraphPages[i];
                if (UberPage) {
                    UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: UbergraphPage[%d]: Name='%s', Nodes.Num()=%d"),
                           i, *UberPage->GetName(), UberPage->Nodes.Num());
                }
            }
        } else {
            UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: UbergraphPages array is EMPTY."));
        }
    }
    else
    {
        UE_LOG(LogExtractor, Error, TEXT("  DiagnosticInfo: Could not determine Owning Blueprint for graph '%s'"), *Graph->GetName());
    }

    // Log editor context
    if (GEditor && GEditor->GetEditorWorldContext().World()) {
        UE_LOG(LogExtractor, Warning, TEXT("  DiagnosticInfo: GEditor World is valid: %s"), 
               *GEditor->GetEditorWorldContext().World()->GetPathName());
    } else {
        UE_LOG(LogExtractor, Warning, TEXT("  DiagnosticInfo: GEditor World is %s"), 
               GEditor ? TEXT("NULL") : TEXT("GEditor is NULL"));
    }
}

bool FBlueprintDataExtractor::AttemptGraphInterventions(UEdGraph* Graph, UBlueprint* OwningBP) const
{
    if (!OwningBP)
    {
        UE_LOG(LogExtractor, Warning, TEXT("AttemptGraphInterventions: No owning blueprint found, cannot perform interventions"));
        return false;
    }

    bool bSuccess = false;
    int32 InitialNodeCount = Graph->Nodes.Num();

    // Intervention 1: Re-FullyLoad the Blueprint package
    UPackage* BPPackage = OwningBP->GetOutermost();
    if (BPPackage)
    {
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 1: Attempting BPPackage->FullyLoad() for '%s'"), *BPPackage->GetName());
        BPPackage->FullyLoad();
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 1: Graph nodes after FullyLoad: %d"), Graph->Nodes.Num());
        
        if (Graph->Nodes.Num() > InitialNodeCount)
        {
            bSuccess = true;
        }
    }

    // Intervention 2: ConditionalPostLoad on Blueprint
    if (Graph->Nodes.Num() == 0)
    {
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 2: Attempting OwningBlueprint->ConditionalPostLoad()"));
        OwningBP->ConditionalPostLoad();
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 2: Graph nodes after Blueprint PostLoad: %d"), Graph->Nodes.Num());
        
        if (Graph->Nodes.Num() > InitialNodeCount)
        {
            bSuccess = true;
        }
    }

    // Intervention 3: ConditionalPostLoad on Graph
    if (Graph->Nodes.Num() == 0)
    {
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 3: Attempting Graph->ConditionalPostLoad()"));
        Graph->ConditionalPostLoad();
        UE_LOG(LogExtractor, Error, TEXT("  Intervention 3: Graph nodes after Graph PostLoad: %d"), Graph->Nodes.Num());
        
        if (Graph->Nodes.Num() > InitialNodeCount)
        {
            bSuccess = true;
        }
    }

    return bSuccess;
}

UBlueprint* FBlueprintDataExtractor::FindOwningBlueprint(UEdGraph* Graph, const FString& GraphPath) const
{
    if (!Graph)
    {
        return nullptr;
    }

    // Try finding by iterating through outers
    UBlueprint* OwningBP = nullptr;
    UObject* CurrentOuter = Graph->GetOuter();
    while (CurrentOuter && !OwningBP)
    {
        OwningBP = Cast<UBlueprint>(CurrentOuter);
        if (!OwningBP)
        {
            CurrentOuter = CurrentOuter->GetOuter();
        }
    }

    // If not found, try extracting from GraphPath
    if (!OwningBP)
    {
        int32 LastColonIndex = -1;
        if (GraphPath.FindLastChar(TEXT(':'), LastColonIndex))
        {
            FString PathBeforeColon = GraphPath.Left(LastColonIndex);
            FTopLevelAssetPath AssetPathHelper(PathBeforeColon);
            FString BlueprintAssetPathString = AssetPathHelper.ToString();
            
            if (!BlueprintAssetPathString.IsEmpty())
            {
                UE_LOG(LogExtractor, Warning, TEXT("FindOwningBlueprint: Attempting LoadObject<UBlueprint> on '%s'"), *BlueprintAssetPathString);
                OwningBP = LoadObject<UBlueprint>(nullptr, *BlueprintAssetPathString);
            }
        }
    }

    if (OwningBP)
    {
        UE_LOG(LogExtractor, Log, TEXT("FindOwningBlueprint: Found owning blueprint: '%s'"), *OwningBP->GetPathName());
    }
    else
    {
        UE_LOG(LogExtractor, Warning, TEXT("FindOwningBlueprint: Could not find owning blueprint for graph '%s'"), *Graph->GetName());
    }

    return OwningBP;
}

bool FBlueprintDataExtractor::IsProblematicMainEventGraph(UEdGraph* Graph, UBlueprint* OwningBP) const
{
    if (!Graph || !OwningBP)
    {
        return false;
    }

    // Check if it's a native C++ parent that is NOT an Actor, and if Graph is the first UbergraphPage
    bool bIsNativeNonActorParent = OwningBP->ParentClass && 
                                   OwningBP->ParentClass->GetClassPathName().ToString().StartsWith(TEXT("/Script/")) && 
                                   !OwningBP->ParentClass->IsChildOf(AActor::StaticClass());

    bool bIsMainEventGraph = !OwningBP->UbergraphPages.IsEmpty() && OwningBP->UbergraphPages[0] == Graph;

    bool bIsEmptyGraph = Graph->Nodes.Num() == 0;

    return bIsNativeNonActorParent && bIsMainEventGraph && bIsEmptyGraph;
}

// =============================================================================
// Node Processing Helpers
// =============================================================================

bool FBlueprintDataExtractor::ProcessGraphNodes(UEdGraph* Graph, TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const
{
    if (!Graph)
    {
        return false;
    }

    TArray<UEdGraphNode*> NodesToProcess;
    NodesToProcess.Append(Graph->Nodes);

    UE_LOG(LogExtractor, Log, TEXT("ProcessGraphNodes: Starting processing for %d nodes in graph '%s'"), 
           NodesToProcess.Num(), *Graph->GetName());

    // Convert UEdGraphNodes to FBlueprintNodes
    ConvertNodesToBlueprints(NodesToProcess, Graph->GetName(), OutNodes);

    // Finalize with critical properties and link resolution
    if (OutNodes.Num() > 0)
    {
        FinalizeExtractedNodes(OutNodes);
    }
    else
    {
        UE_LOG(LogExtractor, Log, TEXT("ProcessGraphNodes: No nodes successfully converted for graph '%s'"), *Graph->GetName());
    }

    UE_LOG(LogExtractor, Error, TEXT("ProcessGraphNodes: Completed processing for graph '%s'. Final node count: %d"), 
           *Graph->GetName(), OutNodes.Num());

    return true;
}

void FBlueprintDataExtractor::ConvertNodesToBlueprints(
    const TArray<UEdGraphNode*>& Nodes, 
    const FString& GraphName,
    TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const
{
    int32 NodesSuccessfullyConverted = 0;
    int32 NodesFailedConversion = 0;

    for (UEdGraphNode* Node : Nodes)
    {
        if (!Node)
        {
            UE_LOG(LogExtractor, Warning, TEXT("ConvertNodesToBlueprints: Encountered NULL UEdGraphNode in graph '%s'"), *GraphName);
            continue;
        }

        UE_LOG(LogExtractor, Log, TEXT("ConvertNodesToBlueprints: Processing UEdGraphNode: Title='%s', Class='%s', GUID='%s'"), 
               *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
               *Node->GetClass()->GetName(),
               *Node->NodeGuid.ToString());

        TSharedPtr<FBlueprintNode> ExtractedNode = ExtractNodeData(Node);
        
        if (ExtractedNode.IsValid() && !ExtractedNode->Guid.IsEmpty())
        {
            OutNodes.Add(ExtractedNode->Guid, ExtractedNode);
            NodesSuccessfullyConverted++;
            UE_LOG(LogExtractor, Log, TEXT("  ConvertNodesToBlueprints: Successfully converted GUID '%s' (NodeType: '%s')"), 
                   *ExtractedNode->Guid, *ExtractedNode->NodeType);
        }
        else
        {
            NodesFailedConversion++;
            if (!ExtractedNode.IsValid())
            {
                UE_LOG(LogExtractor, Error, TEXT("  ConvertNodesToBlueprints: ExtractNodeData returned NULL for UEdGraphNode: Title='%s'"),
                       *Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
            }
            else if (ExtractedNode->Guid.IsEmpty())
            {
                UE_LOG(LogExtractor, Error, TEXT("  ConvertNodesToBlueprints: Extracted node has EMPTY GUID! Title='%s', NodeType='%s'"),
                       *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(), *ExtractedNode->NodeType);
            }
        }
    }

    UE_LOG(LogExtractor, Error, TEXT("ConvertNodesToBlueprints: Conversion complete for graph '%s'. Success: %d, Failed: %d"), 
           *GraphName, NodesSuccessfullyConverted, NodesFailedConversion);
}

void FBlueprintDataExtractor::FinalizeExtractedNodes(TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes) const
{
    if (Nodes.IsEmpty())
    {
        return;
    }

    // Preserve critical properties before link resolution
    UE_LOG(LogExtractor, Log, TEXT("FinalizeExtractedNodes: Preserving critical properties for %d nodes..."), Nodes.Num());
    int32 PreservedCount = 0;
    for (auto& Pair : Nodes)
    {
        if (Pair.Value.IsValid())
        {
            Pair.Value->PreserveCriticalProperties();
            PreservedCount++;
        }
    }
    UE_LOG(LogExtractor, Log, TEXT("FinalizeExtractedNodes: Preserved critical properties for %d nodes"), PreservedCount);

    // Resolve links between nodes
    UE_LOG(LogExtractor, Log, TEXT("FinalizeExtractedNodes: Resolving links between nodes..."));
    ResolveLinks(Nodes);

    // 🔧 NEW DEBUG: Show all node connections to see execution flow
    UE_LOG(LogExtractor, Error, TEXT("🔧 BLUEPRINT CONNECTION DEBUG:"));
    for (const auto& NodePair : Nodes)
    {
        const TSharedPtr<FBlueprintNode>& Node = NodePair.Value;
        if (Node.IsValid() && (Node->NodeType == TEXT("VariableGet") || Node->NodeType == TEXT("CommutativeAssociativeBinaryOperator")))
        {
            UE_LOG(LogExtractor, Error, TEXT("🔧   Node: '%s' (GUID:%s, Type:%s)"), *Node->Name, *Node->Guid.Left(8), *Node->NodeType);
        
            for (const auto& PinPair : Node->Pins)
            {
                const TSharedPtr<FBlueprintPin>& Pin = PinPair.Value;
                if (Pin.IsValid() && Pin->SourcePinFor.Num() > 0)
                {
                    UE_LOG(LogExtractor, Error, TEXT("🔧     Pin '%s' connects to %d sources:"), *Pin->Name, Pin->SourcePinFor.Num());
                    for (int32 i = 0; i < Pin->SourcePinFor.Num(); ++i)
                    {
                        if (Pin->SourcePinFor[i].IsValid())
                        {
                            UE_LOG(LogExtractor, Error, TEXT("🔧       Source[%d]: NodeGUID='%s', PinName='%s'"), 
                                i, *Pin->SourcePinFor[i]->NodeGuid.Left(8), *Pin->SourcePinFor[i]->Name);
                        }
                    }
                }
            }
        }
    }

    UE_LOG(LogExtractor, Error, TEXT("DEBUG Post-ResolveLinks: Inspecting SourcePinFor for GetDataTableRow input pins:"));

    
    UE_LOG(LogExtractor, Log, TEXT("FinalizeExtractedNodes: Link resolution complete"));
}

// =============================================================================
// Core extraction methods (unchanged)
// =============================================================================

TSharedPtr<FBlueprintNode> FBlueprintDataExtractor::ExtractNodeData(UEdGraphNode* GraphNode) const
{
    if (!GraphNode)
    {
        return nullptr;
    }

    return FBlueprintNodeFactory::CreateNode(GraphNode);
}

void FBlueprintDataExtractor::ResolveLinks(TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes) const
{
    UE_LOG(LogExtractor, Log, TEXT("FBlueprintDataExtractor: Starting link resolution..."));
    int32 ResolvedCount = 0;
    int32 BackRefCount = 0;

    for (auto& NodePair : Nodes)
    {
        TSharedPtr<FBlueprintNode>& SourceNode = NodePair.Value;
        if (!SourceNode.IsValid()) continue;

        bool bIsBoundEvent = (SourceNode->NodeType == TEXT("ComponentBoundEvent") || SourceNode->NodeType == TEXT("ActorBoundEvent"));
        int32 InitialPropCount = -1;
        if (bIsBoundEvent) {
            InitialPropCount = SourceNode->RawProperties.Num();
            UE_LOG(LogExtractor, Error, TEXT("ResolveLinks: BEFORE Pins Loop for %s (%s). Initial RawProperties.Num() = %d"),
                *SourceNode->Guid.Left(8), *SourceNode->NodeType, InitialPropCount);
        }
        UE_LOG(LogExtractor, Error, TEXT("🔧 LINK RESOLUTION DEBUG: Processing Node='%s' (GUID:%s)"), 
    *SourceNode->Name, *SourceNode->Guid.Left(8));

        for (auto& PinPair : SourceNode->Pins) {
            TSharedPtr<FBlueprintPin>& Pin = PinPair.Value;
            if (Pin.IsValid() && Pin->IsInput()) {
                UE_LOG(LogExtractor, Error, TEXT("🔧   Input Pin='%s', LinkedTo props in RawProperties:"), *Pin->Name);
        
                // Check what link info was stored during extraction
                for (const auto& Prop : Pin->RawProperties) {
                    if (Prop.Key.Contains(TEXT("LinkedTo"))) {
                        UE_LOG(LogExtractor, Error, TEXT("🔧     %s = %s"), *Prop.Key, *Prop.Value);
                    }
                }
        
                UE_LOG(LogExtractor, Error, TEXT("🔧   SourcePinFor.Num after resolution: %d"), Pin->SourcePinFor.Num());
            }
        }

        
        for (auto& PinPair : SourceNode->Pins)
        {
            TSharedPtr<FBlueprintPin>& SourcePin = PinPair.Value;
            if (!SourcePin.IsValid() || !SourcePin->IsOutput()) continue;

            int32 LinkIndex = 0;
            FName LinkNodeKeyName = FName(*FString::Printf(TEXT("LinkedTo_%d_NodeGuid"), LinkIndex));
            FName LinkPinKeyName = FName(*FString::Printf(TEXT("LinkedTo_%d_PinID"), LinkIndex));

            while(SourcePin->RawProperties.Contains(LinkNodeKeyName.ToString()) && SourcePin->RawProperties.Contains(LinkPinKeyName.ToString()))
            {
                const FString TargetNodeGuid = SourcePin->RawProperties.FindChecked(LinkNodeKeyName.ToString());
                const FString TargetPinId = SourcePin->RawProperties.FindChecked(LinkPinKeyName.ToString());

                UE_LOG(LogExtractor, Verbose, TEXT("  ResolveLinks: Source Pin '%s' attempting link to Node GUID '%s', Pin ID '%s'"), 
                       *SourcePin->Name, *TargetNodeGuid.Left(8), *TargetPinId);

                TSharedPtr<FBlueprintNode>* TargetNodePtr = Nodes.Find(TargetNodeGuid);

                if (TargetNodePtr && TargetNodePtr->IsValid())
                {
                    TSharedPtr<FBlueprintPin>* TargetPinPtr = (*TargetNodePtr)->Pins.Find(TargetPinId);

                    if (TargetPinPtr && TargetPinPtr->IsValid())
                    {
                        UE_LOG(LogExtractor, Verbose, TEXT("    ResolveLinks: Found Target Node '%s', Target Pin '%s'. Adding links."), 
                               *(*TargetNodePtr)->Name, *(*TargetPinPtr)->Name);

                        SourcePin->LinkedPins.Add(*TargetPinPtr);
                        ResolvedCount++;

                        (*TargetPinPtr)->SourcePinFor.Add(SourcePin);
                        BackRefCount++;

                        UE_LOG(LogExtractor, Log, TEXT("      ResolveLinks: Added SourcePinFor link to Target Pin '%s' (%s). New SourcePinFor count: %d"),
                            *(*TargetPinPtr)->Name, *(*TargetPinPtr)->NodeGuid.Left(8), (*TargetPinPtr)->SourcePinFor.Num());

                    } else {
                        UE_LOG(LogExtractor, Warning, TEXT("    ResolveLinks: Found Target Node '%s', but Target Pin ID '%s' NOT FOUND on it."), 
                               *(*TargetNodePtr)->Name, *TargetPinId);
                    }
                } else {
                    UE_LOG(LogExtractor, Warning, TEXT("  ResolveLinks: Target Node GUID '%s' NOT FOUND in extracted nodes map. Linked from Node %s Pin %s."),
                           *TargetNodeGuid.Left(8), *SourceNode->Guid.Left(8), *SourcePin->Name);
                }

                LinkIndex++;
                LinkNodeKeyName = FName(*FString::Printf(TEXT("LinkedTo_%d_NodeGuid"), LinkIndex));
                LinkPinKeyName = FName(*FString::Printf(TEXT("LinkedTo_%d_PinID"), LinkIndex));
            }
        }

        if (bIsBoundEvent) {
            int32 FinalPropCount = SourceNode->RawProperties.Num();
            if (FinalPropCount != InitialPropCount) {
                UE_LOG(LogExtractor, Error, TEXT("ResolveLinks: AFTER Pins Loop for %s (%s). RawProperties.Num() CHANGED! Was %d, now %d"),
                      *SourceNode->Guid.Left(8), *SourceNode->NodeType, InitialPropCount, FinalPropCount);
                UE_LOG(LogExtractor, Error, TEXT("  Current Keys:"));
                for(const auto& PropPair : SourceNode->RawProperties) { 
                    UE_LOG(LogExtractor, Error, TEXT("    Key: %s"), *PropPair.Key); 
                }
            } else {
                UE_LOG(LogExtractor, Error, TEXT("ResolveLinks: AFTER Pins Loop for %s (%s). RawProperties.Num() = %d (Unchanged)"),
                      *SourceNode->Guid.Left(8), *SourceNode->NodeType, FinalPropCount);
            }
        }
    }

    UE_LOG(LogExtractor, Log, TEXT("FBlueprintDataExtractor: Finished link resolution. Resolved %d forward links, added %d SourcePinFor back-references."), 
           ResolvedCount, BackRefCount);
}

void FBlueprintDataExtractor::ExpandNestedFunctions(TArray<UEdGraphNode*>& Nodes, TSet<UEdGraph*>& ProcessedGraphs) const
{
    // Stub implementation - Not the focus currently
}

FString FBlueprintDataExtractor::ExtractSimpleNameFromPath(const FString& Path) const
{
    FString Name = Path;
    if (Name.Contains(TEXT("'"))) {
        Name = Name.Mid(Name.Find(TEXT("'")) + 1);
        Name.LeftChopInline(1, EAllowShrinking::No);
    }
    Name.TrimQuotesInline();
    Name = FPaths::GetCleanFilename(Name);
    Name.RemoveFromStart(TEXT("Default__"));
    Name.RemoveFromStart(TEXT("BP_"));
    Name.RemoveFromStart(TEXT("WBP_"));
    Name.RemoveFromStart(TEXT("ABP_"));
    Name.RemoveFromStart(TEXT("K2Node_"));
    Name.RemoveFromStart(TEXT("EdGraphNode_"));
    if (Name.EndsWith(TEXT("_C"))) { 
        Name.LeftChopInline(2, EAllowShrinking::No); 
    }
    if (Name.Contains(TEXT("::"))) { 
        Name = Name.Left(Name.Find(TEXT(":::"))); 
    }
    return Name;
}