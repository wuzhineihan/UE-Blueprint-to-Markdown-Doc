/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/FlowValidationHelper.cpp

#include "FlowValidationHelper.h"

#include "Trace/Generation/GenerationShared.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownTracerUtils.h"

FFlowValidationHelper::FFlowValidationHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FFlowValidationHelper: Initialized"));
}

FFlowValidationHelper::~FFlowValidationHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FFlowValidationHelper: Destroyed"));
}

void FFlowValidationHelper::FDuplicateDetectionReport::RecordEntry(
    const FString& DefKey, 
    const FString& GraphPath, 
    const FString& GraphNameHint)
{
    DefKeyToHints.FindOrAdd(DefKey).AddUnique(GraphNameHint);
    GraphPathToHints.FindOrAdd(GraphPath).AddUnique(GraphNameHint);
    HintToDefKeys.FindOrAdd(GraphNameHint).AddUnique(DefKey);
}

bool FFlowValidationHelper::FDuplicateDetectionReport::HasDuplicates() const
{
    // ✅ CORRECT LOGIC: Only fail on hints with multiple DefKeys
    // Multiple hints mapping to same DefKey = successful normalization (GOOD)
    // Same hint mapping to multiple DefKeys = ambiguous resolution (BAD)
    
    for (const auto& Pair : HintToDefKeys)
    {
        if (Pair.Value.Num() > 1)
        {
            UE_LOG(LogPathTracer, Error, TEXT("TRUE DUPLICATE: Hint '%s' maps to %d DefKeys"), 
                *Pair.Key, Pair.Value.Num());
            return true;
        }
    }
    
    return false;
}

void FFlowValidationHelper::FDuplicateDetectionReport::LogReport() const
{
    UE_LOG(LogPathTracer, Warning, TEXT("=== DUPLICATE DETECTION REPORT ==="));
    UE_LOG(LogPathTracer, Warning, TEXT("Total DefKeys: %d"), DefKeyToHints.Num());
    UE_LOG(LogPathTracer, Warning, TEXT("Total GraphPaths: %d"), GraphPathToHints.Num());
    UE_LOG(LogPathTracer, Warning, TEXT("Total Hints: %d"), HintToDefKeys.Num());
    
    // Report successful normalizations (multiple hints → one DefKey = GOOD)
    int32 SuccessfulNormalizations = 0;
    for (const auto& Pair : DefKeyToHints)
    {
        if (Pair.Value.Num() > 1)
        {
            SuccessfulNormalizations++;
            UE_LOG(LogPathTracer, Warning, TEXT("✅ SUCCESSFUL NORMALIZATION: DefKey '%s' consolidates %d hints:"), 
                *Pair.Key, Pair.Value.Num());
            for (const FString& Hint : Pair.Value)
            {
                UE_LOG(LogPathTracer, Warning, TEXT("  - '%s'"), *Hint);
            }
        }
    }
    
    // Report true failures (one hint → multiple DefKeys = BAD)
    int32 TrueFailures = 0;
    for (const auto& Pair : HintToDefKeys)
    {
        if (Pair.Value.Num() > 1)
        {
            TrueFailures++;
            UE_LOG(LogPathTracer, Error, TEXT("❌ TRUE DUPLICATE: Hint '%s' maps to %d DefKeys:"), 
                *Pair.Key, Pair.Value.Num());
            for (const FString& DefKey : Pair.Value)
            {
                UE_LOG(LogPathTracer, Error, TEXT("  - '%s'"), *DefKey);
            }
        }
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("Summary: %d successful normalizations, %d true failures"), 
        SuccessfulNormalizations, TrueFailures);
    
    if (TrueFailures == 0)
    {
        UE_LOG(LogPathTracer, Warning, TEXT("✅ NO TRUE DUPLICATES DETECTED"));
    }
    else
    {
        UE_LOG(LogPathTracer, Error, TEXT("❌ %d TRUE DUPLICATES FOUND"), TrueFailures);
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("=== END DUPLICATE DETECTION REPORT ==="));
}

