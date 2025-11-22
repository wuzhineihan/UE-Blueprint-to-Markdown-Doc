/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/Formatters_ControlFlow.cpp
#include "Formatters_Private.h"
#include "Logging/BP2AILog.h"

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "Logging/LogMacros.h"

namespace MarkdownNodeFormatters_Private
{
	// FULL FUNCTION REPLACEMENT
	FString FormatIfThenElse( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
		TSharedPtr<const FBlueprintPin> ConditionPin = Node->GetPin(TEXT("Condition"), TEXT("EGPD_Input"));
		FString ConditionStr = FMarkdownSpan::Error(TEXT("<?>"));
		if (ConditionPin.IsValid())
		{
			// Pass bSymbolicTraceForData and CurrentBlueprintContext
			ConditionStr = DataTracer.TracePinValue(ConditionPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
		}
		FString Keyword = FMarkdownSpan::Keyword(TEXT("If"));
		return FString::Printf(TEXT("%s (%s)"), *Keyword, *ConditionStr);
	}

	// FULL FUNCTION REPLACEMENT
	FString FormatSwitch( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
		TSharedPtr<const FBlueprintPin> SelectionPin = Node->GetPin(TEXT("Selection"), TEXT("EGPD_Input"));
		FString SelectionStr = FMarkdownSpan::Error(TEXT("<?>"));
		FString SwitchTypeStr = TEXT("");

		if (SelectionPin.IsValid())
		{
			// Pass bSymbolicTraceForData and CurrentBlueprintContext
			SelectionStr = DataTracer.TracePinValue(SelectionPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
			
			if (Node->NodeType == TEXT("SwitchEnum")) {
				 FString EnumTypePath = Node->RawProperties.FindRef(TEXT("Enum"));
				 if (EnumTypePath.IsEmpty() && !SelectionPin->SubCategoryObject.IsEmpty()) { EnumTypePath = SelectionPin->SubCategoryObject; }
				 FString EnumTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(EnumTypePath); // No context needed for type name
				 if (EnumTypeName.IsEmpty()) EnumTypeName = TEXT("Enum?");
				 SwitchTypeStr = FString::Printf(TEXT(" on Enum %s"), *FMarkdownSpan::EnumType(FString::Printf(TEXT("%s"), *EnumTypeName)));
			} else if (Node->NodeType == TEXT("SwitchInteger")) {
				FString dataType = FMarkdownSpan::DataType(TEXT("Integer"));
				SwitchTypeStr = FString::Printf(TEXT(" on %s"), *dataType);
			} else if (Node->NodeType == TEXT("SwitchString")) {
				 SwitchTypeStr = FString::Printf(TEXT(" on %s"), *FMarkdownSpan::DataType(TEXT("String")));
			} else if (Node->NodeType == TEXT("SwitchName")) {
				 SwitchTypeStr = FString::Printf(TEXT(" on %s"), *FMarkdownSpan::DataType(TEXT("Name")));
			} else {
                 FString PinType = SelectionPin->GetTypeSignature();
                 if (!PinType.IsEmpty() && PinType != TEXT("exec")) {
                      SwitchTypeStr = FString::Printf(TEXT(" on %s"), *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *PinType)));
                 }
            }
		}

		FString Keyword = FMarkdownSpan::Keyword(TEXT("Switch"));
		return FString::Printf(TEXT("%s%s (%s)"), *Keyword, *SwitchTypeStr, *SelectionStr);
	}

	// FULL FUNCTION REPLACEMENT
    FString FormatSequence( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext // Context received, but not used by this formatter
	)
    {
		return FMarkdownSpan::Keyword(TEXT("Sequence"));
	}

