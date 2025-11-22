/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownFormattingUtils.cpp

#include "MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownSpanSystem.h" // Crucial: FMarkdownSpan delegates to this
#include "MarkdownTracerUtils.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Internationalization/Regex.h"
#include "Misc/DefaultValueHelper.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/BP2AILog.h" // For UE_LOG categories like LogFormatter, LogPathTracer
#include "Trace/Utils/SemanticDataHelper.h"
#include "UObject/UObjectGlobals.h" // For FindObject
#include "UObject/EnumProperty.h"



// Define NAME_ constants manually
#ifndef NAME_Bool
#define NAME_Bool FName(TEXT("bool"))
#endif
#ifndef NAME_Byte
#define NAME_Byte FName(TEXT("byte"))
#endif
#ifndef NAME_Int
#define NAME_Int FName(TEXT("int"))
#endif
#ifndef NAME_Int64
#define NAME_Int64 FName(TEXT("int64"))
#endif
#ifndef NAME_Float
#define NAME_Float FName(TEXT("float"))
#endif
#ifndef NAME_Double
#define NAME_Double FName(TEXT("double"))
#endif
#ifndef NAME_Real
#define NAME_Real FName(TEXT("real"))
#endif
#ifndef NAME_String
#define NAME_String FName(TEXT("string"))
#endif
#ifndef NAME_Text
#define NAME_Text FName(TEXT("text"))
#endif
#ifndef NAME_Name
#define NAME_Name FName(TEXT("name"))
#endif
#ifndef NAME_Object
#define NAME_Object FName(TEXT("object"))
#endif
#ifndef NAME_Class
#define NAME_Class FName(TEXT("class"))
#endif
#ifndef NAME_Interface
#define NAME_Interface FName(TEXT("interface"))
#endif
#ifndef NAME_Struct
#define NAME_Struct FName(TEXT("struct"))
#endif
#ifndef NAME_Delegate
#define NAME_Delegate FName(TEXT("delegate"))
#endif

// FMarkdownSpan thin wrapper implementations
FString FMarkdownSpan::Create(const FString& CssClass, const FString& Text) { return FMarkdownSpanSystem::CreateSpanInternal(CssClass, Text); }
FString FMarkdownSpan::Keyword(const FString& Text) { return FMarkdownSpanSystem::Keyword(Text); }
FString FMarkdownSpan::Variable(const FString& Text) { return FMarkdownSpanSystem::Variable(Text); }
FString FMarkdownSpan::FunctionName(const FString& Text) { return FMarkdownSpanSystem::FunctionName(Text); }
FString FMarkdownSpan::EventName(const FString& Text) { return FMarkdownSpanSystem::EventName(Text); }
FString FMarkdownSpan::MacroName(const FString& Text) { return FMarkdownSpanSystem::MacroName(Text); }
FString FMarkdownSpan::DataType(const FString& Text) { return FMarkdownSpanSystem::DataType(Text); }
FString FMarkdownSpan::PinName(const FString& Text) { return FMarkdownSpanSystem::PinName(Text); }
FString FMarkdownSpan::ParamName(const FString& Text) { return FMarkdownSpanSystem::ParamName(Text); }
FString FMarkdownSpan::Operator(const FString& Text) { return FMarkdownSpanSystem::Operator(Text); }
FString FMarkdownSpan::LiteralString(const FString& Text) { return FMarkdownSpanSystem::LiteralString(Text); }
FString FMarkdownSpan::LiteralNumber(const FString& Text) { return FMarkdownSpanSystem::LiteralNumber(Text); }
FString FMarkdownSpan::LiteralBoolean(const FString& Text) { return FMarkdownSpanSystem::LiteralBoolean(Text); }
FString FMarkdownSpan::LiteralName(const FString& Text) { return FMarkdownSpanSystem::LiteralName(Text); }
FString FMarkdownSpan::LiteralObject(const FString& Text) { return FMarkdownSpanSystem::LiteralObject(Text); }
FString FMarkdownSpan::LiteralTag(const FString& Text) { return FMarkdownSpanSystem::LiteralTag(Text); }
FString FMarkdownSpan::LiteralContainer(const FString& Text) { return FMarkdownSpanSystem::LiteralContainer(Text); }
FString FMarkdownSpan::LiteralStructType(const FString& Text) { return FMarkdownSpanSystem::LiteralStructType(Text); }
FString FMarkdownSpan::LiteralStructVal(const FString& Text) { return FMarkdownSpanSystem::LiteralStructVal(Text); }
FString FMarkdownSpan::LiteralText(const FString& Text) { return FMarkdownSpanSystem::LiteralText(Text); }
FString FMarkdownSpan::LiteralUnknown(const FString& Text) { return FMarkdownSpanSystem::LiteralUnknown(Text); }
FString FMarkdownSpan::EnumType(const FString& Text) { return FMarkdownSpanSystem::EnumType(Text); }
FString FMarkdownSpan::EnumValue(const FString& Text) { return FMarkdownSpanSystem::EnumValue(Text); }
FString FMarkdownSpan::ClassName(const FString& Text) { return FMarkdownSpanSystem::ClassName(Text); }
FString FMarkdownSpan::ComponentName(const FString& Text) { return FMarkdownSpanSystem::ComponentName(Text); }
FString FMarkdownSpan::WidgetName(const FString& Text) { return FMarkdownSpanSystem::WidgetName(Text); }
FString FMarkdownSpan::DelegateName(const FString& Text) { return FMarkdownSpanSystem::DelegateName(Text); }
FString FMarkdownSpan::TimelineName(const FString& Text) { return FMarkdownSpanSystem::TimelineName(Text); }
FString FMarkdownSpan::Modifier(const FString& Text) { return FMarkdownSpanSystem::Modifier(Text); }
FString FMarkdownSpan::Info(const FString& Text) { return FMarkdownSpanSystem::Info(Text); }
FString FMarkdownSpan::Error(const FString& Text) { return FMarkdownSpanSystem::Error(Text); }
FString FMarkdownSpan::GraphName(const FString& Text) { return FMarkdownSpanSystem::GraphName(Text); }
FString FMarkdownSpan::MontageName(const FString& Text) { return FMarkdownSpanSystem::MontageName(Text); }
FString FMarkdownSpan::ActionName(const FString& Text) { return FMarkdownSpanSystem::ActionName(Text); }
FString FMarkdownSpan::NodeTitle(const FString& Text) { return FMarkdownSpanSystem::NodeTitle(Text); }

