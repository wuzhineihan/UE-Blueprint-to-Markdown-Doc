/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownFormattingUtils.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/SemanticData.h" 
#include "Trace/Generation/GenerationShared.h"
// Forward declarations to avoid circular includes
class FBlueprintPin;
class FBlueprintNode;
class FMarkdownDataTracer;

/**
 * Helper struct for consistently formatting output strings.
 * This is now a thin wrapper around FMarkdownSpanSystem.
 */
struct BP2AI_API FMarkdownSpan
{
public:
    // These methods now delegate to FMarkdownSpanSystem.
    static FString Create(const FString& CssClass, const FString& Text);
    static FString Keyword(const FString& Text);
    static FString Variable(const FString& Text);
    static FString FunctionName(const FString& Text);
    static FString EventName(const FString& Text);
    static FString MacroName(const FString& Text);
    static FString DataType(const FString& Text);
    static FString PinName(const FString& Text);
    static FString ParamName(const FString& Text);
    static FString Operator(const FString& Text);
    static FString LiteralString(const FString& Text);
    static FString LiteralNumber(const FString& Text);
    static FString LiteralBoolean(const FString& Text);
    static FString LiteralName(const FString& Text);
    static FString LiteralObject(const FString& Text);
    static FString LiteralTag(const FString& Text);
    static FString LiteralContainer(const FString& Text);
    static FString LiteralStructType(const FString& Text);
    static FString LiteralStructVal(const FString& Text);
    static FString LiteralText(const FString& Text);
    static FString LiteralUnknown(const FString& Text);
    static FString EnumType(const FString& Text);
    static FString EnumValue(const FString& Text);
    static FString ClassName(const FString& Text);
    static FString ComponentName(const FString& Text);
    static FString WidgetName(const FString& Text);
    static FString DelegateName(const FString& Text);
    static FString TimelineName(const FString& Text);
    static FString Modifier(const FString& Text);
    static FString Info(const FString& Text);
    static FString Error(const FString& Text);
    static FString GraphName(const FString& Text);
    static FString MontageName(const FString& Text);
    static FString ActionName(const FString& Text);
    static FString NodeTitle(const FString& Text);
};

/**
 * Namespace containing static helper functions for formatting node/pin data into Markdown strings.
 */
namespace MarkdownFormattingUtils
{
    BP2AI_API FString FormatOperator(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        int32 Depth,
        TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext);

    BP2AI_API FString FormatUnaryOperator(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        int32 Depth,
        TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext);

    BP2AI_API FString FormatConversion(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
        int32 Depth,
        TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext);

    BP2AI_API FString FormatDefaultValue(
       TSharedPtr<const FBlueprintPin> Pin,
       const FMarkdownDataTracer* Tracer);

    BP2AI_API FString FormatLiteralValue(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueString,
        const FMarkdownDataTracer* Tracer);

    BP2AI_API FString ParseStructDefaultValue(
    const FString& ValueString, const FString& StructTypeName);
    // ===== PHASE 4.1.1: Migration functions for testing =====
    BP2AI_API FString ParseStructDefaultValue_Legacy(
    const FString& ValueString, const FString& StructTypeName);
    /* NOT COMPLETELY INTEGRATED NOT USED     
    BP2AI_API FSemanticValue ParseStructDefaultValue_Semantic(
    const FString& ValueString, const FString& StructTypeName);
 */
    // ===== PHASE 4.1.2: FormatLiteralValue migration =====
    BP2AI_API FString FormatLiteralValue_Legacy(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueString,
        const FMarkdownDataTracer* Tracer);


    /* NOT COMPLETELY INTEGRATED NOT USED 
    BP2AI_API FSemanticValue FormatLiteralValue_Semantic(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueString,
        const FMarkdownDataTracer* Tracer);
 */
    BP2AI_API FString FormatArgumentsForTrace(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
        int32 Depth,
        TSet<FString>& VisitedPins,
        const TSet<FName>& ExcludePinNames,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace = false,
        const FString& CurrentBlueprintContext = TEXT(""));


    // ✅ NEW: Enhanced type parsing and formatting functions
    BP2AI_API FBlueprintTypeInfo ExtractTypeInformation(
        TSharedPtr<const FBlueprintPin> Pin);
        
    BP2AI_API FString FormatTypeForHTML(
        const FBlueprintTypeInfo& TypeInfo);
        
    BP2AI_API FString FormatTypeForMarkdown(
        const FBlueprintTypeInfo& TypeInfo);
        
    BP2AI_API FBlueprintIOSpec CreateIOSpecification(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueExpression = TEXT(""));
        
    BP2AI_API TArray<FBlueprintIOSpec> ExtractPinSpecifications(
        TSharedPtr<const FBlueprintNode> Node,
        bool bInputPins = true);

        
    BP2AI_API FString ConvertEnumInternalToDisplay(
        const FString& InternalValue, 
        const FString& EnumSubCategoryObject);
    
    // Test function declaration (can be removed for final product)
    // This was named TestPhase1MarkdownOnly, implies it's a specific test.
    // If TestDualContextGeneration is more general, it could replace this.
    // For now, I'll keep this as it was in the original file provided for this round.

} // namespace MarkdownFormattingUtils