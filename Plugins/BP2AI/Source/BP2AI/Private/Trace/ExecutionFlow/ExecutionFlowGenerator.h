/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/ExecutionFlow/ExecutionFlowGenerator.h
#pragma once

#include "CoreMinimal.h"
#include "Trace/Generation/GenerationShared.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/FMarkdownPathTracer.h" // Include for EUserGraphType

// Forward Declarations
class UEdGraphNode;
class FBlueprintDataExtractor;
class FMarkdownDataTracer;
class FMarkdownPathTracer;
class IDocumentBuilder;
struct FTracingResults;
struct FGenerationSettings;

// ✅ HELPER FORWARD DECLARATIONS
class FGraphDiscoveryHelper;
class FCategoryAnalysisHelper;
class FDefinitionGenerationHelper;
class FFlowValidationHelper;

/**
 * ✅ CLEAN EXECUTION FLOW GENERATOR WITH HELPER COMPOSITION + CACHING
 * 
 * Responsibilities:
 * - Extract node data from UE4/5 graph nodes
 * - Coordinate tracing between PathTracer and DataTracer  
 * - Collect pure structured data into FTracingResults
 * - Cache expensive tracing results for instant category toggling
 * - NO FORMATTING LOGIC - that's handled by document builders
 * 
 * Architecture:
 * - Domain Layer: Pure tracing coordination 
 * - Helper Composition: Complex logic delegated to specialized helpers
 * - Performance Layer: Smart caching for UI responsiveness
 * - No knowledge of HTML vs Markdown (separation of concerns)
 * - No dependency on presentation contexts (dependency rule)
 */
class BP2AI_API FExecutionFlowGenerator
{
public:
    FExecutionFlowGenerator();
    ~FExecutionFlowGenerator();

    // ✅ MAIN ENTRY POINT - Clean architecture compliant
    FString GenerateDocumentForNodes(
       const TArray<UEdGraphNode*>& InSelectedEditorNodes,
       const FGenerationSettings& InSettings,
       const FMarkdownGenerationContext& InContext = FMarkdownGenerationContext()
   );

    // ✅ Fast path for category-only changes (context-aware cache)
    FString GenerateDocumentFromCache(
        const FGenerationSettings& InDisplaySettings,
        const FMarkdownGenerationContext& InContext = FMarkdownGenerationContext()
    );

    // ✅ Cache access for UI integration
    bool HasCachedResults() const { return CachedHTMLResults.IsSet() || CachedMarkdownResults.IsSet(); }

    
    // ✅ Cache management - now context-aware
    // ✅ Cache management
    void InvalidateCache() { 
        CachedHTMLResults.Reset(); 
        CachedMarkdownResults.Reset(); 
        LastTracingSettings.Reset(); 
        LastSelectedNodes.Empty();
    }
    bool IsCacheValidForSettings(const FGenerationSettings& InSettings, const FMarkdownGenerationContext& InContext) const;
    
    bool IsCacheValidForSettings(const FGenerationSettings& InSettings) const;

    // ✅ LEGACY COMPATIBILITY (keep for existing UI)
    FString GenerateMarkdownForNodes(
        const TArray<UEdGraphNode*>& InSelectedEditorNodes,
        bool bInTraceAllSelected,
        bool bInDefineUserGraphsSeparately,
        bool bInExpandCompositesInline,
        bool bInShowTrivialDefaultParams,
        bool bInShouldTraceSymbolicallyForData,
        const FMarkdownGenerationContext& InContext = FMarkdownGenerationContext()
    );

    // ✅ PURE TRACING COORDINATION - now context-aware
    FTracingResults PerformTracing(
        const TArray<UEdGraphNode*>& InSelectedEditorNodes,
        const FGenerationSettings& InSettings,
        const FMarkdownGenerationContext& InContext
    );

    
    FString GenerateHTMLWithEmbeddedMarkdown(
const TArray<UEdGraphNode*>& InSelectedEditorNodes,
const FGenerationSettings& InSettings);

private:
    // ✅ HELPER COMPOSITION (UE 5.5.4 Compliant)
    TUniquePtr<FGraphDiscoveryHelper> DiscoveryHelper;
    TUniquePtr<FCategoryAnalysisHelper> CategoryHelper;
    TUniquePtr<FDefinitionGenerationHelper> DefinitionHelper;
    TUniquePtr<FFlowValidationHelper> ValidationHelper;

    // ✅ NEW: Context-aware performance caching for instant category toggling
    TOptional<FTracingResults> CachedMarkdownResults;
    TOptional<FGenerationSettings> LastTracingSettings; // Settings that affect tracing (not display)
    TArray<UEdGraphNode*> LastSelectedNodes; // For cache invalidation on node changes
    

    // ? NEW: Context-aware performance caching for instant category toggling
    TOptional<FTracingResults> CachedHTMLResults;

    


    static FTracingResults PerformSemanticTracing(
      const TArray<UEdGraphNode*>& InSelectedEditorNodes,
      const FGenerationSettings& InSettings,
      const FMarkdownGenerationContext& InContext);
    
