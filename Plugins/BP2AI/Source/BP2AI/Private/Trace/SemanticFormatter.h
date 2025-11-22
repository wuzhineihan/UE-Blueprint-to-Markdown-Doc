/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/SemanticFormatter.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/SemanticData.h" // Include the new semantic data structures

// Forward declare FMarkdownSpanSystem to avoid including its header here if not strictly necessary
// However, if formatters directly use FMarkdownSpanSystem::EscapeHtml, it's better to include.
// For now, assuming formatters might need it for utility functions.
#include "Trace/Utils/MarkdownSpanSystem.h"


/**
 * Interface for formatting semantic execution steps into different output string formats.
 * This replaces the direct embedding of FMarkdownSpan calls during the tracing phase,
 * centralizing formatting logic within specific formatter implementations.
 */
class BP2AI_API ISemanticFormatter
{
public:
    virtual ~ISemanticFormatter() = default;

    /**
     * Formats a single FSemanticExecutionStep into a string representation.
     * @param Step The semantic step to format.
     * @return A string representing the formatted step.
     */
    virtual FString FormatExecutionStep(const FSemanticExecutionStep& Step) const = 0;

    /**
     * Formats an array of FSemanticArgument into a string representation (e.g., "(Param1=Val1, Param2=Val2)").
     * @param Args The array of arguments to format.
     * @return A string representing the formatted arguments list.
     */
    virtual FString FormatArguments(const TArray<FSemanticArgument>& Args) const = 0;

    /**
     * Formats a single FSemanticArgument into a string representation (e.g., "ParamName=Value").
     * @param Arg The argument to format.
     * @return A string representing the formatted argument.
     */
    virtual FString FormatArgument(const FSemanticArgument& Arg) const = 0;

protected:
    /**
     * Helper to apply indentation based on the Step's IndentPrefix.
     * Assumes IndentPrefix already contains the correct characters (e.g., "    |-- ").
     * This version prepends the indent prefix and a common execution marker (like "* ").
     */
    FString ApplyFormattingPrefix(const FString& Content, const FString& IndentPrefix, const FString& ExecPrefix = TEXT("* ")) const
    {
        return IndentPrefix + ExecPrefix + Content;
    }
};

/**
 * Formatter for generating clean Markdown, suitable for copy/pasting into documentation.
 * This version aims for readability and avoids Markdown syntax characters (like ** or `)
 * within the core content of execution steps, using them only for overall structure if needed.
 */
class BP2AI_API FCleanMarkdownSemanticFormatter : public ISemanticFormatter
{
public:
    virtual FString FormatExecutionStep(const FSemanticExecutionStep& Step) const override;
    virtual FString FormatArguments(const TArray<FSemanticArgument>& Args) const override;
    virtual FString FormatArgument(const FSemanticArgument& Arg) const override;

private:
    FString GetCleanNodeTypeName(ESemanticNodeType Type) const;
};

/**
 * Formatter for generating traditional "styled" Markdown output.
 * This version will use Markdown syntax like **bold** for keywords and `code` for names,
 * similar to the current FMarkdownSpan::* behavior when in Markdown mode.
 * This is useful for outputs where Markdown rendering will interpret these characters.
 */
class BP2AI_API FStyledMarkdownSemanticFormatter : public ISemanticFormatter
{
public:
    virtual FString FormatExecutionStep(const FSemanticExecutionStep& Step) const override;
    virtual FString FormatArguments(const TArray<FSemanticArgument>& Args) const override;
    virtual FString FormatArgument(const FSemanticArgument& Arg) const override;

private:
    FString GetStyledMarkdownForNodeType(ESemanticNodeType Type, const FString& NodeName) const;
    FString GetStyledMarkdownForArgumentValue(const FSemanticArgument& Arg) const;
};

/**
 * Formatter for generating HTML output, using CSS classes for styling.
 * This version creates HTML structure with appropriate spans and classes,
 * deferring the actual visual styling to CSS.
 */
class BP2AI_API FHTMLSemanticFormatter : public ISemanticFormatter
{
public:
    virtual FString FormatExecutionStep(const FSemanticExecutionStep& Step) const override;
    virtual FString FormatArguments(const TArray<FSemanticArgument>& Args) const override;
    virtual FString FormatArgument(const FSemanticArgument& Arg) const override;

private:
    FString GetHTMLForNodeType(ESemanticNodeType Type, const FString& NodeName, const FString& LinkAnchor) const;
    FString GetHTMLClassForNodeType(ESemanticNodeType Type) const;
    FString GetHTMLClassForArgumentValueType(ESemanticNodeType ValueType) const;
    FString CreateHtmlSpan(const FString& CssClass, const FString& Content, bool bShouldEscapeContent = true) const;
};