/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/SemanticDataHelper.cpp

#include "SemanticDataHelper.h"

#include "MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownSpanSystem.h"
#include "Logging/BP2AILog.h"

// ===== VALIDATION UTILITIES =====

FString FSemanticDataHelper::GenerateLegacyFormattedString(const FSemanticExecutionStep& Step)
{
    // ⚠️ CRITICAL: Must produce EXACT legacy format for validation
    
    FString NodeTypePrefix = FormatNodeTypeForLegacy(Step.NodeType);
    FString NodeNameStr = Step.NodeName;
    
    // Add target expression if present
    if (!Step.TargetExpression.IsEmpty() && Step.TargetExpression != TEXT("self"))
    {
        NodeNameStr = Step.TargetExpression + TEXT(".") + NodeNameStr;
    }
    
    // Add arguments if present  
    if (Step.Arguments.Num() > 0)
    {
        FString ArgsStr = GenerateLegacyArgumentList(Step.Arguments);
        NodeNameStr += TEXT("(") + ArgsStr + TEXT(")");
    }
    
    // Add modifiers
    if (Step.bIsLatent)
    {
        NodeNameStr += TEXT(" [(Latent)]");
    }
    
    if (Step.bIsRepeat)
    {
        NodeNameStr += TEXT(" (Call site repeat)");
    }
    
    // Handle special cases
    if (Step.NodeType == ESemanticNodeType::PathEnd)
    {
        return TEXT("[Path ends]");
    }
    
    if (Step.NodeType == ESemanticNodeType::Error)
    {
        return TEXT("[") + Step.NodeName + TEXT("]");
    }
    
    // Combine prefix and content
    if (!NodeTypePrefix.IsEmpty())
    {
        return NodeTypePrefix + TEXT(" ") + NodeNameStr;
    }
    
    return NodeNameStr;
}

FString FSemanticDataHelper::GenerateLegacyArgumentList(const TArray<FSemanticArgument>& Arguments)
{
    if (Arguments.IsEmpty()) 
    {
        return TEXT("");
    }
    
    TArray<FString> ArgStrings;
    for (const FSemanticArgument& Arg : Arguments)
    {
        ArgStrings.Add(GenerateLegacyArgumentString(Arg));
    }
    
    return FString::Join(ArgStrings, TEXT(", "));
}

FString FSemanticDataHelper::GenerateLegacyArgumentString(const FSemanticArgument& Arg)
{
    FString ParamPart = TEXT("`") + Arg.Name + TEXT("`"); // ParamName format
    FString ValuePart = FormatValueForLegacy(Arg);
    
    return ParamPart + TEXT("=") + ValuePart;
}

FString FSemanticDataHelper::CleanStringForComparison(const FString& FormattedString)
{
    return FormattedString
        .Replace(TEXT("**"), TEXT(""))
        .Replace(TEXT("***"), TEXT(""))
        .Replace(TEXT("`"), TEXT(""))
        .Replace(TEXT("[("), TEXT("("))
        .Replace(TEXT(")]"), TEXT(")"))
        .TrimStartAndEnd();
}

bool FSemanticDataHelper::ValidateSemanticConversion(
    const FString& LegacyOutput, 
    const FSemanticExecutionStep& SemanticStep,
    const FString& ContextInfo)
{
    FString SemanticAsLegacy = GenerateLegacyFormattedString(SemanticStep);
    
    FString CleanLegacy = CleanStringForComparison(LegacyOutput);
    FString CleanSemantic = CleanStringForComparison(SemanticAsLegacy);
    
    bool bMatches = CleanLegacy.Equals(CleanSemantic, ESearchCase::IgnoreCase);
    
    if (!bMatches)
    {
        UE_LOG(LogBP2AI, Error, 
            TEXT("🚨 SEMANTIC VALIDATION FAILED %s:"), 
            *ContextInfo);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Legacy Clean: '%s'"), *CleanLegacy);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Semantic Clean: '%s'"), *CleanSemantic);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Legacy Raw: '%s'"), *LegacyOutput);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Semantic Raw: '%s'"), *SemanticAsLegacy);
    }
    
    return bMatches;
}

// ===== FACTORY METHODS =====

FSemanticExecutionStep FSemanticDataHelper::CreateEventStep(const FString& EventName)
{
    FSemanticExecutionStep Step;
    Step.NodeType = ESemanticNodeType::Event;
    Step.NodeName = EventName;
    return Step;
}

FSemanticExecutionStep FSemanticDataHelper::CreateFunctionCallStep(
    const FString& FuncName, 
    const FString& TargetExpression,
    const TArray<FSemanticArgument>& Args)
{
    FSemanticExecutionStep Step;
    Step.NodeType = ESemanticNodeType::FunctionCall;
    Step.NodeName = FuncName;
    Step.TargetExpression = TargetExpression;
    Step.Arguments = Args;
    return Step;
}