	// FULL FUNCTION REPLACEMENT
// PROVIDE FULL FUNCTION
FString FormatForEachLoop( 
    TSharedPtr<const FBlueprintNode> Node, 
    FMarkdownDataTracer& DataTracer, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
    TSet<FString>& VisitedDataPins,
    bool bSymbolicTraceForData,
    const FString& CurrentBlueprintContext
)
{
    UE_LOG(LogFormatter, Log, TEXT("FormatForEachLoop: Formatting node %s (Type: %s, GUID: %s). Context: '%s'"), *Node->Name, *Node->NodeType, *Node->Guid, *CurrentBlueprintContext);
    TSharedPtr<const FBlueprintPin> ArrayPin = Node->GetPin(TEXT("Array"), TEXT("EGPD_Input"));
    UE_LOG(LogFormatter, Log, TEXT("  ArrayInputPin valid: %s (PinName: %s, PinID: %s)"), 
        ArrayPin.IsValid() ? TEXT("true") : TEXT("false"),
        ArrayPin.IsValid() ? *ArrayPin->Name : TEXT("N/A"),
        ArrayPin.IsValid() ? *ArrayPin->Id : TEXT("N/A")
    );
    
    FString ArrayStr = FMarkdownSpan::Error(TEXT("<?>"));
    if (ArrayPin.IsValid()) {
        // Simplified: Directly trace the ArrayPin. FNodeTraceHandlers_Functions::HandleCallFunction 
        // will now be responsible for providing the rich string like "Lib.Func(Args).Pin"
        ArrayStr = DataTracer.TracePinValue(ArrayPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
    }
    UE_LOG(LogFormatter, Log, TEXT("  Final Traced ArrayStr for loop header (from DataTracer): '%s'"), *ArrayStr);

    // Attempt to get Element and Index pins, trying both naming conventions
    TSharedPtr<const FBlueprintPin> ElemPin = Node->GetPin(TEXT("Array Element"), TEXT("EGPD_Output"));
    if (!ElemPin.IsValid()) { 
        ElemPin = Node->GetPin(TEXT("ArrayElement"), TEXT("EGPD_Output"));
    }

    TSharedPtr<const FBlueprintPin> IndexPin = Node->GetPin(TEXT("Array Index"), TEXT("EGPD_Output"));
    if (!IndexPin.IsValid()) { 
        IndexPin = Node->GetPin(TEXT("ArrayIndex"), TEXT("EGPD_Output"));
    }
    
    UE_LOG(LogFormatter, Log, TEXT("  After checks - ElemPin valid: %s, IndexPin valid: %s"), ElemPin.IsValid() ? TEXT("true") : TEXT("false"), IndexPin.IsValid() ? TEXT("true") : TEXT("false"));
    
    FString ElemType = TEXT("?");
    if (ElemPin.IsValid())
    {
        ElemType = ElemPin->GetTypeSignature();
        UE_LOG(LogFormatter, Log, TEXT("  ElemOutputPin Name: '%s', Category: '%s', SubCategory: '%s', SubCategoryObject: '%s', TypeSignature: '%s'"), *ElemPin->Name, *ElemPin->Category, *ElemPin->SubCategory, *ElemPin->SubCategoryObject, *ElemType);
    } else {
        UE_LOG(LogFormatter, Warning, TEXT("  FormatForEachLoop: 'Element' output pin (tried 'Array Element' and 'ArrayElement') not found for node %s."), *Node->Guid);
    }

    FString IndexType = TEXT("?");
    if (IndexPin.IsValid())
    {
        IndexType = IndexPin->GetTypeSignature();
        UE_LOG(LogFormatter, Log, TEXT("  IndexOutputPin Name: '%s', Category: '%s', SubCategory: '%s', SubCategoryObject: '%s', TypeSignature: '%s'"), *IndexPin->Name, *IndexPin->Category, *IndexPin->SubCategory, *IndexPin->SubCategoryObject, *IndexType);
    } else {
        UE_LOG(LogFormatter, Warning, TEXT("  FormatForEachLoop: 'Index' output pin (tried 'Array Index' and 'ArrayIndex') not found for node %s."), *Node->Guid);
    }
		FString Keyword; // Will be set below
		// Determine keyword based on SimpleMacroName for better accuracy
		FString SimpleMacroName;
		const FString* MacroPathPtr = Node->RawProperties.Find(TEXT("MacroGraphReference"));
		if (MacroPathPtr && !MacroPathPtr->IsEmpty()) { // Ensure MacroPathPtr is valid and not empty
			// Use CurrentBlueprintContext as these macros are standard but their usage is in a specific BP context
			SimpleMacroName = MarkdownTracerUtils::ExtractSimpleNameFromPath(**MacroPathPtr, CurrentBlueprintContext);
		} else {
			// Fallback if MacroGraphReference is missing (should be rare for standard macros)
			// Try Node->Name, which might be "For Each Loop with Break" etc.
			FString NodeNameStr = Node->Name;
			if (NodeNameStr.Contains(TEXT("Reverse For Each Loop"))) SimpleMacroName = TEXT("ReverseForEachLoop");
			else if (NodeNameStr.Contains(TEXT("For Each Loop with Break"))) SimpleMacroName = TEXT("ForEachLoopWithBreak");
			else if (NodeNameStr.Contains(TEXT("For Each Loop"))) SimpleMacroName = TEXT("ForEachLoop");
			else SimpleMacroName = Node->Name; // Fallback to raw node name
		}
    
		UE_LOG(LogFormatter, Log, TEXT("  FormatForEachLoop: Derived SimpleMacroName for Keyword: '%s' (Original Node Name: '%s')"), *SimpleMacroName, *Node->Name);

		if (SimpleMacroName == TEXT("ForEachLoopWithBreak")) {
			Keyword = FMarkdownSpan::Keyword(TEXT("For Each Loop with Break"));
		} else if (SimpleMacroName == TEXT("ReverseForEachLoop")) {
			Keyword = FMarkdownSpan::Keyword(TEXT("Reverse For Each"));
		} else { // Default for "ForEachLoop" or any other unrecognized variant
			Keyword = FMarkdownSpan::Keyword(TEXT("For Each"));
		}

    FString FormattedString = FString::Printf(TEXT("%s in (%s) [%s:%s, %s:%s]"),
        *Keyword,
        *ArrayStr, 
        *FMarkdownSpan::ParamName(TEXT("Element")), *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *ElemType)),
        *FMarkdownSpan::ParamName(TEXT("Index")), *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *IndexType))
    );
    UE_LOG(LogFormatter, Log, TEXT("  Final FormattedString: '%s'"), *FormattedString);
    return FormattedString;
}
} // namespace MarkdownNodeFormatters_Private