/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// PROVIDE FULL FILE
// Source/BP2AI/Private/Formatter/Formatters_Data.cpp
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
	FString FormatSetFieldsInStruct( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
		TSharedPtr<const FBlueprintPin> StructPin = Node->GetPin(TEXT("StructRef"), TEXT("EGPD_Input"));
		if (!StructPin) StructPin = Node->GetPin(TEXT("Struct In"), TEXT("EGPD_Input"));

		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString StructStr = StructPin.IsValid() ? DataTracer.TracePinValue(StructPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("[?Struct?]"));

		TSet<FName> Exclusions;
        if(StructPin.IsValid()) Exclusions.Add(FName(*StructPin->Name));
        TSharedPtr<const FBlueprintPin> OutputPin = Node->GetPin(TEXT("StructRef"), TEXT("EGPD_Output"));
         if (!OutputPin) OutputPin = Node->GetPin(TEXT("Result"), TEXT("EGPD_Output")); 
         if(OutputPin.IsValid()) Exclusions.Add(FName(*OutputPin->Name));

		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString FieldsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Set Fields"));
		return FString::Printf(TEXT("%s in (%s) %s"), *Keyword, *StructStr, *FieldsStr);
	}

	// FULL FUNCTION REPLACEMENT
	FString FormatCallArrayFunction( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
        const FString FuncName = Node->RawProperties.FindRef(TEXT("FunctionName"));
		if (FuncName.IsEmpty()) return FMarkdownSpan::Error(TEXT("[Unknown Array Function]"));

		TSharedPtr<const FBlueprintPin> ArrayPin = nullptr;
        for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsInput() && Pair.Value->ContainerType == TEXT("Array")){ ArrayPin = Pair.Value; break; }}
        if (!ArrayPin) ArrayPin = Node->GetPin(TEXT("Target Array"), TEXT("EGPD_Input")); 

		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString ArrayStr = ArrayPin.IsValid() ? DataTracer.TracePinValue(ArrayPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("[?Array?]"));

        TSet<FName> Exclusions;
        if(ArrayPin.IsValid()) Exclusions.Add(FName(*ArrayPin->Name));

		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
        FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
        FString Keyword = FMarkdownSpan::Keyword(TEXT("Array Op"));
		FString FuncNameSpan = FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName));

		return FString::Printf(TEXT("%s %s%s on (%s)"), *Keyword, *FuncNameSpan, *ArgsStr, *ArrayStr);
	}

	// FULL FUNCTION REPLACEMENT
	FString FormatFormatText( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
        TSharedPtr<const FBlueprintPin> FormatPin = Node->GetPin(TEXT("Format"), TEXT("EGPD_Input"));
		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString FormatStr = FormatPin.IsValid() ? DataTracer.TracePinValue(FormatPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("<?>"));

		TSet<FName> Exclusions;
		if(FormatPin.IsValid()) Exclusions.Add(FName(*FormatPin->Name));

		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Format Text"));
		return FString::Printf(TEXT("%s %s %s"), *Keyword, *FormatStr, *ArgsStr);
	}

	FString MarkdownNodeFormatters_Private::FormatGetDataTableRow_NodeDescription(
    TSharedPtr<const FBlueprintNode> Node,
    FMarkdownDataTracer& DataTracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    TSet<FString>& VisitedDataPins, // Note: This specific formatter might not need to pass VisitedDataPins *down* if it only calls RVR once per input.
    bool bSymbolicTraceForData,    // This is the bSymbolicTrace flag from FormatNodeDescription's perspective
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("GetDataTableRow"));
    UE_LOG(LogFormatter, Log, TEXT("FormatGetDataTableRow_NodeDescription: Node='%s' (GUID:%s), CtxRecv:'%s', SymbolicForData:%d"),
        *Node->Name, *Node->Guid.Left(8), *CurrentBlueprintContext, bSymbolicTraceForData);

    // 1. Find and trace the 'DataTable' input pin
    TSharedPtr<const FBlueprintPin> DataTableInputPin = Node->GetPin(TEXT("DataTable"), TEXT("EGPD_Input"));
    FString DataTableNameStr = FMarkdownSpan::Error(TEXT("?DataTable?"));
    if (DataTableInputPin.IsValid())
    {
        // For the DataTable asset itself, we almost always want its actual name, not a further symbolic representation.
        // So, override bSymbolicTraceForData to 'false' for this specific RVR call.
        UE_LOG(LogFormatter, Log, TEXT("  FormatGetDataTableRow_NodeDescription: Tracing DataTable pin '%s'. CtxToPass:'%s', SymbolicForThisPinTrace:false"), *DataTableInputPin->Name, *CurrentBlueprintContext);
        DataTableNameStr = DataTracer.ResolvePinValueRecursive(DataTableInputPin, AllNodes, 0, VisitedDataPins, nullptr, nullptr, false, CurrentBlueprintContext);
    }
    else
    {
        UE_LOG(LogFormatter, Warning, TEXT("  FormatGetDataTableRow_NodeDescription: DataTable input pin not found for node %s."), *Node->Guid.Left(8));
    }

    // 2. Find and trace the 'RowName' input pin
    TSharedPtr<const FBlueprintPin> RowNameInputPin = Node->GetPin(TEXT("RowName"), TEXT("EGPD_Input"));
    FString RowNameStr = FMarkdownSpan::Error(TEXT("?RowName?"));
    if (RowNameInputPin.IsValid())
    {
        // For the RowName, respect the bSymbolicTraceForData flag passed in, as it could be dynamic.
        UE_LOG(LogFormatter, Log, TEXT("  FormatGetDataTableRow_NodeDescription: Tracing RowName pin '%s'. CtxToPass:'%s', SymbolicForThisPinTrace:%d"), *RowNameInputPin->Name, *CurrentBlueprintContext, bSymbolicTraceForData);
        RowNameStr = DataTracer.ResolvePinValueRecursive(RowNameInputPin, AllNodes, 0, VisitedDataPins, nullptr, nullptr, bSymbolicTraceForData, CurrentBlueprintContext);
    }
    else
    {
        UE_LOG(LogFormatter, Warning, TEXT("  FormatGetDataTableRow_NodeDescription: RowName input pin not found for node %s."), *Node->Guid.Left(8));
    }

    // 3. Construct the descriptive string
    // FMarkdownSpan::Keyword is currently passthrough, so "Get Data Table Row" handles NOT bolding.
    FString Description = FString::Printf(TEXT("%s from %s (Row: %s)"),
        *FMarkdownSpan::Keyword(TEXT("Get Data Table Row")),
        *DataTableNameStr, // Should already be formatted with backticks if a literal/variable name
        *RowNameStr        // Should already be formatted
    );

    // Optional: Append original node title if it's custom and different from "Get Data Table Row"
    // This matches the pattern used for the desired output of Scenario 3.
    if (!Node->Name.IsEmpty() && Node->Name != TEXT("Get Data Table Row") && Node->Name != Node->NodeType)
    {
        Description += FString::Printf(TEXT(" (Node Title: %s)"), *Node->Name);
    }
    
    UE_LOG(LogFormatter, Log, TEXT("  FormatGetDataTableRow_NodeDescription: Result: '%s'"), *Description);
    return Description;
}
	
	
} // namespace MarkdownNodeFormatters_Private