FFlowValidationHelper::FDuplicateDetectionReport FFlowValidationHelper::ValidateGraphDiscovery(
    const TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& DiscoveredGraphs) const
{
    FDuplicateDetectionReport Report;
    
    for (const auto& GraphInfo : DiscoveredGraphs)
    {
        const FString& GraphNameHint = GraphInfo.Get<0>();
        const FString& GraphPath = GraphInfo.Get<1>();
        FMarkdownPathTracer::EUserGraphType GraphType = GraphInfo.Get<2>();
        
        FString DefKey = MarkdownTracerUtils::GetCanonicalDefKey(GraphPath, GraphNameHint, GraphType);  // <<<< USING SHARED UTILITY
        Report.RecordEntry(DefKey, GraphPath, GraphNameHint);
    }
    
    return Report;
}

void FFlowValidationHelper::ValidatePhase1Fixes(const FTracingResults& Results)
{
    UE_LOG(LogPathTracer, Warning, TEXT("=== PHASE 1 FIXES VALIDATION START ==="));
        
    ValidateInterfaceSignatureExtraction(Results);
    ValidateCustomEventsDiscovery(Results);
        
    UE_LOG(LogPathTracer, Warning, TEXT("=== PHASE 1 FIXES VALIDATION COMPLETE ==="));
}

void FFlowValidationHelper::ValidateInterfaceSignatureExtraction(const FTracingResults& Results) const
{
    UE_LOG(LogPathTracer, Warning, TEXT("=== INTERFACE SIGNATURE VALIDATION ==="));
    
    int32 InterfaceDefinitions = 0;
    int32 ValidSignatures = 0;
    int32 PlaceholderSignatures = 0;
    
    for (const FGraphDefinitionEntry& GraphDef : Results.GraphDefinitions) {
        if (GraphDef.Category == TEXT("Interfaces")) {
            InterfaceDefinitions++;
            
            bool bHasValidInputs = false;
            bool bHasValidOutputs = false;
            bool bHasPlaceholders = false;
            
            // Check inputs
            for (const FString& InputSpec : GraphDef.InputSpecs) {
                if (InputSpec.Contains(TEXT("*")) && InputSpec.Contains(TEXT("not yet implemented"))) {
                    bHasPlaceholders = true;
                } else if (!InputSpec.Contains(TEXT("*"))) {
                    bHasValidInputs = true;
                }
            }
            
            // Check outputs
            for (const FString& OutputSpec : GraphDef.OutputSpecs) {
                if (OutputSpec.Contains(TEXT("*")) && OutputSpec.Contains(TEXT("not yet implemented"))) {
                    bHasPlaceholders = true;
                } else if (!OutputSpec.Contains(TEXT("*"))) {
                    bHasValidOutputs = true;
                }
            }
            
            if (bHasPlaceholders) {
                PlaceholderSignatures++;
                UE_LOG(LogPathTracer, Error, TEXT("  ❌ PLACEHOLDER SIGNATURE: %s"), *GraphDef.GraphName);
            } else {
                ValidSignatures++;
                UE_LOG(LogPathTracer, Warning, TEXT("  ✅ VALID SIGNATURE: %s (Inputs: %d, Outputs: %d)"), 
                    *GraphDef.GraphName, GraphDef.InputSpecs.Num(), GraphDef.OutputSpecs.Num());
            }
        }
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("Interface Signature Summary:"));
    UE_LOG(LogPathTracer, Warning, TEXT("  Total Interface Definitions: %d"), InterfaceDefinitions);
    UE_LOG(LogPathTracer, Warning, TEXT("  Valid Signatures: %d"), ValidSignatures);
    UE_LOG(LogPathTracer, Warning, TEXT("  Placeholder Signatures: %d"), PlaceholderSignatures);
    
    if (PlaceholderSignatures > 0) {
        UE_LOG(LogPathTracer, Error, TEXT("❌ INTERFACE SIGNATURE EXTRACTION FAILED: %d interfaces still have placeholders"), PlaceholderSignatures);
    } else if (InterfaceDefinitions > 0) {
        UE_LOG(LogPathTracer, Warning, TEXT("✅ INTERFACE SIGNATURE EXTRACTION SUCCESS: All %d interfaces have valid signatures"), InterfaceDefinitions);
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("=== END INTERFACE SIGNATURE VALIDATION ==="));
}