// MarkdownFormattingUtils namespace implementation
namespace MarkdownFormattingUtils
{
    namespace
    {
        FString TruncateFloatString(const FString& InStr)
        {
            if (InStr.IsEmpty()) return InStr;
            double Value = FCString::Atod(*InStr);
            // Round to 3 decimal places
            double RoundedValue = FMath::RoundToDouble(Value * 1000.0) / 1000.0;
            // Format with %g to remove trailing zeros and unnecessary decimal points
            FString Result = FString::Printf(TEXT("%g"), RoundedValue);
            return Result;
        }
    }


    
    FString FormatOperator(
        TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, 
        int32 Depth, TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext)
    {
        check(Node.IsValid() && OutputPin.IsValid() && Tracer);
        UE_LOG(LogFormatter, Error, TEXT("FormatOperator (Utils) RECV: Node=%s (%s). Context: '%s'. CallingNode=%s (%s), OuterMap=%p"),
           *Node->Name,
           *Node->Guid.Left(8),
           *CurrentBlueprintContext,
           CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL_NODE_NAME"),
           CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID"),
           OuterNodesMap);
        
        const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
        const FString FuncName = FuncNamePtr ? **FuncNamePtr : Node->NodeType;
        FString BaseOperation = FuncName;
        if (FuncName.Contains(TEXT("_"))) { BaseOperation = FuncName.Left(FuncName.Find(TEXT("_"))); }

        const TMap<FString, FString>& OpMap = Tracer->GetMathOperatorMap();
        const FString* OpSymbol = OpMap.Find(BaseOperation);

        TArray<TSharedPtr<const FBlueprintPin>> InputPins;
        for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && !Pair.Value->IsHidden()) { InputPins.Add(Pair.Value); } }
        InputPins.Sort([](const auto& PinA, const auto& PinB) { return PinA->Name < PinB->Name; });

        UE_LOG(LogFormatter, Error, TEXT("🔍 DEBUG OPERATOR: FuncName='%s', BaseOperation='%s'"), *FuncName, *BaseOperation);
        UE_LOG(LogFormatter, Error, TEXT("🔍 DEBUG OPERATOR: OpSymbol found? %s"), OpSymbol ? TEXT("YES") : TEXT("NO"));
        if (OpSymbol) {
            UE_LOG(LogFormatter, Error, TEXT("🔍 DEBUG OPERATOR: OpSymbol='%s'"), **OpSymbol);
        }

        
        TArray<FString> InputValues;
        for(const auto& Pin : InputPins) {
            UE_LOG(LogFormatter, Log, TEXT("  FormatOperator (Utils): Tracing operand pin '%s'. Context to pass to RVR: %s"), *Pin->Name, *CurrentBlueprintContext);
            InputValues.Add(Tracer->ResolvePinValueRecursive(Pin, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext));
        }

        if (BaseOperation == TEXT("Concat"))
        {
            const FString& PlusSymbol = OpMap.FindRef(TEXT("Add"));
            FString PlusOperator = FMarkdownSpan::Operator(PlusSymbol.IsEmpty() ? TEXT("+") : PlusSymbol);
            FString JoinedValues = FString::Join(InputValues, *FString::Printf(TEXT(" %s "), *PlusOperator));
            return (InputValues.Num() > 1) ? FString::Printf(TEXT("(%s)"), *JoinedValues) : (InputValues.Num() == 1 ? InputValues[0] : FString(TEXT("")));
        }
        else if (BaseOperation == TEXT("Percent") || FuncName == TEXT("Percent_FloatFloat") || FuncName == TEXT("Percent_IntInt"))
        {
             const FString* PercentSymbolPtr = OpMap.Find(TEXT("Percent"));
             if (PercentSymbolPtr && InputValues.Num() == 2) {
                 return FString::Printf(TEXT("(%s %s %s)"), *InputValues[0], *FMarkdownSpan::Operator(*PercentSymbolPtr), *InputValues[1]);
             }
             UE_LOG(LogFormatter, Warning, TEXT("    FormatOperator: Fallback format for Percent function %s"), *FuncName);
             return FMarkdownSpan::FunctionName(TEXT("Percent")) + FString::Printf(TEXT("(%s)"), *FString::Join(InputValues, TEXT(", ")));
        }
        else if (OpSymbol && InputValues.Num() == 2)
        {
            return FString::Printf(TEXT("(%s %s %s)"), *InputValues[0], *FMarkdownSpan::Operator(*OpSymbol), *InputValues[1]);
        }

        UE_LOG(LogFormatter, Log, TEXT("    FormatOperator: Fallback format for '%s' (Base: '%s')"), *FuncName, *BaseOperation);
        return FMarkdownSpan::FunctionName(BaseOperation) + FString::Printf(TEXT("(%s)"), *FString::Join(InputValues, TEXT(", ")));
    }

    FString FormatUnaryOperator(
        TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, 
        int32 Depth, TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext)
    {
        check(Node.IsValid() && OutputPin.IsValid() && Tracer);
        UE_LOG(LogFormatter, Verbose, TEXT("    FormatUnaryOperator (Utils): Formatting node %s. Context: %s"), *Node->Guid, *CurrentBlueprintContext);

        const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
        const FString FuncName = FuncNamePtr ? **FuncNamePtr : Node->NodeType;

        TSharedPtr<const FBlueprintPin> InputPin;
        for(const auto& Pair : Node->Pins) {
            if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && !Pair.Value->IsHidden()){
                InputPin = Pair.Value;
                break;
            }
        }
        UE_LOG(LogFormatter, Log, TEXT("  FormatUnaryOperator (Utils): Tracing input pin '%s'. Context to pass to RVR: %s"), InputPin.IsValid() ? *InputPin->Name : TEXT("INVALID"), *CurrentBlueprintContext);
        FString InputValue = InputPin.IsValid() ? Tracer->ResolvePinValueRecursive(InputPin, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("[Input Missing]"));
        
        if (FuncName == TEXT("Not_PreBool"))
        {
            const FString* OpSymbol = Tracer->GetMathOperatorMap().Find(TEXT("Not")); // ✅ CORRECT KEY!
            if (OpSymbol)
            {
                return FString::Printf(TEXT("%s (%s)"), *FMarkdownSpan::Operator(*OpSymbol), *InputValue);
            }
        }

        FString BaseOperation = FuncName;
        if (FuncName.Contains(TEXT("_"))) { BaseOperation = FuncName.Left(FuncName.Find(TEXT("_"))); }
        return FMarkdownSpan::FunctionName(BaseOperation) + FString::Printf(TEXT("(%s)"), *InputValue);
    }

    FString FormatConversion(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, 
        int32 Depth,
        TSet<FString>& VisitedPins,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext)
    {
        check(Node.IsValid() && Tracer);
        UE_LOG(LogFormatter, Warning, TEXT("MarkdownFormattingUtils::FormatConversion called for %s. Context: '%s' - Ensure this is intended, primary logic moved to DataTracer handlers."), *Node->Guid, *CurrentBlueprintContext);
        
        const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
        const FString FuncName = FuncNamePtr ? **FuncNamePtr : TEXT("UnknownConv");
        
        FString NormalizedFuncName = MarkdownTracerUtils::NormalizeConversionName(FuncName, Tracer->GetTypeConversionMap());
        if(NormalizedFuncName.IsEmpty()) NormalizedFuncName = FuncName; 

        TSharedPtr<const FBlueprintPin> InputPin;
        for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && !Pair.Value->IsHidden()){ InputPin = Pair.Value; break; } }
        
        UE_LOG(LogFormatter, Log, TEXT("  FormatConversion (Utils): Tracing input pin '%s'. Context to pass to RVR: %s"), InputPin.IsValid() ? *InputPin->Name : TEXT("INVALID"), *CurrentBlueprintContext);
        FString InputValue = InputPin.IsValid() ? Tracer->ResolvePinValueRecursive(InputPin, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("[Input Missing]"));
        
        return FMarkdownSpan::FunctionName(NormalizedFuncName) + FString::Printf(TEXT("(%s)"), *InputValue);
    }
