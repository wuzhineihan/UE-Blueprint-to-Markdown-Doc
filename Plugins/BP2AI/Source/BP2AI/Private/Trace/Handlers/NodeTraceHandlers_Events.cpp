/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Events.cpp



#include "NodeTraceHandlers_Events.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"


//----------------------------------------------------------------//
// Event Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Events::HandleEventOutputParam( // Renamed from HandleEvent
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
    check(Node.IsValid());
    check(Tracer);
    check(OutputPin.IsValid());
    // This handler assumes the node is an event-like node based on registration
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEventOutputParam: Processing node %s (%s) for output pin %s"), *Node->NodeType, *Node->Guid, *OutputPin->Name);

    FString EventName = TEXT("Event"); // Default
    FString EventQualifier = TEXT(""); // For bound events

    if (Node->NodeType == TEXT("ComponentBoundEvent"))
    {
        const FString* CompNamePtr = Node->RawProperties.Find(TEXT("ComponentPropertyName"));
        const FString* OwnerClassPathPtr = Node->RawProperties.Find(TEXT("DelegateOwnerClass"));

        FString CompName = CompNamePtr ? **CompNamePtr : TEXT("UnknownComp");
        FString OwnerClassName = OwnerClassPathPtr ? MarkdownTracerUtils::ExtractSimpleNameFromPath(*OwnerClassPathPtr) : TEXT("Owner");
        
        FString DelegateSignatureName = TEXT("UnknownDelegate");
        if (!Node->Name.IsEmpty())
        {
             int32 ParenPos = Node->Name.Find(TEXT("("));
             if (ParenPos != INDEX_NONE)
             {
                 DelegateSignatureName = Node->Name.Left(ParenPos).TrimEnd();
                 if (DelegateSignatureName.Equals(TEXT("ComponentBeginOverlapSignature__DelegateSignature"), ESearchCase::IgnoreCase)) { DelegateSignatureName = TEXT("OnComponentBeginOverlap"); }
                 else if (DelegateSignatureName.Equals(TEXT("ComponentEndOverlapSignature__DelegateSignature"), ESearchCase::IgnoreCase)) { DelegateSignatureName = TEXT("OnComponentEndOverlap"); }
                 else if (DelegateSignatureName.Equals(TEXT("ComponentHitSignature__DelegateSignature"), ESearchCase::IgnoreCase)) { DelegateSignatureName = TEXT("OnComponentHit"); }
                 else if (DelegateSignatureName.Equals(TEXT("ComponentEndTouchOverSignature__DelegateSignature"), ESearchCase::IgnoreCase)) { DelegateSignatureName = TEXT("OnInputTouchLeave"); }
             }
             else { DelegateSignatureName = Node->Name; }
             UE_LOG(LogDataTracer, Log, TEXT("  HandleEventOutputParam (CompBound): Parsed DelegateSignatureName='%s' from Node->Name='%s'"), *DelegateSignatureName, *Node->Name);
        }
         else {
              const FString* DelegateNamePropPtr = Node->RawProperties.Find(TEXT("DelegatePropertyName"));
              DelegateSignatureName = DelegateNamePropPtr ? MarkdownTracerUtils::ExtractSimpleNameFromPath(**DelegateNamePropPtr) : TEXT("UnknownDelegate");
              UE_LOG(LogDataTracer, Warning, TEXT("  HandleEventOutputParam (CompBound): Node->Name is empty, falling back to DelegatePropertyName: '%s'"), *DelegateSignatureName);
         }
        
        EventQualifier = FString::Printf(TEXT(" (%s on %s)"),
            *FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *CompName)),
            *FMarkdownSpan::ClassName(FString::Printf(TEXT("%s"), *OwnerClassName))
        );
        EventName = FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateSignatureName));
    }
    else if (Node->NodeType == TEXT("ActorBoundEvent"))
    {
         FString DelegateSignatureName = TEXT("UnknownDelegate");
         if (!Node->Name.IsEmpty())
         {
              int32 ParenPos = Node->Name.Find(TEXT("("));
              if (ParenPos != INDEX_NONE) { DelegateSignatureName = Node->Name.Left(ParenPos).TrimEnd(); }
              else { DelegateSignatureName = Node->Name; }
              UE_LOG(LogDataTracer, Log, TEXT("  HandleEventOutputParam (ActorBound): Parsed DelegateSignatureName='%s' from Node->Name='%s'"), *DelegateSignatureName, *Node->Name);
         }
          else {
              const FString* DelegateNamePropPtr = Node->RawProperties.Find(TEXT("DelegatePropertyName"));
              DelegateSignatureName = DelegateNamePropPtr ? MarkdownTracerUtils::ExtractSimpleNameFromPath(**DelegateNamePropPtr) : TEXT("UnknownDelegate");
              UE_LOG(LogDataTracer, Warning, TEXT("  HandleEventOutputParam (ActorBound): Node->Name is empty, falling back to DelegatePropertyName: '%s'"), *DelegateSignatureName);
         }
         EventQualifier = TEXT(" (Actor Bound)");
         EventName = FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateSignatureName));
    }
    else
    {
        const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("EventFunctionName"));
        const FString* CustomNamePtr = Node->RawProperties.Find(TEXT("CustomFunctionName"));
        const FString* ActionNamePtr = Node->RawProperties.Find(TEXT("InputActionName"));
        const FString* AxisNamePtr = Node->RawProperties.Find(TEXT("InputAxisName"));
        const FString* KeyNamePtr = Node->RawProperties.Find(TEXT("InputKey"));

        if (FuncNamePtr && !FuncNamePtr->IsEmpty()) { EventName = *FuncNamePtr; }
        else if (CustomNamePtr && !CustomNamePtr->IsEmpty()) { EventName = *CustomNamePtr; }
        else if (ActionNamePtr && !ActionNamePtr->IsEmpty()) {
             if (ActionNamePtr->Contains(TEXT("/"))) { EventName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*ActionNamePtr); }
             else { EventName = *ActionNamePtr; }
        }
        else if (AxisNamePtr && !AxisNamePtr->IsEmpty()) { EventName = *AxisNamePtr; }
        else if (KeyNamePtr && !KeyNamePtr->IsEmpty()) { EventName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*KeyNamePtr); }
        else if (!Node->Name.IsEmpty()) { EventName = Node->Name; }
        else { EventName = Node->NodeType; }

        if(EventName == TEXT("ReceiveBeginPlay")) EventName = TEXT("BeginPlay");
        else if(EventName == TEXT("ReceiveTick")) EventName = TEXT("Tick");
        // Add more mappings
    	FString BaseEventName = EventName;
    	
        EventName = FMarkdownSpan::Variable(FString::Printf(TEXT("%s"), *BaseEventName));
    }

    FString ParamName = OutputPin->Name;
    FString FormattedString = EventName + EventQualifier + TEXT(".") + FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"), *ParamName));

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEventOutputParam: Returning '%s'"), *FormattedString);
    return FormattedString;
}
