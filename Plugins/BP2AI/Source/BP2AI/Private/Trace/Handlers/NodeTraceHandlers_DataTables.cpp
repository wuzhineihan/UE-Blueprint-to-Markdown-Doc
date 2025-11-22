/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_DataTables.cpp

// In NodeTraceHandlers_Data.cpp (or NodeTraceHandlers_DataTables.cpp)
#include "Trace/Handlers/NodeTraceHandlers_DataTables.h"
// Or NodeTraceHandlers_DataTables.h
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Engine/DataTable.h"
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_DataTables::HandleGetDataTableRow(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("GetDataTableRow"));
    check(Tracer);
    check(OutputPin.IsValid());

    UE_LOG(LogDataTracer, Error, TEXT("  HandleGetDataTableRow: Node='%s' (GUID:%s), OutputPin='%s' (ID:%s). CtxRecv:'%s', Symbolic:%d, Depth:%d"),
        *Node->Name, *Node->Guid.Left(8),
        *OutputPin->Name, *OutputPin->Id.Left(8),
        *CurrentBlueprintContext, bSymbolicTrace, Depth);

    // 1. Find and trace the 'DataTable' input pin
    TSharedPtr<const FBlueprintPin> DataTableInputPin = Node->GetPin(TEXT("DataTable"), TEXT("EGPD_Input"));
    FString DataTableNameStr = FMarkdownSpan::Error(TEXT("?DataTable?"));
    if (DataTableInputPin.IsValid())
    {
        UE_LOG(LogDataTracer, Warning, TEXT("    HandleGetDataTableRow: Tracing DataTableInputPin '%s' (ID:%s). DefObj:'%s', Links:%d. CtxToPass:'%s'"),
            *DataTableInputPin->Name, *DataTableInputPin->Id.Left(8), *DataTableInputPin->DefaultObject, DataTableInputPin->SourcePinFor.Num(), *CurrentBlueprintContext);
        DataTableNameStr = Tracer->ResolvePinValueRecursive(DataTableInputPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, false, CurrentBlueprintContext);
        UE_LOG(LogDataTracer, Warning, TEXT("    HandleGetDataTableRow: DataTableInputPin resolved to: '%s'"), *DataTableNameStr);
    }
    else
    {
        UE_LOG(LogDataTracer, Error, TEXT("    HandleGetDataTableRow: DataTableInputPin NOT FOUND."));
    }

    // 2. Find and trace the 'RowName' input pin
    TSharedPtr<const FBlueprintPin> RowNameInputPin = Node->GetPin(TEXT("RowName"), TEXT("EGPD_Input"));
    FString RowNameStr = FMarkdownSpan::Error(TEXT("?RowName?"));
    if (RowNameInputPin.IsValid())
    {
        UE_LOG(LogDataTracer, Warning, TEXT("    HandleGetDataTableRow: Tracing RowNameInputPin '%s' (ID:%s). DefVal:'%s', Links:%d. CtxToPass:'%s', SymbolicForThisTrace:%d"),
            *RowNameInputPin->Name, *RowNameInputPin->Id.Left(8), *RowNameInputPin->DefaultValue, RowNameInputPin->SourcePinFor.Num(), *CurrentBlueprintContext, bSymbolicTrace);
        RowNameStr = Tracer->ResolvePinValueRecursive(RowNameInputPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
        UE_LOG(LogDataTracer, Warning, TEXT("    HandleGetDataTableRow: RowNameInputPin resolved to: '%s'"), *RowNameStr);
    }
    else
    {
        UE_LOG(LogDataTracer, Error, TEXT("    HandleGetDataTableRow: RowNameInputPin NOT FOUND."));
    }

    FString BaseSymbolicRepresentation = FString::Printf(TEXT("(%s[%s])"), *DataTableNameStr, *RowNameStr);
    UE_LOG(LogDataTracer, Log, TEXT("    HandleGetDataTableRow: BaseSymbolicRepresentation created: '%s'"), *BaseSymbolicRepresentation);

    bool bIsMainStructOutputPin = (OutputPin->Name == TEXT("Out Row"));
    if (!bIsMainStructOutputPin && OutputPin->Category == TEXT("struct") && !OutputPin->SubCategoryObject.IsEmpty())
    {
        bIsMainStructOutputPin = true; // Assuming if it's a struct type output, it's the main one
    }
    
    if (OutputPin->Category == TEXT("exec")) {
         UE_LOG(LogDataTracer, Error, TEXT("  HandleGetDataTableRow: Attempting to trace an EXEC pin '%s'. This handler is for DATA pins."), *OutputPin->Name);
         return FMarkdownSpan::Error(TEXT("[Tracing Exec Pin on GetDataTableRow]"));
    }

    FString Result;
    if (bIsMainStructOutputPin)
    {
        Result = BaseSymbolicRepresentation;
        UE_LOG(LogDataTracer, Log, TEXT("  HandleGetDataTableRow: OutputPin '%s' is main struct output. Result: '%s'"), *OutputPin->Name, *Result);
    }
    else
    {
        FString MemberName = OutputPin->FriendlyName.IsEmpty() ? OutputPin->Name : OutputPin->FriendlyName;
        Result = FString::Printf(TEXT("%s.%s"), *BaseSymbolicRepresentation, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *MemberName)));
        UE_LOG(LogDataTracer, Log, TEXT("  HandleGetDataTableRow: OutputPin '%s' is a member. Result: '%s'"), *OutputPin->Name, *Result);
    }
    return Result;
}