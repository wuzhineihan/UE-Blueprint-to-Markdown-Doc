/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/SemanticFormatter.cpp

#include "Trace/SemanticFormatter.h"
#include "Trace/Utils/MarkdownSpanSystem.h" // For FMarkdownSpanSystem::EscapeHtml
#include "Logging/BP2AILog.h"   // For UE_LOG

// --- FCleanMarkdownSemanticFormatter Implementation ---

FString FCleanMarkdownSemanticFormatter::FormatExecutionStep(const FSemanticExecutionStep& Step) const
{
    FString ResultString = GetCleanNodeTypeName(Step.NodeType);

    if (!Step.NodeName.IsEmpty())
    {
        ResultString += TEXT(" ") + Step.NodeName;
    }

    if (!Step.TargetExpression.IsEmpty())
    {
        ResultString = Step.TargetExpression + TEXT(".") + ResultString;
    }

    if (Step.Arguments.Num() > 0)
    {
        ResultString += FormatArguments(Step.Arguments);
    }

    if (Step.bIsLatent)
    {
        ResultString += TEXT(" [Latent]");
    }

    if (Step.bIsRepeat)
    {
        ResultString += TEXT(" (Call site repeat)");
    }
    
    if (Step.NodeType == ESemanticNodeType::PathEnd && Step.NodeName.IsEmpty()) // Handle [Path ends] specifically
    {
        ResultString = TEXT("[Path ends]");
    }


    return ApplyFormattingPrefix(ResultString, Step.IndentPrefix);
}

FString FCleanMarkdownSemanticFormatter::FormatArguments(const TArray<FSemanticArgument>& Args) const
{
    if (Args.IsEmpty())
    {
        // For function/macro calls, an empty argument list is "()".
        // For other contexts, it might be an empty string.
        // Assuming call context for now.
        return TEXT("()");
    }

    TArray<FString> FormattedArgs;
    for (const FSemanticArgument& Arg : Args)
    {
        FormattedArgs.Add(FormatArgument(Arg));
    }

    return FString::Printf(TEXT("(%s)"), *FString::Join(FormattedArgs, TEXT(", ")));
}

FString FCleanMarkdownSemanticFormatter::FormatArgument(const FSemanticArgument& Arg) const
{
    // Clean format: ParamName=Value (no special characters like backticks)
    // String literals might retain their single quotes for clarity in plain text.
    if (Arg.DataType.Equals(TEXT("string"), ESearchCase::IgnoreCase) || Arg.DataType.Equals(TEXT("text"), ESearchCase::IgnoreCase) || Arg.DataType.Equals(TEXT("name"), ESearchCase::IgnoreCase))
    {
         if (!Arg.ValueRepresentation.StartsWith(TEXT("'")) && !Arg.ValueRepresentation.EndsWith(TEXT("'")) && Arg.ValueType == ESemanticNodeType::Literal)
         {
            return FString::Printf(TEXT("%s='%s'"), *Arg.Name, *Arg.ValueRepresentation);
         }
    }
    return FString::Printf(TEXT("%s=%s"), *Arg.Name, *Arg.ValueRepresentation);
}

