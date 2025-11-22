/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/SemanticDataHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/Generation/GenerationShared.h"
#include "Trace/SemanticData.h" // Make sure FSemanticValue is included

/**
 * ?? MIGRATION UTILITY CLASS - For Phase 4 semantic conversion validation ONLY
 * DO NOT use for production features - only for testing semantic equivalence
 */
class BP2AI_API FSemanticDataHelper
{
public:
    // ===== VALIDATION UTILITIES =====
    
    /**
     * CRITICAL: Converts semantic step back to legacy FMarkdownSpan format
     * Must produce EXACT output match for validation during migration
     */
    static FString GenerateLegacyFormattedString(const FSemanticExecutionStep& Step);
    
    /**
     * Converts semantic arguments to legacy "Param=Value, Param2=Value2" format
     */
    static FString GenerateLegacyArgumentList(const TArray<FSemanticArgument>& Arguments);
    
    /**
     * Converts single semantic argument to legacy format with proper type styling
     */
    static FString GenerateLegacyArgumentString(const FSemanticArgument& Arg);
    
    /**
     * Clean strings for comparison - removes formatting markup for diff
     */
    static FString CleanStringForComparison(const FString& FormattedString);
    
    /**
     * Validate execution step conversion  Validate semantic conversion matches legacy output 
     */
    static bool ValidateSemanticConversion(
    const FString& LegacyOutput, 
    const FSemanticExecutionStep& SemanticStep,  // ← Keep existing signature
    const FString& ContextInfo = TEXT("")
);

   // Validate individual value conversion (for FormatDefaultValue, etc.)
    
    // ===== CONVERSION UTILITIES =====
    static bool ValidateValueConversion(
      const FString& LegacyOutput, 
      const FString& SemanticOutput,
      const FString& ContextInfo = TEXT("")
  );

    static FSemanticArgument CreateErrorArgument( 
       const FString& ParamName, 
       const FString& ErrorMessage
   );

    

    
    
    // ===== FACTORY METHODS =====
    
    static FSemanticExecutionStep CreateEventStep(const FString& EventName);
    static FSemanticExecutionStep CreateFunctionCallStep(
        const FString& FuncName, 
        const FString& TargetExpression = TEXT(""),
        const TArray<FSemanticArgument>& Args = {}
    );
    static FSemanticExecutionStep CreateOperatorStep( // Declaration was missing
        const FString& Operator, 
        const TArray<FSemanticArgument>& Operands
    );
    static FSemanticExecutionStep CreateKeywordStep(const FString& Keyword);
    static FSemanticExecutionStep CreatePathEndStep(const FString& Reason = TEXT(""));
    
    static FSemanticArgument CreateLiteralArgument(
        const FString& ParamName, 
        const FString& LiteralValue, 
        const FString& DataType
    );
   

  
    
    
    /**
     * Converts an FSemanticValue to its final FString representation using FMarkdownSpanSystem.
     * This is used by the semantic path to produce the string for display or validation.
     */
    static FString ConvertSemanticValueToString(const FSemanticValue& Value);

    /**
    * Converts semantic literal value to formatted string (for FormatLiteralValue validation).
    */
    static FString ConvertSemanticLiteralValueToString(const FSemanticValue& LiteralValue);

    
private:
    // Internal conversion helpers
    static FString FormatNodeTypeForLegacy(ESemanticNodeType NodeType);
    static FString FormatValueForLegacy(const FSemanticArgument& Arg);
    // static FString FormatDataTypeForLegacy(const FString& DataType); // Declaration was present but not used, can be removed if not needed
};