FString FormatDefaultValue(
    TSharedPtr<const FBlueprintPin> Pin,
    const FMarkdownDataTracer* Tracer)
{
    if (!Pin.IsValid()) { 
        UE_LOG(LogFormatter, Warning, TEXT("FormatDefaultValue: Invalid Pin!")); 
        return FMarkdownSpan::Error(TEXT("[Invalid Pin]")); 
    }
    check(Tracer);
    
    UE_LOG(LogFormatter, Verbose, TEXT("FormatDefaultValue: Pin=%s (%s)"), *Pin->Name, *Pin->Id);

    // Handle non-empty default values first
    if (!Pin->DefaultValue.IsEmpty()) { 
        return FormatLiteralValue(Pin, Pin->DefaultValue, Tracer); 
    }
    
    if (!Pin->DefaultObject.IsEmpty() && 
        !Pin->DefaultObject.Equals(TEXT("None"), ESearchCase::IgnoreCase) && 
        !Pin->DefaultObject.Equals(TEXT("NULL"), ESearchCase::IgnoreCase)) {
        return FormatLiteralValue(Pin, Pin->DefaultObject, Tracer); 
    }

    // 🔧 ENHANCED: Handle struct pins with extracted default values
    if (Pin->DefaultStruct.Num() > 0) {
        // Get the struct type name from the pin
        FString StructTypeName = TEXT("Struct");
        if (!Pin->SubCategoryObject.IsEmpty()) {
            StructTypeName = Pin->SubCategoryObject;
            // Extract clean struct name from path
            int32 LastDotIndex;
            if (StructTypeName.FindLastChar(TEXT('.'), LastDotIndex)) {
                StructTypeName = StructTypeName.Mid(LastDotIndex + 1);
            }
        }

        // Format the struct members
        TArray<FString> MembersList;
        for (const auto& Pair : Pin->DefaultStruct) {
            FString Key = Pair.Key.TrimStartAndEnd();
            FString Value = Pair.Value.TrimStartAndEnd().TrimQuotes();

            // Format as ParamName=Value with proper formatting
            MembersList.Add(FString::Printf(TEXT("%s=%s"), 
                *FMarkdownSpan::ParamName(Key),
                *FMarkdownSpan::LiteralString(FString::Printf(TEXT("\"%s\""), *Value))));
        }

        if (MembersList.Num() > 0) {
            FString MembersStr = FString::Join(MembersList, TEXT(", "));
            return FString::Printf(TEXT("%s(%s)"), 
                *FMarkdownSpan::DataType(StructTypeName), 
                *MembersStr);
        }

        // Fallback if no members formatted properly
        return FString::Printf(TEXT("%s(...)"), *FMarkdownSpan::DataType(StructTypeName));
    }

    // 🔧 NEW: Handle Blueprint variable defaults for struct pins
    const FString* BlueprintVariableDefault = Pin->RawProperties.Find(TEXT("BlueprintVariableDefault"));
    if (Pin->Category == TEXT("struct") && BlueprintVariableDefault && !BlueprintVariableDefault->IsEmpty()) {
        // Extract struct type name
        FString StructTypeName = TEXT("Struct");
        if (!Pin->SubCategoryObject.IsEmpty()) {
            StructTypeName = Pin->SubCategoryObject;
            int32 LastDotIndex;
            if (StructTypeName.FindLastChar(TEXT('.'), LastDotIndex)) {
                StructTypeName = StructTypeName.Mid(LastDotIndex + 1);
            }
        }

        // Parse the Blueprint variable default string
        FString DefaultStr = BlueprintVariableDefault->TrimStartAndEnd();
        if (DefaultStr.StartsWith(TEXT("(")) && DefaultStr.EndsWith(TEXT(")"))) {
            // Remove outer parentheses
            FString Content = DefaultStr.Mid(1, DefaultStr.Len() - 2).TrimStartAndEnd();

            if (!Content.IsEmpty()) {
                // Parse key=value pairs
                TArray<FString> Parts;
                Content.ParseIntoArray(Parts, TEXT(","));

                TArray<FString> MembersList;
                bool bValidParsing = true;

                for (const FString& Part : Parts) {
                    FString Key, Value;
                    if (Part.Split(TEXT("="), &Key, &Value)) {
                        Key = Key.TrimStartAndEnd();
                        Value = Value.TrimStartAndEnd();

                        // Clean up the value (remove quotes if present)
                        if (Value.StartsWith(TEXT("\"")) && Value.EndsWith(TEXT("\""))) {
                            Value = Value.Mid(1, Value.Len() - 2);
                        }

                        MembersList.Add(FString::Printf(TEXT("%s=%s"), 
                            *FMarkdownSpan::ParamName(Key),
                            *FMarkdownSpan::LiteralString(FString::Printf(TEXT("\"%s\""), *Value))));
                    } else {
                        bValidParsing = false;
                        break;
                    }
                }

                if (bValidParsing && MembersList.Num() > 0) {
                    FString MembersStr = FString::Join(MembersList, TEXT(", "));
                    return FString::Printf(TEXT("%s(%s)"), 
                        *FMarkdownSpan::DataType(StructTypeName), 
                        *MembersStr);
                }
            }
        }

        // Fallback: show struct type with ellipsis
        return FString::Printf(TEXT("%s(...)"), *FMarkdownSpan::DataType(StructTypeName));
    }

    // Handle special input pins
    if (Pin->IsInput() && (Pin->Name == TEXT("self") || Pin->Name == TEXT("Target") || Pin->Name == TEXT("WorldContextObject"))) {
        return FMarkdownSpan::Variable(TEXT("self"));
    }

    // Handle primitive types
    const FName Category = FName(*Pin->Category);
    const FName Container = FName(*Pin->ContainerType);

    if (Category == NAME_Bool) return FMarkdownSpan::LiteralBoolean(TEXT("false"));
    if (Category == NAME_Byte || Category == NAME_Int || Category == NAME_Int64) return FMarkdownSpan::LiteralNumber(TEXT("0"));
    if (Category == NAME_Real || Category == NAME_Float || Category == NAME_Double) return FMarkdownSpan::LiteralNumber(TEXT("0.0"));
    if (Category == NAME_String) return FMarkdownSpan::LiteralString(TEXT("''"));
    if (Category == NAME_Text) return FMarkdownSpan::LiteralText(TEXT("''"));
    if (Category == NAME_Name) return FMarkdownSpan::LiteralName(TEXT("None"));
    if (Category == NAME_Object || Category == NAME_Class || Category == NAME_Interface || 
        Category == FName(TEXT("asset")) || Category == FName(TEXT("assetclass")) || 
        Category == FName(TEXT("softobject")) || Category == FName(TEXT("softclass"))) { 
        return FMarkdownSpan::LiteralObject(TEXT("`None`")); 
    }

    // Handle container types
    if (Container == FName(TEXT("Array")) || Container == FName(TEXT("Set")) || Container == FName(TEXT("Map"))) {
        if (Container == FName(TEXT("Array"))) return FMarkdownSpan::LiteralContainer(TEXT("[]"));
        if (Container == FName(TEXT("Set"))) return FMarkdownSpan::LiteralContainer(TEXT("{}"));
        if (Container == FName(TEXT("Map"))) return FMarkdownSpan::LiteralContainer(TEXT("{}"));
    }

    // 🔧 ENHANCED: Handle struct types with proper type name
    if (Category == NAME_Struct) {
        FString StructTypeName = TEXT("Struct");
        if (!Pin->SubCategoryObject.IsEmpty()) {
            StructTypeName = Pin->SubCategoryObject;
            int32 LastDotIndex;
            if (StructTypeName.FindLastChar(TEXT('.'), LastDotIndex)) {
                StructTypeName = StructTypeName.Mid(LastDotIndex + 1);
            }
        }
        return FString::Printf(TEXT("%s(...)"), *FMarkdownSpan::DataType(StructTypeName));
    }

    return FMarkdownSpan::Info(TEXT("(No Default)"));
}
    // ===== SEMANTIC APPROACH COMMENTED OUT =====
    // TODO: Re-enable semantic approach when ready for migration
    /*
    // ===== PHASE 4.1.2: FormatLiteralValue Semantic Version =====
    FSemanticValue FormatLiteralValue_Semantic(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueString,
        const FMarkdownDataTracer* Tracer)
    {
        check(Pin.IsValid());
        check(Tracer);
        
        const FName Category = FName(*Pin->Category);
        const FName Container = FName(*Pin->ContainerType);
        FString OriginalValue = ValueString; 
        FString CleanValue = ValueString.TrimStartAndEnd();
        
        // ✅ FIX 1: ADD MISSING CONTAINER LOGIC (must come BEFORE category checks)
        if (Container == FName(TEXT("Array")) || Container == FName(TEXT("Set")) || Container == FName(TEXT("Map")))
        {
            // Clean container notation - NO formatting added here
            if (CleanValue.IsEmpty() || CleanValue == TEXT("()") || CleanValue == TEXT("{}") || CleanValue == TEXT("[]"))
            {
                if (Container == FName(TEXT("Array"))) {
                    return FSemanticValue(TEXT("[]"), ESemanticNodeType::Literal, TEXT("container"));
                } else if (Container == FName(TEXT("Set")) || Container == FName(TEXT("Map"))) {
                    return FSemanticValue(TEXT("{}"), ESemanticNodeType::Literal, TEXT("container"));
                }
            }
            // Non-empty containers - return as-is, let formatter handle styling
            return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("container"));
        }
        
        // Handle quotes for strings and names
        if (Category == NAME_String || Category == NAME_Text || Category == NAME_Name)
        {
            if (CleanValue.Len() >= 2 && CleanValue.StartsWith(TEXT("\"")) && CleanValue.EndsWith(TEXT("\"")))
            {
                CleanValue = CleanValue.Mid(1, CleanValue.Len() - 2);
            }
            else if (CleanValue.Len() >= 2 && CleanValue.StartsWith(TEXT("'")) && CleanValue.EndsWith(TEXT("'")))
            {
                if (!CleanValue.Contains(TEXT("/"), ESearchCase::CaseSensitive) && !CleanValue.Contains(TEXT("."), ESearchCase::CaseSensitive))
                {
                    CleanValue = CleanValue.Mid(1, CleanValue.Len() - 2);
                }
            }
        }
        
        // ✅ FIX 2: BETTER ENUM DETECTION (check actual UE path patterns)
        if (!Pin->SubCategoryObject.IsEmpty())
        {
            // Real UE enum paths: /Script/Engine.ECollisionEnabled, /Game/MyProject.EMyEnum
            FString SubCatPath = Pin->SubCategoryObject;
            if (SubCatPath.StartsWith(TEXT("/Script/")) || SubCatPath.StartsWith(TEXT("/Game/")))
            {
                // Extract the final component after the dot
                int32 LastDotIndex = -1;
                if (SubCatPath.FindLastChar(TEXT('.'), LastDotIndex) && LastDotIndex < SubCatPath.Len() - 1)
                {
                    FString TypeName = SubCatPath.Mid(LastDotIndex + 1);
                    // UE enum naming convention: starts with E and is CamelCase
                    if (TypeName.StartsWith(TEXT("E")) && TypeName.Len() > 1 && FChar::IsUpper(TypeName[1]))
                    {
                        FString EnumTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
                        if (EnumTypeName.IsEmpty()) EnumTypeName = TypeName;
                        
                        FString EnumValueStr = CleanValue;
                        if (EnumValueStr.IsEmpty() || EnumValueStr.Equals(TEXT("None"), ESearchCase::IgnoreCase))
                        {
                            EnumValueStr = TEXT("[Default Value]"); 
                        }
                        
                        // ✅ CLEAN ENUM FORMAT - NO STYLING
                        FString EnumContent = EnumTypeName + TEXT("::") + EnumValueStr;
                        return FSemanticValue(EnumContent, ESemanticNodeType::Literal, TEXT("enum"));
                    }
                }
            }
        }

        // Handle by category
        if (Category == NAME_Bool)
        {
            return FSemanticValue(CleanValue.ToLower(), ESemanticNodeType::Literal, TEXT("bool"));
        }
        
        if (Category == NAME_Byte || Category == NAME_Int || Category == FName(TEXT("int64")))
        {
            int64 IntVal;
            if (FDefaultValueHelper::ParseInt64(CleanValue, IntVal))
            {
                return FSemanticValue(FString::Printf(TEXT("%lld"), IntVal), ESemanticNodeType::Literal, TEXT("int"));
            }
            return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("int"));
        }
        
        if (Category == NAME_Real || Category == NAME_Float || Category == FName(TEXT("double")))
        {
            TOptional<double> NumVal = FCString::Atod(*CleanValue);
            if (NumVal.IsSet())
            {
                double Value = NumVal.GetValue();
                return FSemanticValue(FString::Printf(TEXT("%g"), Value), ESemanticNodeType::Literal, TEXT("float"));
            }
            return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("float"));
        }
        
        // ✅ FIX 3: ARCHITECTURE VIOLATION FIXED - NO QUOTES ADDED HERE
        if (Category == NAME_String)
        {
            // Return CLEAN string content - formatter will add quotes
            return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("string"));
        }
        
        if (Category == NAME_Text)
        {
            // Return CLEAN text content - formatter will add quotes  
            return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("text"));
        }
        
        if (Category == NAME_Name)
        {
            FString NameValue = CleanValue.ToLower() == TEXT("none") ? TEXT("None") : CleanValue;
            return FSemanticValue(NameValue, ESemanticNodeType::Literal, TEXT("name"));
        }
        
        if (Category == NAME_Object || Category == NAME_Class || Category == NAME_Interface || 
            Category == FName(TEXT("asset")) || Category == FName(TEXT("assetclass")) || 
            Category == FName(TEXT("softobject")) || Category == FName(TEXT("softclass")))
        {
            FString PathToSimplify = OriginalValue.TrimQuotes();
            FString SimpleName = MarkdownTracerUtils::ExtractSimpleNameFromPath(PathToSimplify);
            if (SimpleName.IsEmpty() || SimpleName.ToLower() == TEXT("none"))
            {
                return FSemanticValue(TEXT("None"), ESemanticNodeType::Literal, TEXT("object"));
            }
            return FSemanticValue(SimpleName, ESemanticNodeType::Literal, TEXT("object"));
        }
        
        if (Category == NAME_Struct)
        {
            FString StructTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
            if (StructTypeName.IsEmpty()) StructTypeName = TEXT("Struct");
            
            FSemanticValue StructResult = ParseStructDefaultValue_Semantic(OriginalValue);
            
            if (StructTypeName == TEXT("GameplayTag"))
            {
                if (!StructResult.ValueRepresentation.IsEmpty() && StructResult.ValueRepresentation.ToLower() != TEXT("none") && 
                    StructResult.ValueRepresentation != TEXT("(...)") && StructResult.ValueRepresentation != TEXT("()"))
                {
                    return FSemanticValue(StructResult.ValueRepresentation, ESemanticNodeType::Literal, TEXT("gameplaytag"));
                }
                else
                {
                    return FSemanticValue(TEXT(""), ESemanticNodeType::Literal, TEXT("gameplaytag"));
                }
            }
            
            if (StructTypeName == TEXT("GameplayTagContainer"))
            {
                // Complex GameplayTagContainer handling
                FString TagsContent = StructResult.ValueRepresentation.TrimStartAndEnd().TrimChar(TEXT('(')).TrimChar(TEXT(')')).TrimStartAndEnd();
                TArray<FString> Tags;
                TagsContent.ParseIntoArray(Tags, TEXT(","), true);
                TArray<FString> CleanTags;
                for (const FString& Tag : Tags)
                {
                    FString CleanTag = Tag.TrimStartAndEnd().TrimQuotes();
                    if (!CleanTag.IsEmpty() && CleanTag.ToLower() != TEXT("none"))
                    {
                        CleanTags.Add(CleanTag);
                    }
                }
                FString ContainerContent = FString::Printf(TEXT("GameplayTagContainer({%s})"), *FString::Join(CleanTags, TEXT(", ")));
                return FSemanticValue(ContainerContent, ESemanticNodeType::Literal, TEXT("struct"));
            }
            
            // Regular struct
            FString StructContent = StructTypeName + StructResult.ValueRepresentation;
            return FSemanticValue(StructContent, ESemanticNodeType::Literal, TEXT("struct"));
        }

        // Final fallback
        return FSemanticValue(CleanValue, ESemanticNodeType::Literal, TEXT("unknown"));
    }
    */