FString FCleanMarkdownSemanticFormatter::GetCleanNodeTypeName(ESemanticNodeType Type) const
{
    // Provides the plain text representation for the node type.
    switch (Type)
    {
        case ESemanticNodeType::Event: return TEXT("Event");
        case ESemanticNodeType::CustomEvent: return TEXT("Custom Event"); // Or "Call Custom Event" depending on context
        case ESemanticNodeType::FunctionCall: return TEXT("Call Function:");
        case ESemanticNodeType::ParentFunctionCall: return TEXT("Call Parent Function:");
        case ESemanticNodeType::MacroCall: return TEXT("Macro:");
        case ESemanticNodeType::CollapsedGraph: return TEXT("Collapsed Graph:");
        case ESemanticNodeType::InterfaceCall: return TEXT("Interface Call:");
        case ESemanticNodeType::Variable: return TEXT("Variable"); // Context will determine Get/Set
        case ESemanticNodeType::Parameter: return TEXT("Parameter");
        case ESemanticNodeType::Literal: return TEXT("Literal");
        case ESemanticNodeType::Operator: return TEXT("Operator");
        case ESemanticNodeType::DataType: return TEXT("DataType");
        case ESemanticNodeType::Sequence: return TEXT("Sequence");
        case ESemanticNodeType::Branch: return TEXT("Branch");
        case ESemanticNodeType::Loop: return TEXT("Loop");
        case ESemanticNodeType::Timeline: return TEXT("Timeline");
        case ESemanticNodeType::Delay: return TEXT("Delay");
        case ESemanticNodeType::PlayMontage: return TEXT("Play Montage");
        case ESemanticNodeType::ReturnNode: return TEXT("Return");
        case ESemanticNodeType::Keyword: return TEXT("Keyword"); // Should be replaced by actual keyword text in Step.NodeName
        case ESemanticNodeType::Comment: return TEXT("Comment");
        case ESemanticNodeType::Modifier: return TEXT("Modifier");
        case ESemanticNodeType::PathEnd: return TEXT(""); // Handled in FormatExecutionStep
        case ESemanticNodeType::Unknown:
        default:
            return TEXT("UnknownNode");
    }
}

// --- FStyledMarkdownSemanticFormatter Implementation ---

FString FStyledMarkdownSemanticFormatter::FormatExecutionStep(const FSemanticExecutionStep& Step) const
{
    FString FormattedString = GetStyledMarkdownForNodeType(Step.NodeType, Step.NodeName);

    if (!Step.TargetExpression.IsEmpty())
    {
        // Assuming TargetExpression is already styled (e.g., `MyVar`) or is 'self'
        FormattedString = FString::Printf(TEXT("`%s`."), *Step.TargetExpression) + FormattedString;
    }

    if (Step.Arguments.Num() > 0)
    {
        FormattedString += FormatArguments(Step.Arguments);
    }
    
    if (!Step.LinkAnchor.IsEmpty() && (Step.NodeType == ESemanticNodeType::FunctionCall || Step.NodeType == ESemanticNodeType::MacroCall || Step.NodeType == ESemanticNodeType::CustomEvent || Step.NodeType == ESemanticNodeType::CollapsedGraph || Step.NodeType == ESemanticNodeType::InterfaceCall))
    {
        // Re-wrap the NodeName part (which is inside FormattedString) with the link
        // This is a bit tricky as GetStyledMarkdownForNodeType already styled NodeName.
        // A more robust way would be for GetStyledMarkdownForNodeType to return parts.
        // For now, let's assume NodeName is the last part that needs linking.
        FString StyledNodeName = FString::Printf(TEXT("`%s`"), *Step.NodeName); // Base assumption
        if(Step.NodeType == ESemanticNodeType::Event || Step.NodeType == ESemanticNodeType::Keyword) StyledNodeName = FString::Printf(TEXT("**%s**"), *Step.NodeName);

        FString LinkedName = FString::Printf(TEXT("[%s](%s)"), *StyledNodeName, *Step.LinkAnchor);
        
        // Heuristic to replace the unlinked styled name with the linked one
        if (FormattedString.Contains(StyledNodeName))
        {
            FormattedString.ReplaceInline(*StyledNodeName, *LinkedName);
        }
        else // Fallback if direct replacement fails
        {
             FormattedString = FormattedString.Replace(*Step.NodeName, *FString::Printf(TEXT("[%s](%s)"), *Step.NodeName, *Step.LinkAnchor));
        }
    }


    if (Step.bIsLatent)
    {
        FormattedString += TEXT(" [(Latent)]"); // Modifier, no special Markdown
    }

    if (Step.bIsRepeat)
    {
        FormattedString += TEXT(" (Call site repeat)"); // Info, no special Markdown
    }
    
    if (Step.NodeType == ESemanticNodeType::PathEnd && Step.NodeName.IsEmpty())
    {
        FormattedString = TEXT("[Path ends]");
    }

    return ApplyFormattingPrefix(FormattedString, Step.IndentPrefix);
}

