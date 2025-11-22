/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownSpanSystem.cpp


#include "MarkdownSpanSystem.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Logging/BP2AILog.h"


// Define the thread_local variable
thread_local FMarkdownGenerationContext* g_MarkdownSpanSystem_CurrentContext = nullptr;

// Context Management
const FMarkdownGenerationContext& FMarkdownSpanSystem::GetCurrentContext()
{
    if (g_MarkdownSpanSystem_CurrentContext)
    {
        return *g_MarkdownSpanSystem_CurrentContext;
    }
    // Default context if none is set (e.g., for systems not explicitly managing context)
    static FMarkdownGenerationContext DefaultContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    UE_LOG(LogBP2AI, Verbose, TEXT("FMarkdownSpanSystem::GetCurrentContext - Using default RawMarkdown context."));
    return DefaultContext;
}

void FMarkdownSpanSystem::SetGlobalContext(const FMarkdownGenerationContext* Context)
{
    g_MarkdownSpanSystem_CurrentContext = const_cast<FMarkdownGenerationContext*>(Context);
    if (Context)
    {
        UE_LOG(LogBP2AI, Verbose, TEXT("FMarkdownSpanSystem::SetGlobalContext - Context set to: %s"), 
            Context->IsHTML() ? TEXT("StyledHTML") : (Context->IsClean() ? TEXT("CleanText") : TEXT("RawMarkdown")));
    }
    else
    {
        UE_LOG(LogBP2AI, Verbose, TEXT("FMarkdownSpanSystem::SetGlobalContext - Context cleared."));
    }
}

void FMarkdownSpanSystem::ClearGlobalContext()
{
    UE_LOG(LogBP2AI, Verbose, TEXT("FMarkdownSpanSystem::ClearGlobalContext - Clearing context."));
    g_MarkdownSpanSystem_CurrentContext = nullptr;
}

FString FMarkdownSpanSystem::EscapeHtml(const FString& Text)
{
    // If text contains our span classes, it's likely already HTML-formatted or structured.
    // Avoid double-escaping. This is a simple check; more robust parsing might be needed for complex cases.
    if (Text.Contains(TEXT("<span")) || Text.Contains(TEXT("class=\"bp-")))
    {
        return Text; 
    }
    
    // Basic HTML escaping
    return Text.Replace(TEXT("&"), TEXT("&amp;"))
              .Replace(TEXT("<"), TEXT("&lt;"))
              .Replace(TEXT(">"), TEXT("&gt;"))
              .Replace(TEXT("\""), TEXT("&quot;"));
              // Single quotes are generally not escaped in HTML attributes unless the attribute itself is quoted with single quotes.
              // .Replace(TEXT("'"), TEXT("&#39;")); 
}

// Helper function to clean text specifically for HTML spans by removing common Markdown syntax
FString FMarkdownSpanSystem::CleanTextForHTML(const FString& Text)
{
    FString CleanText = Text;
    
    // Remove surrounding backticks `code` -> code
    if (CleanText.Len() >= 2 && CleanText.StartsWith(TEXT("`")) && CleanText.EndsWith(TEXT("`")))
    {
        CleanText = CleanText.Mid(1, CleanText.Len() - 2);
    }
    
    // Remove surrounding **bold** -> bold
    if (CleanText.Len() >= 4 && CleanText.StartsWith(TEXT("**")) && CleanText.EndsWith(TEXT("**")))
    {
        CleanText = CleanText.Mid(2, CleanText.Len() - 4);
    }
    
    // Remove surrounding *italic* -> italic (if you use single asterisks for italics)
    // if (CleanText.Len() >= 2 && CleanText.StartsWith(TEXT("*")) && CleanText.EndsWith(TEXT("*")) && !CleanText.StartsWith(TEXT("**")))
    // {
    //     CleanText = CleanText.Mid(1, CleanText.Len() - 2);
    // }

    return CleanText.TrimStartAndEnd();
}