FSemanticArgument FSemanticDataHelper::CreateLiteralArgument(
    const FString& ParamName, 
    const FString& LiteralValue, 
    const FString& DataType)
{
    return FSemanticArgument(ParamName, LiteralValue, ESemanticNodeType::Literal, DataType);
}


// ===== PRIVATE HELPERS =====
FString FSemanticDataHelper::FormatNodeTypeForLegacy(ESemanticNodeType NodeType)
{
    // ⚠️ CRITICAL: Must match exact FMarkdownSpan output
    switch (NodeType)
    {
        case ESemanticNodeType::Event: 
            return TEXT("**Event**");
        case ESemanticNodeType::FunctionCall: 
            return TEXT("**Call Function:**");
        case ESemanticNodeType::MacroCall: 
            return TEXT("**Macro:**");
        case ESemanticNodeType::CustomEventCall: 
            return TEXT("**Call Custom Event:**");
        case ESemanticNodeType::Sequence: 
            return TEXT("***Sequence***");
        case ESemanticNodeType::Branch: 
            return TEXT("**Branch**");
        case ESemanticNodeType::Loop: 
            return TEXT("***For Each***");
        case ESemanticNodeType::VariableSet: 
            return TEXT("**Set**");
        case ESemanticNodeType::Operator: 
            return TEXT(""); // Operators don't have type prefix
        case ESemanticNodeType::Conversion: 
            return TEXT(""); // Conversions show as function calls
        case ESemanticNodeType::Literal:
        case ESemanticNodeType::Variable:
        case ESemanticNodeType::PathEnd:
        case ESemanticNodeType::Error:
            return TEXT(""); // These don't have prefixes
        default: 
            return TEXT("**Unknown**");
    }
}