FString FStyledMarkdownSemanticFormatter::FormatArguments(const TArray<FSemanticArgument>& Args) const
{
    if (Args.IsEmpty())
    {
        return TEXT("()");
    }

    TArray<FString> FormattedArgs;
    for (const FSemanticArgument& Arg : Args)
    {
        FormattedArgs.Add(FormatArgument(Arg));
    }

    return FString::Printf(TEXT("((%s))"), *FString::Join(FormattedArgs, TEXT(", "))); // Double parens for styled arguments
}

FString FStyledMarkdownSemanticFormatter::FormatArgument(const FSemanticArgument& Arg) const
{
    return FString::Printf(TEXT("`%s`=%s"), *Arg.Name, *GetStyledMarkdownForArgumentValue(Arg));
}

FString FStyledMarkdownSemanticFormatter::GetStyledMarkdownForNodeType(ESemanticNodeType Type, const FString& NodeName) const
{
    // Provides Markdown styled representation (e.g., **Keyword**, `Name`)
    switch (Type)
    {
        case ESemanticNodeType::Event: return FString::Printf(TEXT("**Event** **`%s`**"), *NodeName);
        case ESemanticNodeType::CustomEvent: return FString::Printf(TEXT("**Call Custom Event:** **`%s`**"), *NodeName);
        case ESemanticNodeType::FunctionCall: return FString::Printf(TEXT("Call Function: `%s`"), *NodeName);
        case ESemanticNodeType::ParentFunctionCall: return FString::Printf(TEXT("Call Parent Function: `%s`"), *NodeName);
        case ESemanticNodeType::MacroCall: return FString::Printf(TEXT("**Macro:** `%s`"), *NodeName);
        case ESemanticNodeType::CollapsedGraph: return FString::Printf(TEXT("**Collapsed Graph:** `%s`"), *NodeName);
        case ESemanticNodeType::InterfaceCall: return FString::Printf(TEXT("Interface Call: `%s`"), *NodeName);
        case ESemanticNodeType::Variable: return FString::Printf(TEXT("`%s`"), *NodeName); // Used for variable value representation
        case ESemanticNodeType::Sequence: return TEXT("***Sequence***");
        case ESemanticNodeType::Branch: return FString::Printf(TEXT("**Branch** on (`%s`)"), *NodeName); // NodeName here is condition
        case ESemanticNodeType::Loop: return FString::Printf(TEXT("***For Each*** in (`%s`)"), *NodeName); // NodeName here is array
        case ESemanticNodeType::Timeline: return FString::Printf(TEXT("***Timeline:*** **`%s`**"), *NodeName);
        case ESemanticNodeType::Delay: return FString::Printf(TEXT("***Delay***"));
        case ESemanticNodeType::PlayMontage: return FString::Printf(TEXT("***Play Montage*** **`%s`**"), *NodeName);
        case ESemanticNodeType::ReturnNode: return TEXT("***Return***");
        case ESemanticNodeType::Keyword: return FString::Printf(TEXT("**%s**"), *NodeName);
        case ESemanticNodeType::Comment: return FString::Printf(TEXT("/* %s */"), *NodeName);
        case ESemanticNodeType::PathEnd: return TEXT(""); // Handled in FormatExecutionStep
        default:
            return FString::Printf(TEXT("`%s`"), *NodeName); // Default to backticks
    }
}

