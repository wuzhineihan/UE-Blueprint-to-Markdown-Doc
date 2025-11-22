/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/Trace/FMarkdownPathTracer.h
#pragma once

#include "CoreMinimal.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Widgets/SMarkdownOutputWindow.h" // For FCapturedEventData

// Forward declarations
class FBlueprintDataExtractor;
class FMarkdownDataTracer;

class BP2AI_API FMarkdownPathTracer
{
public:
    enum class EUserGraphType : uint8
    {
        Unknown = 0,
        Function = 1,
        Macro = 2,
        CustomEventGraph = 3,
        CollapsedGraph = 4,
        Interface = 5  
    };
    
    struct FTraceStepContext
    {
        TSharedPtr<const FBlueprintPin> IntendedEntryPin;
        
        FTraceStepContext() = default;
        explicit FTraceStepContext(TSharedPtr<const FBlueprintPin> InEntryPin) 
            : IntendedEntryPin(InEntryPin) {}
    };

    FMarkdownPathTracer(FMarkdownDataTracer& InDataTracer, FBlueprintDataExtractor& InDataExtractor);

    // Main tracing method - CLEANED (no context parameter)
    TArray<FString> TraceExecutionPath(
        TSharedPtr<const FBlueprintNode> StartNode,
        const TOptional<FCapturedEventData>& CapturedData,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        bool bInShouldTraceSymbolicallyForData,
        bool bInDefineUserGraphsSeparately,
        bool bInExpandCompositesInline,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        const FString& CurrentBlueprintContext
    );

    // User graph detection
    EUserGraphType IsInternalUserGraph(
        TSharedPtr<const FBlueprintNode> Node,
        FString& OutGraphPath,
        FString& OutGraphNameHint
    ) const;

    // Utility methods
    static FString SanitizeAnchorName(const FString& InputName);

private:
    // Core references
    FMarkdownDataTracer& DataTracerRef;
    FBlueprintDataExtractor& DataExtractorRef;

    // Formatting constants
    FString ExecPrefix;
    FString LineCont;
    FString BranchJoin;
    FString BranchLast;
    FString IndentSpace;
    int32 MaxTraceDepth;

    // Current tracing state
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>* CurrentGraphsToDefineSeparatelyPtr;
    TSet<FString>* CurrentProcessedSeparateGraphPathsPtr;
    FString PathTracerCurrentBlueprintContext;
    bool bCurrentTraceDataSymbolically = false;
    bool bCurrentDefineUserGraphsSeparately = false;
    bool bCurrentExpandCompositesInline = false;

    // Core tracing methods
    void TracePathRecursive(
        TSharedPtr<const FBlueprintNode> CurrentNode,
        const TOptional<FCapturedEventData>& CurrentCapturedData,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        TSet<FString>& ProcessedInCurrentPath,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        const FString& CurrentBlueprintContext,
        const FTraceStepContext& StepContext
    );
    TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>> HandleInitialChecksAndFindExecutable(
        TSharedPtr<const FBlueprintNode> CurrentNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        int32 CurrentDepth,
        const FString& CurrentIndentPrefix,
        TArray<FString>& OutLines
    );

    bool HandlePathLoop(
        TSharedPtr<const FBlueprintNode> NodeToCheck,
        TSet<FString>& ProcessedInCurrentPath,
        const FString& CurrentIndentPrefix,
        TArray<FString>& OutLines
    );
    
    // Helper methods
    FString GenerateMarkdownLine(const FString& Content, const FString& IndentPrefix) const;
    FString CalculateNextPrefix(const FString& CurrentPrefix, bool bIsLastSegment) const;

    TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>> _find_next_executable_node(
        TSharedPtr<const FBlueprintNode> FromNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TArray<FString>& OutLines,
        const FString& SearchIndentPrefix
    );

    // User graph handling
    bool HandleUserGraphNode(
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ExecutableGuid,
        EUserGraphType InternalGraphNodeType,
        const FString& ActualGraphPath,
        const FString& UniqueGraphNameHint,
        bool bIsInternalGraphCall,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        TSet<FString>& ProcessedInCurrentPath,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        bool bWasAlreadyGloballyProcessed,
        TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace
    );

