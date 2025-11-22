/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownSpanSystem.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/MarkdownGenerationContext.h" // For FMarkdownGenerationContext
#include "Trace/Generation/GenerationShared.h"

/**
 * Internal system for context-aware semantic text formatting.
 * FMarkdownSpan acts as a public wrapper around this system.
 */
class FMarkdownSpanSystem
{
public:
    // Context management
    static const FMarkdownGenerationContext& GetCurrentContext();
    static void SetGlobalContext(const FMarkdownGenerationContext* Context);
    static void ClearGlobalContext();

    // Core span creation (internal name to avoid conflict with FMarkdownSpan::Create if it were different)
    static FString CreateSpanInternal(const FString& CssClass, const FString& Text);
    static FString EscapeHtml(const FString& Unescaped);

    // ✅ NEW: Helper for cleaning text for HTML output
    static FString CleanTextForHTML(const FString& Text);
    
    // Semantic methods
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
    // Add any other semantic types from your FMarkdownSpan list here
    static FString GraphName(const FString& Text);
    static FString MontageName(const FString& Text);
    static FString ActionName(const FString& Text);
    static FString NodeTitle(const FString& Text);

    // ✅ NEW: Type-aware rendering
    static FString CreateTypeSpan(const FBlueprintTypeInfo& TypeInfo);

    
private:
    // Hide constructor, destructor - all static methods
    FMarkdownSpanSystem() = delete;
    ~FMarkdownSpanSystem() = delete;
};