FString FStyledMarkdownSemanticFormatter::GetStyledMarkdownForArgumentValue(const FSemanticArgument& Arg) const
{
    switch (Arg.ValueType)
    {
        case ESemanticNodeType::Literal:
            if (Arg.DataType.Equals(TEXT("string"), ESearchCase::IgnoreCase) || Arg.DataType.Equals(TEXT("text"), ESearchCase::IgnoreCase))
                return FString::Printf(TEXT("'%s'"), *Arg.ValueRepresentation); // Strings/Text with single quotes
            if (Arg.DataType.Equals(TEXT("name"), ESearchCase::IgnoreCase))
                return FString::Printf(TEXT("`%s`"), *Arg.ValueRepresentation); // Names with backticks
            return Arg.ValueRepresentation; // Numbers, bools as is
        case ESemanticNodeType::Variable:
            return FString::Printf(TEXT("`%s`"), *Arg.ValueRepresentation);
        case ESemanticNodeType::FunctionCall: // Value is a function call result
             // Assuming ValueRepresentation is already formatted symbolically like "FunctionName()"
            return Arg.ValueRepresentation;
        default:
            return FString::Printf(TEXT("`%s`"), *Arg.ValueRepresentation); // Default to backticks
    }
}


// --- FHTMLSemanticFormatter Implementation ---

FString FHTMLSemanticFormatter::FormatExecutionStep(const FSemanticExecutionStep& Step) const
{
    FString HtmlString = GetHTMLForNodeType(Step.NodeType, Step.NodeName, Step.LinkAnchor);

    if (!Step.TargetExpression.IsEmpty())
    {
        // Escape target expression before putting it into a span or directly
        FString EscapedTarget = FMarkdownSpanSystem::EscapeHtml(Step.TargetExpression);
        FString TargetSpan;
        // Heuristic: if it looks like a variable (common for targets), style it as such
        if (EscapedTarget.StartsWith(TEXT("`")) && EscapedTarget.EndsWith(TEXT("`")))
        {
            TargetSpan = CreateHtmlSpan(TEXT("bp-var"), EscapedTarget);
        }
        else
        {
            TargetSpan = EscapedTarget; // Assume it's already formatted or simple text
        }
        HtmlString = TargetSpan + TEXT(".") + HtmlString;
    }

    if (Step.Arguments.Num() > 0)
    {
        HtmlString += FormatArguments(Step.Arguments);
    }

    if (Step.bIsLatent)
    {
        HtmlString += TEXT(" ") + CreateHtmlSpan(TEXT("bp-modifier"), TEXT("[Latent]"));
    }

    if (Step.bIsRepeat)
    {
        HtmlString += TEXT(" ") + CreateHtmlSpan(TEXT("repeat-indicator"), TEXT("(Call site repeat)"));
    }
    
    if (Step.NodeType == ESemanticNodeType::PathEnd && Step.NodeName.IsEmpty())
    {
        HtmlString = CreateHtmlSpan(TEXT("bp-info"), TEXT("[Path ends]"));
    }

    return ApplyFormattingPrefix(HtmlString, Step.IndentPrefix);
}

FString FHTMLSemanticFormatter::FormatArguments(const TArray<FSemanticArgument>& Args) const
{
    if (Args.IsEmpty())
    {
        return TEXT("()");
    }

    TArray<FString> FormattedArgs;
    for (const FSemanticArgument& Arg : Args)
    {
        FormattedArgs.Add(FormatArgument(Arg));
    }

    return FString::Printf(TEXT("(%s)"), *FString::Join(FormattedArgs, TEXT(", ")));
}