// Core Span Creation - ENHANCED
FString FMarkdownSpanSystem::CreateSpanInternal(const FString& CssClass, const FString& Text)
{
    // This function is primarily for HTML context.
    // For other contexts, the calling semantic function should handle formatting.
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML() && !CssClass.IsEmpty())
    {
        // Clean the text of Markdown syntax before wrapping in HTML span,
        // as CSS will handle the styling.
        FString CleanedTextForHTML = CleanTextForHTML(Text);
        return FString::Printf(TEXT("<span class=\"%s\">%s</span>"), *CssClass, *EscapeHtml(CleanedTextForHTML));
    }
    
    // If not HTML or no CSS class, this function might be misused.
    // The semantic functions should handle non-HTML cases directly.
    // However, to be safe, return the original text if it's not HTML context.
    // This also covers cases where CreateSpanInternal might be called unexpectedly.
    UE_LOG(LogBP2AI, Verbose, TEXT("CreateSpanInternal called in non-HTML context or with empty CSS class for text: %s. Returning raw text."), *Text);
    return Text; 
}

// Semantic Methods - All updated for HTML, CleanText, and RawMarkdown contexts

FString FMarkdownSpanSystem::Keyword(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-keyword"), Text);
    }
    else if (Context.IsClean())
    {
        return Text; // Return clean text without formatting
    }
    // RawMarkdown formatting
    return FString::Printf(TEXT("**%s**"), *Text);
}

FString FMarkdownSpanSystem::Variable(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-var"), Text);
    }
    else if (Context.IsClean())
    {
        // For clean text, remove backticks if they were primarily for Markdown.
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting: preserve backticks if already present, otherwise add them.
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text; 
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::FunctionName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-func-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::EventName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-event-name"), Text);
    }
    else if (Context.IsClean())
    {
        return Text; // Return clean text without bold formatting
    }
    // RawMarkdown formatting
    return FString::Printf(TEXT("**%s**"), *Text);
}

FString FMarkdownSpanSystem::MacroName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-macro-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::DataType(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-data-type"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::PinName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-pin-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::ParamName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-param-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::Operator(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-operator"), Text);
    }
    else if (Context.IsClean())
    {
        return Text; 
    }
    // RawMarkdown formatting (Operators don't typically need special markdown)
    return Text;
}

FString FMarkdownSpanSystem::LiteralString(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        // For HTML, we pass the original Text which might include quotes.
        // CleanTextForHTML will handle the quotes if they are standard Markdown (like backticks).
        // If quotes are part of the string literal itself, they should be preserved and escaped by EscapeHtml.
        return CreateSpanInternal(TEXT("bp-literal-string"), Text);
    }
    else if (Context.IsClean())
    {
        // For clean format, normalize single quotes to double quotes if Text is a single-quoted string.
        FString CleanText = Text;
        if (CleanText.Len() >= 2 && CleanText.StartsWith(TEXT("'")) && CleanText.EndsWith(TEXT("'")))
        {
            // Ensure it's not an empty string like "''" which would become """
            if (CleanText.Len() > 2) 
            {
                CleanText = TEXT("\"") + CleanText.Mid(1, CleanText.Len() - 2) + TEXT("\"");
            }
            else // Handle "''" -> ""
            {
                CleanText = TEXT("\"\"");
            }
        }
        // If already double-quoted or not quoted in a way we transform, return as is.
        return CleanText;
    }
    // RawMarkdown formatting (Strings are usually already quoted, or don't need extra Markdown syntax)
    return Text; 
}

FString FMarkdownSpanSystem::LiteralNumber(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-number"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting
    return Text;
}

FString FMarkdownSpanSystem::LiteralBoolean(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-bool"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting
    return Text;
}