FString MarkdownFormattingUtils::FormatLiteralValue_Legacy(
    TSharedPtr<const FBlueprintPin> Pin,
    const FString& ValueString,
    const FMarkdownDataTracer* Tracer)
{
    check(Pin.IsValid());
    check(Tracer);
    const FName Category = FName(*Pin->Category);
    
    FString StructTypeName = (Category == NAME_Struct) ? MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject) : TEXT("");
    if(Category == NAME_Struct && StructTypeName.IsEmpty()) StructTypeName = TEXT("Struct");

    FString OriginalValue = ValueString; 
    FString CleanValue = ValueString.TrimStartAndEnd();

    if (Category == NAME_String || Category == NAME_Text || Category == NAME_Name)
    {
        if (CleanValue.Len() >= 2 && CleanValue.StartsWith(TEXT("\"")) && CleanValue.EndsWith(TEXT("\""))) {
            CleanValue = CleanValue.Mid(1, CleanValue.Len() - 2);
        }
        else if (CleanValue.Len() >= 2 && CleanValue.StartsWith(TEXT("'")) && CleanValue.EndsWith(TEXT("'"))) {
            if (!CleanValue.Contains(TEXT("/"), ESearchCase::CaseSensitive) && !CleanValue.Contains(TEXT("."), ESearchCase::CaseSensitive))
            {
                CleanValue = CleanValue.Mid(1, CleanValue.Len() - 2);
            }
        }
    }
    
    // ? --- CORRECTED ENUM DETECTION LOGIC ---
    bool bIsEnumPin = false;
    if (!Pin->SubCategoryObject.IsEmpty())
    {
        // Use reflection to robustly check if the SubCategoryObject is actually a UEnum.
        UObject* EnumObject = FindObject<UObject>(nullptr, *Pin->SubCategoryObject);
        if (EnumObject && EnumObject->IsA(UEnum::StaticClass()))
        {
            bIsEnumPin = true;
        }
    }
    
    if (bIsEnumPin) {
    // --- END CORRECTION ---
        FString EnumType = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
        if (EnumType.IsEmpty()) EnumType = TEXT("Enum?");
        
        FString EnumValueStr = CleanValue;
        if (EnumValueStr.IsEmpty() || EnumValueStr.Equals(TEXT("None"), ESearchCase::IgnoreCase)) {
            EnumValueStr = TEXT("[Default Value]"); 
        }
        else
        {
            EnumValueStr = ConvertEnumInternalToDisplay(EnumValueStr, Pin->SubCategoryObject);
        }
    
        return FMarkdownSpan::EnumType(EnumType) + TEXT("::") + FMarkdownSpan::EnumValue(EnumValueStr);
    }
    
    if (Category == NAME_Bool) return FMarkdownSpan::LiteralBoolean(CleanValue.ToLower());
    
    if (Category == NAME_Byte || Category == NAME_Int || Category == FName(TEXT("int64")) || Category == NAME_Real || Category == NAME_Float || Category == FName(TEXT("double"))) {
        return FMarkdownSpan::LiteralNumber(TruncateFloatString(CleanValue));
    }
    
    if (Category == NAME_String) return FMarkdownSpan::LiteralString(CleanValue);
    if (Category == NAME_Text) return FMarkdownSpan::LiteralText(CleanValue);
    if (Category == NAME_Name) return FMarkdownSpan::LiteralName(CleanValue.ToLower() == TEXT("none") ? TEXT("None") : CleanValue);
    
    if (Category == NAME_Object || Category == NAME_Class || Category == NAME_Interface || 
        Category == FName(TEXT("asset")) || Category == FName(TEXT("assetclass")) || 
        Category == FName(TEXT("softobject")) || Category == FName(TEXT("softclass"))) {
        
        FString SimpleName = MarkdownTracerUtils::ExtractSimpleNameFromPath(OriginalValue.TrimQuotes());
        return FMarkdownSpan::LiteralObject(SimpleName.IsEmpty() || SimpleName.ToLower() == TEXT("none") ? TEXT("None") : SimpleName);
    }
    
    if (Category == NAME_Struct) {
        FString FormattedContent = ParseStructDefaultValue(OriginalValue, StructTypeName);
        
        if (StructTypeName == TEXT("GameplayTag")) {
            return FMarkdownSpan::LiteralTag(FormattedContent.IsEmpty() || FormattedContent.ToLower() == TEXT("none") || FormattedContent == TEXT("(...)") || FormattedContent == TEXT("()") ? TEXT("") : FormattedContent);
        }
        
        if (StructTypeName == TEXT("GameplayTagContainer")) {
            FString TagsContent = FormattedContent.TrimStartAndEnd().TrimChar(TEXT('(')).TrimChar(TEXT(')')).TrimStartAndEnd();
            TArray<FString> Tags;
            TagsContent.ParseIntoArray(Tags, TEXT(","), true); 
            TArray<FString> FormattedTags;
            for(const FString& Tag : Tags) {
                FString CleanTag = Tag.TrimStartAndEnd().TrimQuotes();
                if (!CleanTag.IsEmpty() && CleanTag.ToLower() != TEXT("none")) {
                    FormattedTags.Add(FMarkdownSpan::LiteralTag(CleanTag));
                }
            }
            return FMarkdownSpan::LiteralStructType(TEXT("GameplayTagContainer")) + 
                   FString::Printf(TEXT("({%s})"), *FString::Join(FormattedTags, TEXT(", ")));
        }
        
        return FMarkdownSpan::LiteralStructType(StructTypeName) + FMarkdownSpan::LiteralStructVal(FormattedContent);
    }

    return FMarkdownSpan::LiteralUnknown(CleanValue);
}
    // ===== PHASE 4.1.2: Main Function - Now Legacy Only ====
    FString FormatLiteralValue(
        TSharedPtr<const FBlueprintPin> Pin,
        const FString& ValueString,
        const FMarkdownDataTracer* Tracer)
    {
        // 🔴 SEMANTIC APPROACH DISABLED - Using legacy only
        return FormatLiteralValue_Legacy(Pin, ValueString, Tracer);
        
        /* COMMENTED OUT: Semantic validation approach
        FSemanticValue semanticResult = FormatLiteralValue_Semantic(Pin, ValueString, Tracer);
        FString semanticOutput = FSemanticDataHelper::ConvertSemanticLiteralValueToString(semanticResult);
        FString legacyOutput = FormatLiteralValue_Legacy(Pin, ValueString, Tracer);

        // Clean both outputs for comparison (remove formatting)
        FString cleanSemantic = semanticOutput.Replace(TEXT("`"), TEXT("")).Replace(TEXT("**"), TEXT("")).TrimStartAndEnd();
        FString cleanLegacy = legacyOutput.Replace(TEXT("`"), TEXT("")).Replace(TEXT("**"), TEXT("")).TrimStartAndEnd();

        bool bMatches = cleanSemantic.Equals(cleanLegacy, ESearchCase::IgnoreCase);

        if (!bMatches)
        {
            UE_LOG(LogFormatter, Error, TEXT("🚨 PHASE 4.1.2 VALIDATION FAILED for input '%s':"), *ValueString);
            UE_LOG(LogFormatter, Error, TEXT("  Legacy: %s"), *legacyOutput);
            UE_LOG(LogFormatter, Error, TEXT("  Semantic: %s"), *semanticOutput);
            UE_LOG(LogFormatter, Error, TEXT("  Clean Legacy: %s"), *cleanLegacy);
            UE_LOG(LogFormatter, Error, TEXT("  Clean Semantic: %s"), *cleanSemantic);
        
            // Return legacy on validation failure (safe fallback)
            return legacyOutput;
        }

        // Validation passed - return semantic output
        return semanticOutput;
        */
    }

