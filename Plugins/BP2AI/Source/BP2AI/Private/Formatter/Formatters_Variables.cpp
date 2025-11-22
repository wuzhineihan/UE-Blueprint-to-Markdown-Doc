/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/Formatters_Variables.cpp
#include "Formatters_Private.h"

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan

namespace MarkdownNodeFormatters_Private
{
    FString FormatVariableSet( 
       TSharedPtr<const FBlueprintNode> Node, 
       FMarkdownDataTracer& DataTracer, 
       const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
       TSet<FString>& VisitedDataPins,
       bool bSymbolicTraceForData,
       const FString& CurrentBlueprintContext
    )
    {
       const FString VarName = Node->RawProperties.FindRef(TEXT("VariableName"));
       if (VarName.IsEmpty()) return FMarkdownSpan::Error(TEXT("[Unknown Variable Set]"));

       TSharedPtr<const FBlueprintPin> ValuePin = Node->GetPin(VarName, TEXT("EGPD_Input"));
       if (!ValuePin) { for(const auto& Pair : Node->Pins) { if (Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && Pair.Value->Name != TEXT("self")) { ValuePin = Pair.Value; break; } } }

       FString ValueStr = FMarkdownSpan::Error(TEXT("<?>"));
       // Pass bSymbolicTraceForData and CurrentBlueprintContext
       if (ValuePin.IsValid()) { ValueStr = DataTracer.TracePinValue(ValuePin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext); }

       FString VarTypeSig = ValuePin ? ValuePin->GetTypeSignature() : TEXT("?");
    	
       
       // NEW FIXED CODE: Let span functions add backticks automatically
       FString VarSpan = FMarkdownSpan::Variable(VarName);
       FString TypeSpan = FMarkdownSpan::DataType(VarTypeSig);
       
       // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
       FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext);

       // 🔧 FIX: Properly format variable:type without manual backticks
       return FString::Printf(TEXT("%s %s:%s = %s%s"),
          *FMarkdownSpan::Keyword(TEXT("Set")),
          *VarSpan,      // Already has proper backticks: `VarName`
          *TypeSpan,     // Already has proper backticks: `TypeSig`
          *ValueStr, 
          *TargetStr);
    }

} // namespace MarkdownNodeFormatters_Private