FString FMarkdownSpanSystem::LiteralName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-name"), Text);
    }
    else if (Context.IsClean())
    {
       if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::LiteralObject(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-object"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::LiteralTag(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-tag"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::LiteralContainer(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-container"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting (Containers like [], {} don't typically need backticks)
    return Text;
}

FString FMarkdownSpanSystem::LiteralStructType(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-struct-type"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::LiteralStructVal(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-struct-val"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting (Struct values like (X=1,Y=2) don't typically need backticks)
    return Text;
}

FString FMarkdownSpanSystem::LiteralText(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-text"), Text);
    }
    else if (Context.IsClean())
    {
        // Similar to LiteralString, could normalize quotes, but the original didn't specify for LiteralText.
        // For consistency with the general "clean" approach, just return Text.
        // If specific quote normalization is needed like LiteralString, it can be added here.
        return Text;
    }
    // RawMarkdown formatting (Text literals are often already quoted or represent plain text)
    return Text;
}

FString FMarkdownSpanSystem::LiteralUnknown(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-literal-unknown"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting
    return Text;
}

FString FMarkdownSpanSystem::EnumType(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-enum-type"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::EnumValue(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-enum-value"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::ClassName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-class-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::ComponentName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-component-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::WidgetName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-widget-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::DelegateName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-delegate-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::TimelineName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-timeline-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::Modifier(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-modifier"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting (Modifiers like (Latent) don't typically need special formatting)
    return Text;
}

FString FMarkdownSpanSystem::Info(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-info"), Text);
    }
    else if (Context.IsClean())
    {
        return Text; // Return as-is for clean format
    }
    // RawMarkdown formatting (Info text displayed as-is)
    return Text;
}

FString FMarkdownSpanSystem::Error(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-error"), Text);
    }
    else if (Context.IsClean())
    {
        // Keep error markers but clean the text itself (e.g., remove Markdown from Text if any)
        // The provided example for clean Error is FString::Printf(TEXT("[ERROR: %s]"), *Text)
        // This implies Text itself is the error message, not including the "[ERROR: ...]"
        // If Text can contain markdown, it should be cleaned. For now, assume Text is plain.
        return FString::Printf(TEXT("[ERROR: %s]"), *Text); 
    }
    // RawMarkdown formatting
    return FString::Printf(TEXT("**[ERROR: %s]**"), *Text);
}

FString FMarkdownSpanSystem::GraphName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-graph-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::MontageName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-montage-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::ActionName(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-action-name"), Text);
    }
    else if (Context.IsClean())
    {
        if (Text.Len() >= 2 && Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
        {
            return Text.Mid(1, Text.Len() - 2);
        }
        return Text;
    }
    // RawMarkdown formatting
    if (Text.StartsWith(TEXT("`")) && Text.EndsWith(TEXT("`")))
    {
        return Text;
    }
    return FString::Printf(TEXT("`%s`"), *Text);
}

FString FMarkdownSpanSystem::NodeTitle(const FString& Text)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    if (Context.IsHTML())
    {
        return CreateSpanInternal(TEXT("bp-node-title"), Text);
    }
    else if (Context.IsClean())
    {
        return Text;
    }
    // RawMarkdown formatting (Node titles displayed as-is)
    return Text;
}

// ✅ NEW: Type-aware rendering for enhanced HTML output
FString FMarkdownSpanSystem::CreateTypeSpan(const FBlueprintTypeInfo& TypeInfo)
{
    const FMarkdownGenerationContext& Context = GetCurrentContext();
    
    if (Context.IsHTML())
    {
        // HTML output with full CSS styling
        return FString::Printf(TEXT("<span class=\"%s\">%s</span>"), 
            *TypeInfo.CSSClasses, 
            *EscapeHtml(TypeInfo.DisplayName));
    }
    else if (Context.IsClean())
    {
        // Clean text output
        return TypeInfo.DisplayName;
    }
    else
    {
        // Markdown output with backticks
        return FString::Printf(TEXT("`%s`"), *TypeInfo.DisplayName);
    }
}