FString FSemanticDataHelper::FormatValueForLegacy(const FSemanticArgument& Arg)
{
    // ⚠️ CRITICAL: Must match exact FMarkdownSpan value formatting
    switch (Arg.ValueType)
    {
    case ESemanticNodeType::Literal:
        if (Arg.DataType.Equals(TEXT("string"), ESearchCase::IgnoreCase) ||
            Arg.DataType.Equals(TEXT("text"), ESearchCase::IgnoreCase))
        {
            return TEXT("'") + Arg.ValueRepresentation + TEXT("'");
        }
        else if (Arg.DataType.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
        {
            return Arg.ValueRepresentation.ToLower();
        }
        else
        {
            return Arg.ValueRepresentation; // Numbers, etc.
        }
            
    case ESemanticNodeType::Variable:
        return TEXT("`") + Arg.ValueRepresentation + TEXT("`");
            
    case ESemanticNodeType::FunctionCall:
        return Arg.ValueRepresentation; // Already formatted like "MyFunc()"
            
    case ESemanticNodeType::Operator:
        return TEXT("(") + Arg.ValueRepresentation + TEXT(")");
            
    case ESemanticNodeType::Error:
        return TEXT("[") + Arg.ValueRepresentation + TEXT("]");
            
    default:
        return Arg.ValueRepresentation;
    }
}

// ✅ FIXED: ConvertSemanticValueToString - Handle struct values correctly
FString FSemanticDataHelper::ConvertSemanticValueToString(const FSemanticValue& Value)
{
    switch (Value.NodeType)
    {
        case ESemanticNodeType::Literal:
            if (Value.DataType.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralBoolean(Value.ValueRepresentation);
            else if (Value.DataType.Equals(TEXT("int"), ESearchCase::IgnoreCase) || 
                     Value.DataType.Equals(TEXT("float"), ESearchCase::IgnoreCase) ||
                     Value.DataType.Equals(TEXT("double"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralNumber(Value.ValueRepresentation);
            else if (Value.DataType.Equals(TEXT("string"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralString(Value.ValueRepresentation);
            else if (Value.DataType.Equals(TEXT("text"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralText(Value.ValueRepresentation);
            else if (Value.DataType.Equals(TEXT("name"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralName(Value.ValueRepresentation);
            else if (Value.DataType.Equals(TEXT("object"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralObject(Value.ValueRepresentation);
            // ✅ FIX: For struct values, match legacy behavior exactly
            else if (Value.DataType.Equals(TEXT("struct"), ESearchCase::IgnoreCase))
            {
                // ✅ CRITICAL: ParseStructDefaultValue_Legacy doesn't use any FMarkdownSpan calls
                // It just returns the raw parsed content, so we should too for validation
                return Value.ValueRepresentation;
            }
            // ✅ FIX: For gameplaytag, match legacy behavior (no extra formatting)
            else if (Value.DataType.Equals(TEXT("gameplaytag"), ESearchCase::IgnoreCase))
            {
                // GameplayTag values from ParseStructDefaultValue_Legacy are returned as-is
                return Value.ValueRepresentation;
            }
            else if (Value.DataType.Equals(TEXT("array"), ESearchCase::IgnoreCase) ||
                     Value.DataType.Equals(TEXT("set"), ESearchCase::IgnoreCase) ||
                     Value.DataType.Equals(TEXT("map"), ESearchCase::IgnoreCase))
                return FMarkdownSpanSystem::LiteralContainer(Value.ValueRepresentation);
            else
                return FMarkdownSpanSystem::LiteralUnknown(Value.ValueRepresentation);
                
        case ESemanticNodeType::Variable:
            return FMarkdownSpanSystem::Variable(Value.ValueRepresentation);
            
        case ESemanticNodeType::Error:
            return FMarkdownSpanSystem::Error(Value.ValueRepresentation);
            
        default:
            return FMarkdownSpanSystem::Info(Value.ValueRepresentation);
    }

    
}


// ===== PHASE 4.1 VALUE CONVERSION FUNCTIONS =====

bool FSemanticDataHelper::ValidateValueConversion(
    const FString& LegacyOutput, 
    const FString& SemanticOutput,
    const FString& ContextInfo)
{
    FString CleanLegacy = CleanStringForComparison(LegacyOutput);
    FString CleanSemantic = CleanStringForComparison(SemanticOutput);
    
    bool bMatches = CleanLegacy.Equals(CleanSemantic, ESearchCase::IgnoreCase);
    
    if (!bMatches)
    {
        UE_LOG(LogBP2AI, Error, 
            TEXT("🚨 VALUE VALIDATION FAILED %s:"), *ContextInfo);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Legacy: '%s'"), *CleanLegacy);
        UE_LOG(LogBP2AI, Error, 
            TEXT("  Semantic: '%s'"), *CleanSemantic);
    }
    
    return bMatches;
}

FString FSemanticDataHelper::ConvertSemanticLiteralValueToString(const FSemanticValue& LiteralValue)
{
    // ✅ FORMATTER ADDS STYLING BASED ON DATA TYPE
    if (LiteralValue.DataType == TEXT("string"))
    {
        // Add quotes for string display - THIS IS WHERE FORMATTING HAPPENS
        return FMarkdownSpan::LiteralString(FString::Printf(TEXT("'%s'"), *LiteralValue.ValueRepresentation));
    }
    else if (LiteralValue.DataType == TEXT("text"))
    {
        // Add quotes for text display  
        return FMarkdownSpan::LiteralText(FString::Printf(TEXT("'%s'"), *LiteralValue.ValueRepresentation));
    }
    else if (LiteralValue.DataType == TEXT("container"))
    {
        // Container notation stays clean - no quotes
        return FMarkdownSpan::LiteralContainer(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("enum"))
    {
        // Split EnumType::EnumValue and style appropriately
        FString EnumContent = LiteralValue.ValueRepresentation;
        int32 SeparatorPos = -1;
        if (EnumContent.FindChar(TEXT(':'), SeparatorPos) && SeparatorPos > 0 && EnumContent[SeparatorPos-1] == TEXT(':'))
        {
            FString EnumType = EnumContent.Left(SeparatorPos - 1);
            FString EnumVal = EnumContent.Mid(SeparatorPos + 1);
            return FMarkdownSpan::EnumType(EnumType) + TEXT("::") + FMarkdownSpan::EnumValue(EnumVal);
        }
        return FMarkdownSpan::EnumValue(EnumContent); // Fallback
    }
    else if (LiteralValue.DataType == TEXT("bool"))
    {
        return FMarkdownSpan::LiteralBoolean(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("int") || LiteralValue.DataType == TEXT("float"))
    {
        return FMarkdownSpan::LiteralNumber(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("name"))
    {
        return FMarkdownSpan::LiteralName(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("object"))
    {
        return FMarkdownSpan::LiteralObject(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("gameplaytag"))
    {
        if (LiteralValue.ValueRepresentation.IsEmpty())
        {
            return FMarkdownSpan::LiteralTag(TEXT(""));
        }
        return FMarkdownSpan::LiteralTag(LiteralValue.ValueRepresentation);
    }
    else if (LiteralValue.DataType == TEXT("struct"))
    {
        // Parse StructType(content) format
        FString StructContent = LiteralValue.ValueRepresentation;
        int32 ParenPos = -1;
        if (StructContent.FindChar(TEXT('('), ParenPos))
        {
            FString StructType = StructContent.Left(ParenPos);
            FString StructValues = StructContent.Mid(ParenPos);
            return FMarkdownSpan::LiteralStructType(StructType) + FMarkdownSpan::LiteralStructVal(StructValues);
        }
        return FMarkdownSpan::LiteralStructVal(StructContent);
    }
    
    // Unknown type fallback
    return FMarkdownSpan::LiteralUnknown(LiteralValue.ValueRepresentation);
}