    // Flow control methods
    bool HandleGloballyProcessedNode(
    TSharedPtr<const FBlueprintNode> ExecutableNode,
    TSharedPtr<const FBlueprintPin> TargetPinForExecutable,
    const FString& ExecutableGuid,
    TSet<FString>& InOutProcessedGlobally,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix,
    TArray<FString>& OutLines,
    bool& bWasAlreadyGloballyProcessed,
    const FString& CurrentGuid,
    bool bAddedExecutableToPath,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes  // ADD THIS LINE
);

    void ProcessNodeFormattingAndBranching(
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ExecutableGuid,
        const TOptional<FCapturedEventData>& CurrentCapturedData,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        TSet<FString>& ProcessedInCurrentPath,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        const FString& CurrentBlueprintContext,
        const FTraceStepContext& StepContext
    );

    // Static helper functions (moved to private for organization)
    static EUserGraphType CheckMacroInstanceType_Helper(
        TSharedPtr<const FBlueprintNode> Node,
        const FString& CallingBlueprintName,
        FString& OutGraphPath,
        FString& OutGraphNameHint);

    static EUserGraphType CheckCompositeNodeType_Helper(
        TSharedPtr<const FBlueprintNode> Node,
        const FString& CallingBlueprintName,
        FString& OutGraphPath,
        FString& OutGraphNameHint);

    static EUserGraphType CheckCallFunctionType_Helper(
        TSharedPtr<const FBlueprintNode> Node,
        UBlueprint* CallingNodeOwningBlueprint,
        const FString& CallingBlueprintName,
        FString& OutGraphPath,
        FString& OutGraphNameHint);

    // Helper functions for user graph processing
    static bool ProcessUserFunctionCall_Helper(
        FMarkdownPathTracer* Self,
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ActualGraphPath,
        const FString& UniqueGraphNameHint,
        const FString& DisplayTargetPrefix,
        const FString& FinalLinkTextForDisplay,
        const FString& ArgsStr,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        bool bWasAlreadyGloballyProcessed,
        TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace);

    static bool ProcessMacroCall_Helper(
        FMarkdownPathTracer* Self,
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ActualGraphPath,
        const FString& UniqueGraphNameHint,
        const FString& DisplayTargetPrefix,
        const FString& FinalLinkTextForDisplay,
        const FString& ArgsStr,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        bool bWasAlreadyGloballyProcessed,
        TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace);

    static bool ProcessCollapsedGraph_Helper(
        FMarkdownPathTracer* Self,
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ActualGraphPath,
        const FString& UniqueGraphNameHint,
        const FString& DisplayTargetPrefix,
        const FString& FinalLinkTextForDisplay,
        const FString& ArgsStr,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        TSet<FString>& ProcessedInCurrentPath,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        bool bWasAlreadyGloballyProcessed,
        TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace,
        const FString& PathTracerCurrentBlueprintContextForArgs);

    static bool ProcessCustomEventCall_Helper(
        FMarkdownPathTracer* Self,
        TSharedPtr<const FBlueprintNode> ExecutableNode,
        const FString& ActualGraphPath,
        const FString& UniqueGraphNameHint,
        const FString& DisplayTargetPrefix,
        const FString& FinalLinkTextForDisplay,
        const FString& ArgsStr,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& InOutProcessedGlobally,
        const FString& CurrentIndentPrefix,
        bool bIsLastSegment,
        TArray<FString>& OutLines,
        bool bWasAlreadyGloballyProcessed,
        TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace);

    static bool ProcessInterfaceCall_Helper(
       FMarkdownPathTracer* Self,
       TSharedPtr<const FBlueprintNode> ExecutableNode,
       const FString& ActualGraphPath,
       const FString& UniqueGraphNameHint,
       const FString& DisplayTargetPrefix,
       const FString& FinalLinkTextForDisplay,
       const FString& ArgsStr,
       const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
       TSet<FString>& InOutProcessedGlobally,
       const FString& CurrentIndentPrefix,
       bool bIsLastSegment,
       TArray<FString>& OutLines,
       bool bWasAlreadyGloballyProcessed,
       TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace);

    
};