void FFlowValidationHelper::ValidateCustomEventsDiscovery(const FTracingResults& Results) const
{
    UE_LOG(LogPathTracer, Warning, TEXT("=== CUSTOM EVENTS DISCOVERY VALIDATION ==="));
    
    // Count custom events in different sections
    int32 ExecutionTraceCustomEvents = 0;
    int32 DefinitionCustomEvents = 0;
    
    TArray<FString> TracedCustomEvents;
    TArray<FString> DefinedCustomEvents;
    
    // Count in execution traces
    for (const FTraceEntry& TraceEntry : Results.ExecutionTraces) {
        if (TraceEntry.NodeType == TEXT("CustomEvent") || 
            TraceEntry.TraceName.Contains(TEXT("Event")) ||
            TraceEntry.NodeType.Contains(TEXT("Event"))) {
            ExecutionTraceCustomEvents++;
            TracedCustomEvents.Add(TraceEntry.TraceName);
            UE_LOG(LogPathTracer, Verbose, TEXT("  Execution Trace Custom Event: %s"), *TraceEntry.TraceName);
        }
    }
    
    // Count in definitions
    for (const FGraphDefinitionEntry& GraphDef : Results.GraphDefinitions) {
        if (GraphDef.Category == TEXT("Callable Custom Events")) {
            DefinitionCustomEvents++;
            DefinedCustomEvents.Add(GraphDef.GraphName);
            UE_LOG(LogPathTracer, Verbose, TEXT("  Definition Custom Event: %s"), *GraphDef.GraphName);
        }
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("Custom Events Summary:"));
    UE_LOG(LogPathTracer, Warning, TEXT("  Execution Trace Custom Events: %d"), ExecutionTraceCustomEvents);
    UE_LOG(LogPathTracer, Warning, TEXT("  Definition Custom Events: %d"), DefinitionCustomEvents);
    
    // ✅ PROPER VALIDATION: Check if nested discovery is working
    // Rule: If we have executions traces, definitions should be >= traced events
    // because nested calls should be discovered and defined
    bool bNestedDiscoveryWorking = (DefinitionCustomEvents >= ExecutionTraceCustomEvents);
    
    UE_LOG(LogPathTracer, Warning, TEXT("Nested Discovery Analysis:"));
    UE_LOG(LogPathTracer, Warning, TEXT("  Traced Events: %d"), ExecutionTraceCustomEvents);
    UE_LOG(LogPathTracer, Warning, TEXT("  Defined Events: %d"), DefinitionCustomEvents);
    UE_LOG(LogPathTracer, Warning, TEXT("  Additional Discovered: %d"), 
        FMath::Max(0, DefinitionCustomEvents - ExecutionTraceCustomEvents));
    
    if (ExecutionTraceCustomEvents == 0) {
        UE_LOG(LogPathTracer, Warning, TEXT("ℹ️ NO CUSTOM EVENTS: No custom events in this Blueprint context - validation skipped"));
    } else if (bNestedDiscoveryWorking) {
        if (DefinitionCustomEvents > ExecutionTraceCustomEvents) {
            UE_LOG(LogPathTracer, Warning, TEXT("✅ NESTED DISCOVERY SUCCESS: %d additional custom events discovered through nested calls"), 
                DefinitionCustomEvents - ExecutionTraceCustomEvents);
        } else {
            UE_LOG(LogPathTracer, Warning, TEXT("✅ CUSTOM EVENTS DISCOVERY SUCCESS: All traced events properly defined (no nesting detected)"));
        }
    } else {
        UE_LOG(LogPathTracer, Error, TEXT("❌ CUSTOM EVENTS DISCOVERY FAILED: %d traced events but only %d definitions"), 
            ExecutionTraceCustomEvents, DefinitionCustomEvents);
    }
    
    // Additional context-specific validation for known complex cases
    if (Results.RootBlueprintName.Contains(TEXT("DayNightCycle"))) {
        UE_LOG(LogPathTracer, Warning, TEXT("🎯 COMPLEX BLUEPRINT DETECTED: DayNightCycle - checking for nested event chains"));
        
        // For DayNightCycle specifically, we expect nested discovery
        if (DefinitionCustomEvents > ExecutionTraceCustomEvents) {
            UE_LOG(LogPathTracer, Warning, TEXT("✅ COMPLEX NESTED DISCOVERY VERIFIED: Multi-iteration custom event chains working"));
        } else {
            UE_LOG(LogPathTracer, Warning, TEXT("⚠️ COMPLEX BLUEPRINT NOTE: Expected nested custom events in DayNightCycle, but definitions=%d, traces=%d"), 
                DefinitionCustomEvents, ExecutionTraceCustomEvents);
        }
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("=== END CUSTOM EVENTS DISCOVERY VALIDATION ==="));
}