FString MarkdownFormattingUtils::ParseStructDefaultValue_Legacy(const FString& ValueString, const FString& StructTypeName)
{
    FString Content = ValueString.TrimStartAndEnd();

    // Handle empty cases
    if (Content.IsEmpty() || Content == TEXT("()") || Content == TEXT("{}")) { return TEXT("()"); }

    // --- NEW: Type-aware parsing for Vector, Rotator, Transform ---
    if (StructTypeName == TEXT("Vector") || StructTypeName == TEXT("Transform") || StructTypeName == TEXT("Vector2D"))
    {
        FString CleanContent = Content.TrimChar(TEXT('(')).TrimChar(TEXT(')'));
        TArray<FString> Components;
        CleanContent.ParseIntoArray(Components, TEXT(","));
        
        TArray<FString> Labels = {TEXT("X"), TEXT("Y"), TEXT("Z")};
        if (StructTypeName == TEXT("Transform")) {
            Labels = {TEXT("Location.X"), TEXT("Location.Y"), TEXT("Location.Z"), TEXT("Rotation.X"), TEXT("Rotation.Y"), TEXT("Rotation.Z"), TEXT("Scale.X"), TEXT("Scale.Y"), TEXT("Scale.Z")};
        } else if (StructTypeName == TEXT("Vector2D")) {
            Labels = {TEXT("X"), TEXT("Y")};
        }

        FString Result;
        for (int32 i = 0; i < Components.Num() && i < Labels.Num(); ++i)
        {
            FString Key, Value;
            if (Components[i].Split(TEXT("="), &Key, &Value))
            {
                Result += FString::Printf(TEXT("%s=%s, "), *Key.TrimStartAndEnd(), *TruncateFloatString(Value.TrimStartAndEnd()));
            }
            else // Handle raw value case like "1.0,2.0,3.0"
            {
                Result += FString::Printf(TEXT("%s=%s, "), *Labels[i], *TruncateFloatString(Components[i].TrimStartAndEnd()));
            }
        }
        if (!Result.IsEmpty())
        {
            Result.LeftChopInline(2); // Remove trailing ", "
            return FString::Printf(TEXT("(%s)"), *Result);
        }
    }
    else if (StructTypeName == TEXT("Rotator"))
    {
        FString CleanContent = Content.TrimChar(TEXT('(')).TrimChar(TEXT(')'));
        TArray<FString> Components;
        CleanContent.ParseIntoArray(Components, TEXT(","));

        TArray<FString> Labels = {TEXT("P"), TEXT("Y"), TEXT("R")}; // Pitch, Yaw, Roll
        FString Result;
        for (int32 i = 0; i < Components.Num() && i < Labels.Num(); ++i)
        {
            FString Key, Value;
            if (Components[i].Split(TEXT("="), &Key, &Value))
            {
                 Result += FString::Printf(TEXT("%s=%s, "), *Key.TrimStartAndEnd(), *TruncateFloatString(Value.TrimStartAndEnd()));
            }
            else // Handle raw value case
            {
                Result += FString::Printf(TEXT("%s=%s, "), *Labels[i], *TruncateFloatString(Components[i].TrimStartAndEnd()));
            }
        }
         if (!Result.IsEmpty())
        {
            Result.LeftChopInline(2);
            return FString::Printf(TEXT("(%s)"), *Result);
        }
    }
    
    // --- FALLBACK to original generic parser ---
    if (!Content.StartsWith(TEXT("(")) || !Content.EndsWith(TEXT(")"))) {
         if (!Content.Contains(TEXT("=")) && !Content.Contains(TEXT("\"")) && !Content.Contains(TEXT("'")) && !Content.IsEmpty()) {
            return Content.TrimQuotes();
        }
        return FMarkdownSpan::LiteralUnknown(TEXT("(...)"));
    }
    
    Content = Content.Mid(1, Content.Len() - 2).TrimStartAndEnd();
    if (Content.IsEmpty()) { return TEXT("()"); }
    
    static const FRegexPattern TagPattern(TEXT("TagName\\s*=\\s*\"?`?([^\"`]+)`?\"?"));
    FRegexMatcher TagMatcher(TagPattern, Content);
    if (TagMatcher.FindNext()) {
        return TagMatcher.GetCaptureGroup(1).TrimQuotes();
    }

    TArray<FString> Parts;
    FString ResultContent = TEXT(""); 
    Content.ParseIntoArray(Parts, TEXT(","));

    bool bIsSimpleKeyValue = true;
    for (const FString& Part : Parts) {
        FString Key, Value; 
        if (Part.Split(TEXT("="), &Key, &Value)) {
            Key = Key.TrimStartAndEnd();
            // Use truncation helper for all key-value struct numbers as well
            Value = TruncateFloatString(Value.TrimStartAndEnd().TrimQuotes());
            ResultContent += FString::Printf(TEXT("%s=%s, "), *Key, *Value);
        } else {
            bIsSimpleKeyValue = false;
            break;
        }
    }
    
    if (bIsSimpleKeyValue && !ResultContent.IsEmpty()) {
        ResultContent.LeftChopInline(2);
        return FString::Printf(TEXT("(%s)"), *ResultContent);
    }
    
    return FMarkdownSpan::LiteralUnknown(TEXT("(...)"));
}
    // ===== SEMANTIC STRUCT PARSING COMMENTED OUT =====
    /*
    // ===== PHASE 4.1.1: ParseStructDefaultValue Semantic Version =====
    FSemanticValue ParseStructDefaultValue_Semantic(const FString& ValueString)
    {
        FString Content = ValueString.TrimStartAndEnd();
        
        // Handle empty cases
        if (Content.IsEmpty() || Content == TEXT("()") || Content == TEXT("{}")) 
        {
            return FSemanticValue(TEXT("()"), ESemanticNodeType::Literal, TEXT("struct"));
        }

        // If not wrapped in parens, treat as unknown struct
        if (!Content.StartsWith(TEXT("(")) || !Content.EndsWith(TEXT(")"))) 
        {
            if (!Content.Contains(TEXT("=")) && !Content.Contains(TEXT("\"")) && !Content.Contains(TEXT("'")) && !Content.IsEmpty()) 
            {
                // It's a single value, likely a simple struct
                return FSemanticValue(Content.TrimQuotes(), ESemanticNodeType::Literal, TEXT("struct"));
            }
            return FSemanticValue(TEXT("(...)"), ESemanticNodeType::Literal, TEXT("struct"));
        }

        Content = Content.Mid(1, Content.Len() - 2).TrimStartAndEnd(); // Strip outer parens
        if (Content.IsEmpty()) 
        {
            return FSemanticValue(TEXT("()"), ESemanticNodeType::Literal, TEXT("struct"));
        }

        // Special handling for GameplayTag: extract TagName directly
        static const FRegexPattern TagPattern(TEXT("TagName\\s*=\\s*\"?`?([^\"`]+)`?\"?"));
        FRegexMatcher TagMatcher(TagPattern, Content);
        if (TagMatcher.FindNext()) 
        {
            return FSemanticValue(TagMatcher.GetCaptureGroup(1).TrimQuotes(), ESemanticNodeType::Literal, TEXT("gameplaytag"));
        }

        // Generic struct parsing: "Key1=Value1, Key2=Value2"
        TArray<FString> Parts;
        FString ResultContent = TEXT(""); 
        Content.ParseIntoArray(Parts, TEXT(",")); // Split by comma

        bool bIsSimpleKeyValue = true;
        for (const FString& Part : Parts) 
        {
            FString Key = TEXT(""), Value = TEXT(""); 
            if (Part.Split(TEXT("="), &Key, &Value)) 
            {
                Key = Key.TrimStartAndEnd();
                Value = Value.TrimStartAndEnd().TrimQuotes(); // Strip quotes from value part
                
                // Attempt to parse as number for cleaner output
                if (Value.IsNumeric())
                {
                    // ← ADD THIS CHECK
                    TOptional<double> NumVal = FCString::Atod(*Value);
                    if(NumVal.IsSet()) 
                    {
                        // Check if it's an integer for cleaner display
                        double FracPart, IntPart;
                        FracPart = FMath::Modf(NumVal.GetValue(), &IntPart);
                        if (FMath::IsNearlyZero(FracPart))
                        {
                            Value = FString::Printf(TEXT("%lld"), static_cast<long long>(IntPart));
                        }
                        else
                        {
                            Value = FString::Printf(TEXT("%g"), NumVal.GetValue());
                        }
                    }
                }

                ResultContent += FString::Printf(TEXT("%s=%s, "), *Key, *Value);
            } 
            else 
            {
                // If a part doesn't contain '=', it's not simple key-value
                bIsSimpleKeyValue = false;
                break;
            }
        }

        if (bIsSimpleKeyValue && !ResultContent.IsEmpty()) 
        {
            ResultContent.LeftChopInline(2); // Remove trailing ", "
            return FSemanticValue(FString::Printf(TEXT("(%s)"), *ResultContent), ESemanticNodeType::Literal, TEXT("struct"));
        }

        // If not simple key-value, return generic placeholder
        return FSemanticValue(TEXT("(...)"), ESemanticNodeType::Literal, TEXT("struct"));
    }
    */
    
    FString MarkdownFormattingUtils::ParseStructDefaultValue(const FString& ValueString, const FString& StructTypeName)
    {
  // 🔴 SEMANTIC APPROACH DISABLED - Using legacy only

        return ParseStructDefaultValue_Legacy(ValueString, StructTypeName);
    
      
       
        
        /* COMMENTED OUT: Semantic validation approach
        // Phase 4.1.1: Validate semantic vs legacy output
        FSemanticValue semanticResult = ParseStructDefaultValue_Semantic(ValueString);
        FString semanticOutput = semanticResult.ValueRepresentation; 
        FString legacyOutput = ParseStructDefaultValue_Legacy(ValueString);
    
        // Clean both outputs for comparison (remove formatting)
        FString cleanSemantic = semanticOutput.Replace(TEXT("`"), TEXT("")).TrimStartAndEnd();
        FString cleanLegacy = legacyOutput.Replace(TEXT("`"), TEXT("")).TrimStartAndEnd();
    
        bool bMatches = cleanSemantic.Equals(cleanLegacy, ESearchCase::IgnoreCase);
    
        if (!bMatches)
        {
            UE_LOG(LogFormatter, Error, TEXT("🚨 PHASE 4.1.1 VALIDATION FAILED for input '%s':"), *ValueString);
            UE_LOG(LogFormatter, Error, TEXT("  Legacy: %s"), *legacyOutput);
            UE_LOG(LogFormatter, Error, TEXT("  Semantic: %s"), *semanticOutput);
            UE_LOG(LogFormatter, Error, TEXT("  Clean Legacy: %s"), *cleanLegacy);
            UE_LOG(LogFormatter, Error, TEXT("  Clean Semantic: %s"), *cleanSemantic);
        
            // Return legacy on validation failure (safe fallback)
            return legacyOutput;
        }
    
        // Validation passed - return semantic output
        return semanticOutput;
        */
    }
    
    FString FormatArgumentsForTrace(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
        int32 Depth,
        TSet<FString>& VisitedPins,
        const TSet<FName>& ExcludePinNames,
        TSharedPtr<const FBlueprintNode> CallingNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
        bool bSymbolicTrace,
        const FString& CurrentBlueprintContext)
    {
        if (Node.IsValid() && Node->NodeType == TEXT("Composite"))
        {
            UE_LOG(LogFormatter, Error, TEXT("FormatArgumentsForTrace (Utils): Entered for COMPOSITE Node '%s' (GUID: %s). Context: '%s'. Symbolic: %d, ShowTrivialGlobal: %s"),
                *Node->Name, *Node->Guid.Left(8), *CurrentBlueprintContext, bSymbolicTrace, Tracer->bCurrentShowTrivialDefaultParams ? TEXT("true") : TEXT("false"));
            
            TArray<TSharedPtr<FBlueprintPin>> InputPinsForComposite = Node->GetInputPins(TEXT(""), true, true);
            UE_LOG(LogFormatter, Error, TEXT("  FormatArgumentsForTrace (Composite): Node->GetInputPins() found %d pins to iterate for arguments:"), InputPinsForComposite.Num());
            for (const TSharedPtr<FBlueprintPin>& CompPin : InputPinsForComposite)
            {
                if (CompPin.IsValid())
                {
                    UE_LOG(LogFormatter, Error, TEXT("    Composite Arg Candidate Pin: Name='%s', ID='%s', Direction='%s', Category='%s', TypeSig='%s', FriendlyName=%s"),
                        *CompPin->Name, 
                        *CompPin->Id.Left(8), 
                        *CompPin->Direction, 
                        *CompPin->Category,
                        *CompPin->GetTypeSignature(),
                        *CompPin->FriendlyName
                    );
                }
            }
        }
        check(Node.IsValid() && Tracer);
        TArray<FString> ArgsList;
        
        TSet<FName> FinalExclusions = {
            FName(TEXT("self")), FName(TEXT("Target")), 
            FName(TEXT("WorldContextObject")), FName(TEXT("__WorldContext")), 
            FName(TEXT("LatentInfo")),                                     
            FName(TEXT("execute")), FName(TEXT("exec")), FName(TEXT("then")), FName(TEXT("__then__")), 
            FName(TEXT("ReturnValue"))                                     
        };
        FinalExclusions.Append(ExcludePinNames);

        UE_LOG(LogFormatter, Log, TEXT("FormatArgumentsForTrace (Utils): Node '%s', Context '%s', Symbolic: %d, ShowTrivialGlobal: %s"), 
            *Node->Name, *CurrentBlueprintContext, bSymbolicTrace, Tracer->bCurrentShowTrivialDefaultParams ? TEXT("true") : TEXT("false"));

        for (const TSharedPtr<FBlueprintPin>& Pin : Node->GetInputPins(TEXT(""), true, true)) // Include hidden, exclude exec by default
        {
            if (!Pin.IsValid()) continue; 
            
            // Skip hidden pins unless they are linked or we are showing all defaults
            if (Pin->IsHidden() && Pin->SourcePinFor.Num() == 0 && !Tracer->bCurrentShowTrivialDefaultParams)
            {
                 continue;
            }

            const FName PinFName(*Pin->Name);
            if (!FinalExclusions.Contains(PinFName))
            {
                bool bIsLinked = Pin->SourcePinFor.Num() > 0;
                bool bHasExplicitDefault = !Pin->DefaultValue.IsEmpty() || 
                                           !Pin->DefaultObject.IsEmpty() || 
                                           Pin->DefaultStruct.Num() > 0;
                
                bool bIsPinTrivialDefault = MarkdownTracerUtils::IsTrivialDefault(Pin);

                // Condition to include the argument:
                // 1. Global flag to show all trivial defaults is true, OR
                // 2. Pin is linked (value comes from elsewhere), OR
                // 3. Pin has an explicit default value AND that default is NOT trivial.
                if (Tracer->bCurrentShowTrivialDefaultParams || bIsLinked || (bHasExplicitDefault && !bIsPinTrivialDefault))
                {
                    UE_LOG(LogFormatter, Error, TEXT("    FormatArgumentsForTrace: Tracing arg pin '%s' (Node:'%s'). CtxToPass:'%s', Depth:%d, SymbolicForThisTrace:%d"),
                                           *Pin->Name, *Node->Name, *CurrentBlueprintContext, Depth, bSymbolicTrace);

                    FString PinValue = Tracer->ResolvePinValueRecursive(Pin, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
                        
                    FString DisplayPinName = (!Pin->FriendlyName.IsEmpty() && Pin->FriendlyName != Pin->Name) ? Pin->FriendlyName : Pin->Name;
                    DisplayPinName.TrimStartAndEndInline(); 

                    ArgsList.Add(FString::Printf(TEXT("%s=%s"),
                        *FMarkdownSpan::ParamName(DisplayPinName), // ✅ FIX: Let ParamName handle backticks
                        *PinValue
                    ));
                    UE_LOG(LogFormatter, Verbose, TEXT("  FormatArgumentsForTrace (Utils): Added Pin '%s' (Disp: '%s', Val: '%s'). Context: '%s'. TrivialGlobal=%d, Linked=%d, ExplicitDefault=%d, IsPinTrivial=%d"), 
                        *Pin->Name, *DisplayPinName, *PinValue, *CurrentBlueprintContext,
                        Tracer->bCurrentShowTrivialDefaultParams, bIsLinked, bHasExplicitDefault, bIsPinTrivialDefault);
                }
                else
                {
                    UE_LOG(LogFormatter, Verbose, TEXT("  FormatArgumentsForTrace (Utils): SKIPPED Pin '%s' on Node '%s'. Context: '%s'. TrivialGlobal=%d, Linked=%d, ExplicitDefault=%d, IsPinTrivial=%d"),
                        *Pin->Name, *Node->Name, *CurrentBlueprintContext,
                        Tracer->bCurrentShowTrivialDefaultParams, bIsLinked, bHasExplicitDefault, bIsPinTrivialDefault);
                }
            }
        }
        return FString::Join(ArgsList, TEXT(", "));
    }

    // ✅ NEW: Enhanced type parsing implementation
FBlueprintTypeInfo MarkdownFormattingUtils::ExtractTypeInformation(TSharedPtr<const FBlueprintPin> Pin)
{
    if (!Pin.IsValid())
    {
        return FBlueprintTypeInfo(TEXT("unknown"));
    }
    
    FBlueprintTypeInfo TypeInfo;
    
    // Extract base type from category
    FString Category = Pin->Category;
    if (Category.IsEmpty() || Category == TEXT("None"))
    {
        Category = TEXT("unknown");
    }
        if (Category == TEXT("enum") || 
            (Category == TEXT("byte") && Pin->SubCategoryObject.Contains(TEXT("Enum"), ESearchCase::IgnoreCase)))
        {
            // Extract enum name from SubCategoryObject
            FString EnumName = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
            if (!EnumName.IsEmpty())
            {
                TypeInfo.BaseType = EnumName;
            }
            else
            {
                TypeInfo.BaseType = TEXT("enum");
            }
        }
    // Normalize common type names
    if (Category == TEXT("real") || Category == TEXT("double"))
    {
        Category = TEXT("float");
    }
    else if (Category == TEXT("int64"))
    {
        Category = TEXT("int");
    }
    else if (Category == TEXT("text"))
    {
        Category = TEXT("string");
    }
    
    // Handle object types with SubCategoryObject
    if ((Category == TEXT("object") || Category == TEXT("class") || Category == TEXT("interface")) 
        && !Pin->SubCategoryObject.IsEmpty())
    {
        FString SimpleName = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
        if (!SimpleName.IsEmpty())
        {
            // Common Blueprint object types
            if (SimpleName.Contains(TEXT("Actor")))
            {
                Category = TEXT("actor");
            }
            else if (SimpleName.Contains(TEXT("Component")))
            {
                Category = TEXT("component");
            }
            else if (SimpleName.Contains(TEXT("Widget")))
            {
                Category = TEXT("widget");
            }
            else
            {
                Category = SimpleName.ToLower();
            }
        }
    }
    
    // Handle struct types
    if (Category == TEXT("struct") && !Pin->SubCategoryObject.IsEmpty())
    {
        FString StructName = MarkdownTracerUtils::ExtractSimpleNameFromPath(Pin->SubCategoryObject);
        if (!StructName.IsEmpty())
        {
            // Common vector types
            if (StructName == TEXT("Vector") || StructName == TEXT("FVector"))
            {
                Category = TEXT("vector");
            }
            else if (StructName == TEXT("Vector2D") || StructName == TEXT("FVector2D"))
            {
                Category = TEXT("vector2");
            }
            else if (StructName == TEXT("Rotator") || StructName == TEXT("FRotator"))
            {
                Category = TEXT("rotator");
            }
            else if (StructName == TEXT("Transform") || StructName == TEXT("FTransform"))
            {
                Category = TEXT("transform");
            }
            else
            {
                Category = StructName.ToLower();
            }
        }
    }
    
    TypeInfo.BaseType = Category;
    TypeInfo.ContainerType = Pin->ContainerType;
    
    // Handle Map value types
    if (Pin->ContainerType == TEXT("Map") && !Pin->MapValueTerminalCategory.IsEmpty())
    {
        FString ValueCategory = Pin->MapValueTerminalCategory;
        
        // Apply same normalization to value type
        if (ValueCategory == TEXT("real") || ValueCategory == TEXT("double"))
        {
            ValueCategory = TEXT("float");
        }
        else if (ValueCategory == TEXT("int64"))
        {
            ValueCategory = TEXT("int");
        }
        else if (ValueCategory == TEXT("text"))
        {
            ValueCategory = TEXT("string");
        }
        
        TypeInfo.ValueType = ValueCategory;
    }
    
    // Generate display information
    TypeInfo.GenerateDisplayInfo();
    
    UE_LOG(LogFormatter, Verbose, TEXT("ExtractTypeInformation: Pin '%s' -> BaseType='%s', Container='%s', Display='%s'"),
        *Pin->Name, *TypeInfo.BaseType, *TypeInfo.ContainerType, *TypeInfo.DisplayName);
    
    return TypeInfo;
}

FString MarkdownFormattingUtils::FormatTypeForHTML(const FBlueprintTypeInfo& TypeInfo)
{
    // Generate HTML with CSS classes for styling
    return FString::Printf(TEXT("<span class=\"%s\">%s</span>"), 
        *TypeInfo.CSSClasses, 
        *FMarkdownSpanSystem::EscapeHtml(TypeInfo.DisplayName));
}

FString MarkdownFormattingUtils::FormatTypeForMarkdown(const FBlueprintTypeInfo& TypeInfo)
{
    // Simple backtick formatting for Markdown
    return FString::Printf(TEXT("`%s`"), *TypeInfo.DisplayName);
}

FBlueprintIOSpec MarkdownFormattingUtils::CreateIOSpecification(
    TSharedPtr<const FBlueprintPin> Pin,
    const FString& ValueExpression)
{
    if (!Pin.IsValid())
    {
        return FBlueprintIOSpec();
    }
    
    FBlueprintTypeInfo TypeInfo = ExtractTypeInformation(Pin);
    FString PinName = Pin->FriendlyName.IsEmpty() ? Pin->Name : Pin->FriendlyName;
    
    return FBlueprintIOSpec(PinName, TypeInfo, ValueExpression);
}

TArray<FBlueprintIOSpec> MarkdownFormattingUtils::ExtractPinSpecifications(
    TSharedPtr<const FBlueprintNode> Node,
    bool bInputPins)
{
    TArray<FBlueprintIOSpec> Specifications;
    
    if (!Node.IsValid())
    {
        return Specifications;
    }
    
    // Get appropriate pins
    TArray<TSharedPtr<FBlueprintPin>> Pins = bInputPins ? 
        Node->GetInputPins(TEXT(""), false, true) :  // Include hidden, exclude exec
        Node->GetOutputPins(TEXT(""), false);         // Exclude hidden
    
    for (const TSharedPtr<FBlueprintPin>& Pin : Pins)
    {
        if (!Pin.IsValid())
        {
            continue;
        }
        
        // Skip execution pins for I/O specifications
        if (Pin->IsExecution())
        {
            continue;
        }
        
        // Skip common utility pins
        const FName PinFName(*Pin->Name);
        if (PinFName == FName(TEXT("self")) || 
            PinFName == FName(TEXT("Target")) || 
            PinFName == FName(TEXT("WorldContextObject")) ||
            PinFName == FName(TEXT("__WorldContext")) ||
            PinFName == FName(TEXT("LatentInfo")))
        {
            continue;
        }
        
        FBlueprintIOSpec Spec = CreateIOSpecification(Pin);
        Specifications.Add(Spec);
    }
    
    // Sort by name for consistent display
    Specifications.Sort([](const FBlueprintIOSpec& A, const FBlueprintIOSpec& B) {
        return A.Name < B.Name;
    });
    
    UE_LOG(LogFormatter, Log, TEXT("ExtractPinSpecifications: Node '%s' %s pins -> %d specifications"),
        *Node->Name, bInputPins ? TEXT("input") : TEXT("output"), Specifications.Num());
    
    return Specifications;
}

FString MarkdownFormattingUtils::ConvertEnumInternalToDisplay(const FString& InternalValue, const FString& EnumSubCategoryObject)
{
    // Extract enum class name from SubCategoryObject path
    FString EnumClassName = MarkdownTracerUtils::ExtractSimpleNameFromPath(EnumSubCategoryObject);
    if (EnumClassName.IsEmpty())
    {
        UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: Could not extract enum class name from '%s'"), *EnumSubCategoryObject);
        return InternalValue; // Fallback to original
    }
    
    // Find the UEnum object using UE's reflection system
    UEnum* EnumClass = nullptr;
    
    // Method 1: Try to find by name directly
    EnumClass = FindFirstObject<UEnum>(*EnumClassName, EFindFirstObjectOptions::None, ELogVerbosity::NoLogging);
    
    // Method 2: If not found, try with package path
    if (!EnumClass)
    {
        // Try to load from the full path
        FString FullPath = EnumSubCategoryObject;
        EnumClass = LoadObject<UEnum>(nullptr, *FullPath);
    }
    
    // Method 3: Search through all loaded enums as fallback
    if (!EnumClass)
    {
        for (TObjectIterator<UEnum> EnumIt; EnumIt; ++EnumIt)
        {
            UEnum* CurrentEnum = *EnumIt;
            if (CurrentEnum && CurrentEnum->GetName() == EnumClassName)
            {
                EnumClass = CurrentEnum;
                break;
            }
        }
    }
    
    if (!EnumClass)
    {
        UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: Could not find UEnum for '%s'"), *EnumClassName);
        return InternalValue; // Fallback to original
    }
    
    // Convert internal name to enum value
    int64 EnumValue = EnumClass->GetValueByName(*InternalValue);
    if (EnumValue == INDEX_NONE)
    {
        UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: Could not find enum value for '%s' in enum '%s'"), *InternalValue, *EnumClassName);
        return InternalValue; // Fallback to original
    }
    
    // ? --- NEW LOGIC ---
    // 1. Try to get the user-defined Display Name first.
    FText DisplayNameText = EnumClass->GetDisplayNameTextByValue(EnumValue);
    if (!DisplayNameText.IsEmpty())
    {
        FString DisplayName = DisplayNameText.ToString();
        UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: Converted '%s' to DISPLAY NAME '%s' for enum '%s'"), *InternalValue, *DisplayName, *EnumClassName);
        return DisplayName;
    }
    
    // 2. If no display name, fall back to the "friendly name" (the enumerator name itself, e.g., "Forward").
    FString FriendlyName = EnumClass->GetNameStringByValue(EnumValue);
    if (!FriendlyName.IsEmpty())
    {
        // Remove the enum prefix if present (e.g., "E_MyEnum::Value" -> "Value")
        int32 ColonIndex = FriendlyName.Find(TEXT("::"));
        if (ColonIndex != INDEX_NONE)
        {
            FriendlyName = FriendlyName.RightChop(ColonIndex + 2);
        }
        
        UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: Using FRIENDLY NAME '%s' for '%s'"), *FriendlyName, *InternalValue);
        return FriendlyName;
    }
    
    // 3. Only as a last resort, fall back to the original internal value (which might be the number).
    UE_LOG(LogFormatter, Error, TEXT("ConvertEnumInternalToDisplay: No display or friendly name found for '%s', using original internal value."), *InternalValue);
    return InternalValue;
}
    
    
} // namespace MarkdownFormattingUtils


