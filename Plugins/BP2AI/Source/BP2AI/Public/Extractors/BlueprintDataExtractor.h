/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/Extractors/BlueprintDataExtractor.h


#pragma once

#include "CoreMinimal.h"
#include "Models/BlueprintNode.h"

class UEdGraphNode;
class UEdGraph;
class UBlueprint;

/**
 * Extracts Blueprint node data from UEdGraphNode objects
 */
class BP2AI_API FBlueprintDataExtractor
{
public:
    FBlueprintDataExtractor();
    ~FBlueprintDataExtractor();
    
    /**
     * Extract data from selected nodes
     * @param SelectedNodes The nodes to extract data from
     * @param bIncludeNestedFunctions Whether to traverse into nested function calls
     * @return Map of node GUIDs to extracted node data
     */
    TMap<FString, TSharedPtr<FBlueprintNode>> ExtractFromSelectedNodes(
        const TArray<UEdGraphNode*>& SelectedNodes,
        bool bIncludeNestedFunctions = false) const;
    
    /**
     * Extract nodes from a specific graph by path
     * @param GraphPath The path to the graph to extract from
     * @param OutNodes Map to populate with extracted nodes
     * @return True if extraction was successful (even if no nodes found)
     */
    bool ExtractNodesFromGraph(const FString& GraphPath, TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const;
    
private:
    // Core extraction methods
    TSharedPtr<FBlueprintNode> ExtractNodeData(UEdGraphNode* GraphNode) const;
    void ResolveLinks(TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes) const;
    void ExpandNestedFunctions(TArray<UEdGraphNode*>& Nodes, TSet<UEdGraph*>& ProcessedGraphs) const;
    
    // Graph resolution helpers
    UEdGraph* ResolveTargetGraph(const FString& GraphPath) const;
    UEdGraph* TryFindGraph(const FString& GraphPath) const;
    UEdGraph* LoadGraphFromPackage(const FString& GraphPath) const;
    
    // Empty graph handling
    void HandleEmptyGraph(UEdGraph* Graph, const FString& GraphPath) const;
    void DiagnoseEmptyGraph(UEdGraph* Graph, const FString& GraphPath, UBlueprint* OwningBP) const;
    bool AttemptGraphInterventions(UEdGraph* Graph, UBlueprint* OwningBP) const;
    UBlueprint* FindOwningBlueprint(UEdGraph* Graph, const FString& GraphPath) const;
    
    // Node processing helpers
    bool ProcessGraphNodes(UEdGraph* Graph, TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const;
    void ConvertNodesToBlueprints(
        const TArray<UEdGraphNode*>& Nodes, 
        const FString& GraphName,
        TMap<FString, TSharedPtr<FBlueprintNode>>& OutNodes) const;
    void FinalizeExtractedNodes(TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes) const;
    
    // Utility helpers
    FString ExtractSimpleNameFromPath(const FString& Path) const;
    bool IsProblematicMainEventGraph(UEdGraph* Graph, UBlueprint* OwningBP) const;
    void LogDiagnosticInfo(UEdGraph* Graph, UBlueprint* OwningBP, const FString& GraphPath) const;
};