    static FTracingResults PerformLegacyTracing(
        const TArray<UEdGraphNode*>& InSelectedEditorNodes,
        const FGenerationSettings& InSettings,
        const FMarkdownGenerationContext& InContext);

    
    // ✅ DOCUMENT BUILDER FACTORY - context only used for builder selection
    TUniquePtr<IDocumentBuilder> CreateDocumentBuilder(const FMarkdownGenerationContext& Context);

    // ✅ NEW: Cache validation and settings comparison
    bool DoesTracingSettingsMatch(const FGenerationSettings& SettingsA, const FGenerationSettings& SettingsB) const;
    bool DoesNodeSelectionMatch(const TArray<UEdGraphNode*>& NodesA, const TArray<UEdGraphNode*>& NodesB) const;
    bool DoesContextMatch(const FMarkdownGenerationContext& ContextA, const FMarkdownGenerationContext& ContextB) const;
    
    // ✅ PURE DATA EXTRACTION AND COORDINATION
    TMap<FString, TSharedPtr<FBlueprintNode>> ExtractNodeData(
        const TArray<UEdGraphNode*>& InSelectedEditorNodes,
        FBlueprintDataExtractor& InDataExtractor
    );

    // ✅ CORE ORCHESTRATION METHODS (PRESERVED)
    TTuple<TArray<TSharedPtr<const FBlueprintNode>>, TArray<TSharedPtr<const FBlueprintNode>>> 
    FindExecutionStartNodesInternal(
        const TMap<FString, TSharedPtr<FBlueprintNode>>& NodesMap,
        bool bInTraceAllSelected
    ) const;

    void PerformMainExecutionTraces(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& InSelectedNodesMap,
        TSet<FString>& InOutProcessedGlobally,
        FTracingResults& InOutResults,
        const FGenerationSettings& InSettings,
        const FString& BlueprintContextName,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths
    );

    void DefineReferencedGraphs(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& InSharedNodesMap,
        TSet<FString>& InOutProcessedGlobally,
        FTracingResults& Results,
        const FGenerationSettings& InSettings,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& InOutGraphsToDefineSeparately,  
        TSet<FString>& InOutProcessedSeparateGraphPaths
    );

    void ExecutePrintingPhase(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        const TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>>& CategorizedGraphs,
        FTracingResults& Results,
        const FGenerationSettings& InSettings,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        bool& bInOutExecutableMajorGroupHeaderHasBeenPrinted,
        bool& bInOutPureMajorGroupHeaderHasBeenPrinted
    );

    // ✅ HELPER DELEGATION WRAPPERS (PRIVATE)
    // These maintain compatibility with existing internal calls but delegate to helpers
    void PrescanForPureUserGraphs(
        const TMap<FString, TSharedPtr<FBlueprintNode>>& InSelectedNodesMap,
        FMarkdownPathTracer& InPathTracer,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths
    );

    TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>> ExecuteDiscoveryPhase(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& InOutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        const FGenerationSettings& InSettings
    );

    void ProcessSingleGraphForDiscovery(
        FBlueprintDataExtractor& InDataExtractor,
        const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
        TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>>& OutCategorizedGraphs,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        bool& OutHadNewGraphsAdded
    );

    FString CategorizeGraphByType(
        const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes
    );

    FGraphDefinitionEntry CreateGraphDefinition(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
        const FString& CategoryKey,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        const FGenerationSettings& InSettings,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPathsForDiscovery
    );

    void ValidatePhase1Fixes(const FTracingResults& Results);

    // ✅ UTILITY METHODS (PRESERVED)
    FString ExtractBlueprintName(const TArray<UEdGraphNode*>& InSelectedEditorNodes);
    
    // ✅ COLLECTION METHODS FOR TOC (legacy compatibility)
    void CollectTraceHeader(const FString& TraceName, FTracingResults& Results);
    void CollectGraphDefinition(const FString& GraphName, const FString& Category, FTracingResults& Results);

    // ✅ MINIMAL STATE - NO PRESENTATION CONTEXT STORAGE
    FString RootBlueprintNameForTrace;
    
    // ✅ TEMPORARY STATE FOR RESULT COLLECTION
    FTracingResults* CachedResults = nullptr;
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>> TempGraphsToDefineSeparately;
    TSet<FString> TempProcessedSeparateGraphPaths;

 
 
};

// ============================================================================
// ✅ HELPER MIGRATION + CACHING HEADER COMPLETE
// ============================================================================
// 
// NEW FEATURES:
// ✅ Smart Caching: Expensive tracing cached, instant category toggling
// ✅ Cache Validation: Automatic invalidation when tracing settings change
// ✅ UI Integration: Cache access methods for responsive UI
// ✅ Performance Separation: Tracing settings vs display settings
//
// ARCHITECTURE:
// ✅ Clean Separation: Public interface → Core orchestration → Helper delegation
// ✅ Zero Breaking Changes: All existing callers work identically
// ✅ Performance Layer: Smart caching transparently added
// ✅ Helper Composition: Complex logic isolated in specialized classes
//
// ============================================================================