FString FHTMLSemanticFormatter::FormatArgument(const FSemanticArgument& Arg) const
{
    FString NameSpan = CreateHtmlSpan(TEXT("bp-param-name"), Arg.Name);
    FString ValueHtml = FMarkdownSpanSystem::EscapeHtml(Arg.ValueRepresentation); // Base value, escaped

    // Apply specific styling based on the ValueType
    FString ValueCssClass = GetHTMLClassForArgumentValueType(Arg.ValueType);
    if (!ValueCssClass.IsEmpty())
    {
        // If the value representation already has backticks (like `MyVar`), clean them for HTML span.
        FString CleanValueForSpan = Arg.ValueRepresentation;
        if (CleanValueForSpan.StartsWith(TEXT("`")) && CleanValueForSpan.EndsWith(TEXT("`")))
        {
            CleanValueForSpan = CleanValueForSpan.Mid(1, CleanValueForSpan.Len() - 2);
        }
        ValueHtml = CreateHtmlSpan(ValueCssClass, CleanValueForSpan);
    }
    else if (Arg.ValueType == ESemanticNodeType::Literal) // More specific literal handling
    {
        if (Arg.DataType.Equals(TEXT("string"), ESearchCase::IgnoreCase) || Arg.DataType.Equals(TEXT("text"), ESearchCase::IgnoreCase))
        {
             FString InnerValue = Arg.ValueRepresentation;
             if(InnerValue.StartsWith(TEXT("'")) && InnerValue.EndsWith(TEXT("'"))) InnerValue = InnerValue.Mid(1, InnerValue.Len()-2);
            ValueHtml = CreateHtmlSpan(TEXT("bp-literal-string"), FString::Printf(TEXT("'%s'"), *FMarkdownSpanSystem::EscapeHtml(InnerValue)));
        }
        else if (Arg.DataType.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
        {
            ValueHtml = CreateHtmlSpan(TEXT("bp-literal-bool"), FMarkdownSpanSystem::EscapeHtml(Arg.ValueRepresentation.ToLower()));
        }
        // Add more literal types (number, name, object) here if needed
    }


    return FString::Printf(TEXT("%s=%s"), *NameSpan, *ValueHtml);
}

FString FHTMLSemanticFormatter::GetHTMLForNodeType(ESemanticNodeType Type, const FString& NodeName, const FString& LinkAnchor) const
{
    FString EscapedNodeName = FMarkdownSpanSystem::EscapeHtml(NodeName);
    FString CssClass = GetHTMLClassForNodeType(Type);
    FString NodeNameContent = CreateHtmlSpan(CssClass, EscapedNodeName);

    if (!LinkAnchor.IsEmpty())
    {
        FString LinkClass = TEXT("function-link"); // Default link class
        if (Type == ESemanticNodeType::MacroCall) LinkClass = TEXT("macro-link");
        else if (Type == ESemanticNodeType::CustomEvent) LinkClass = TEXT("custom-event-link");
        else if (Type == ESemanticNodeType::CollapsedGraph) LinkClass = TEXT("collapsed-graph-link");
        else if (Type == ESemanticNodeType::InterfaceCall) LinkClass = TEXT("interface-link");
        
        NodeNameContent = FString::Printf(TEXT("<a href=\"%s\" class=\"graph-link %s\">%s</a>"), *LinkAnchor, *LinkClass, *NodeNameContent);
    }

    // Add descriptive prefixes for calls
    switch (Type)
    {
        case ESemanticNodeType::Event: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Event")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::CustomEvent: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Call Custom Event:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::FunctionCall: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Call Function:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::ParentFunctionCall: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Call Parent Function:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::MacroCall: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Macro:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::CollapsedGraph: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Collapsed Graph:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::InterfaceCall: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Interface Call:")) + TEXT(" ") + NodeNameContent;
        case ESemanticNodeType::Variable: return NodeNameContent; // Variable itself
        case ESemanticNodeType::Sequence: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Sequence"));
        case ESemanticNodeType::Branch: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Branch")) + TEXT(" on (") + CreateHtmlSpan(TEXT("bp-var"), EscapedNodeName) + TEXT(")");
        case ESemanticNodeType::Loop: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("For Each")) + TEXT(" in (") + CreateHtmlSpan(TEXT("bp-var"), EscapedNodeName) + TEXT(")");
        case ESemanticNodeType::Timeline: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Timeline:")) + TEXT(" ") + CreateHtmlSpan(TEXT("bp-timeline-name"), EscapedNodeName);
        case ESemanticNodeType::Delay: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Delay"));
        case ESemanticNodeType::PlayMontage: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Play Montage")) + TEXT(" ") + CreateHtmlSpan(TEXT("bp-montage-name"), EscapedNodeName);
        case ESemanticNodeType::ReturnNode: return CreateHtmlSpan(TEXT("bp-keyword"), TEXT("Return"));
        case ESemanticNodeType::Keyword: return CreateHtmlSpan(TEXT("bp-keyword"), EscapedNodeName);
        case ESemanticNodeType::Comment: return CreateHtmlSpan(TEXT("bp-comment"), FString::Printf(TEXT("/* %s */"), *EscapedNodeName));
        case ESemanticNodeType::PathEnd: return TEXT(""); // Handled in FormatExecutionStep
        default:
            return CreateHtmlSpan(TEXT("bp-error"), FString::Printf(TEXT("Unknown: %s"), *EscapedNodeName));
    }
}

FString FHTMLSemanticFormatter::GetHTMLClassForNodeType(ESemanticNodeType Type) const
{
    // Maps ESemanticNodeType to the primary CSS class for the node name part.
    switch (Type)
    {
        case ESemanticNodeType::Event: return TEXT("bp-event-name");
        case ESemanticNodeType::CustomEvent: return TEXT("bp-event-name");
        case ESemanticNodeType::FunctionCall: return TEXT("bp-func-name");
        case ESemanticNodeType::ParentFunctionCall: return TEXT("bp-func-name");
        case ESemanticNodeType::MacroCall: return TEXT("bp-macro-name");
        case ESemanticNodeType::CollapsedGraph: return TEXT("bp-graph-name");
        case ESemanticNodeType::InterfaceCall: return TEXT("bp-func-name"); // Often styled like functions
        case ESemanticNodeType::Variable: return TEXT("bp-var");
        case ESemanticNodeType::Timeline: return TEXT("bp-timeline-name");
        case ESemanticNodeType::PlayMontage: return TEXT("bp-montage-name");
        case ESemanticNodeType::Keyword: return TEXT("bp-keyword");
        // Types like Sequence, Branch, Loop, Delay, ReturnNode are often keywords themselves,
        // their "NodeName" might be empty or a specific instance name.
        // The GetHTMLForNodeType handles the keyword part, and NodeName if present gets its own span.
        default: return TEXT("bp-node-title"); // Generic fallback
    }
}

FString FHTMLSemanticFormatter::GetHTMLClassForArgumentValueType(ESemanticNodeType ValueType) const
{
    switch (ValueType)
    {
        case ESemanticNodeType::Literal: return TEXT("bp-literal-string"); // Default literal, refined by DataType in FormatArgument
        case ESemanticNodeType::Variable: return TEXT("bp-var");
        case ESemanticNodeType::FunctionCall: return TEXT("bp-func-name"); // If an arg is a result of a func call
        case ESemanticNodeType::MacroCall: return TEXT("bp-macro-name");
        case ESemanticNodeType::Operator: return TEXT("bp-operator"); // If an arg is result of an op
        case ESemanticNodeType::DataType: return TEXT("bp-data-type"); // e.g. for a class literal
        default: return TEXT("bp-literal-unknown"); // Fallback
    }
}

FString FHTMLSemanticFormatter::CreateHtmlSpan(const FString& CssClass, const FString& Content, bool bShouldEscapeContent) const
{
    FString FinalContent = bShouldEscapeContent ? FMarkdownSpanSystem::EscapeHtml(Content) : Content;
    return FString::Printf(TEXT("<span class=\"%s\">%s</span>"), *CssClass, *FinalContent);
}