/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/Formatters_Events.cpp
#include "Formatters_Private.h" // Include the private declarations
#include "Logging/BP2AILog.h"

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Widgets/SMarkdownOutputWindow.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "Logging/LogMacros.h"


namespace MarkdownNodeFormatters_Private
{
    // Combined handler for various event/input types
    // FULL FUNCTION REPLACEMENT
    FString FormatEvent(
        TSharedPtr<const FBlueprintNode> Node,
        const TOptional<FCapturedEventData>& CapturedData, 
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData, // Parameter received
        const FString& CurrentBlueprintContext // Parameter received
    )
    {
        FString EventNameDisplay = TEXT("Unknown Event");
        FString Keyword = FMarkdownSpan::Keyword(TEXT("Event"));
        FString EventArgsStr = TEXT("");

        if (!Node.IsValid()) {
            UE_LOG(LogFormatter, Error, TEXT("FormatEvent called with invalid Node pointer!"));
            return FMarkdownSpan::Error(TEXT("[Invalid Node in FormatEvent]"));
        }

        const FString& NodeType = Node->NodeType;

        // Get Event Output Parameters (Only for certain non-bound event types)
        // This section does not use bSymbolicTraceForData as it's only formatting the *signature*, not tracing values.
        if (NodeType != TEXT("ComponentBoundEvent") && NodeType != TEXT("ActorBoundEvent") && NodeType != TEXT("FunctionEntry"))
        {
            TArray<FString> ArgsList;
            for (const TSharedPtr<FBlueprintPin>& Pin : Node->GetOutputPins(TEXT(""), false)) {
                if (Pin.IsValid() && !Pin->IsExecution()) {
                    FString PinTypeSig = Pin->GetTypeSignature();
                    // Avoid adding empty/delegate types to the Args list
                    if (!PinTypeSig.IsEmpty() && PinTypeSig.ToLower() != TEXT("delegate") && PinTypeSig.ToLower() != TEXT("none") && !Pin->Name.IsEmpty()) {
                        FString PinTypeSpan = FMarkdownSpan::DataType(FString::Printf(TEXT(":%s"), *PinTypeSig));
                        ArgsList.Add(FString::Printf(TEXT("%s%s"), *FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"), *Pin->Name)), *PinTypeSpan));
                    }
                }
            }
            EventArgsStr = ArgsList.Num() > 0 ? FString::Printf(TEXT(" Args: (%s)"), *FString::Join(ArgsList, TEXT(", "))) : TEXT("");
        }

        // Determine Name and Keyword based on NodeType
        if (NodeType == TEXT("ComponentBoundEvent"))
        {
            Keyword = FMarkdownSpan::Keyword(TEXT("Bound Event"));
            FString DelegateNameOrDefault = TEXT("?Delegate?");
            FString CompNameOrDefault = TEXT("?Component?");
            FString OwnerClassOrDefault = TEXT("?Owner?");

            if (CapturedData.IsSet())
            {
                const FCapturedEventData& Data = CapturedData.GetValue();
                DelegateNameOrDefault = Data.PreservedDelPropName.IsEmpty() ? DelegateNameOrDefault : Data.PreservedDelPropName;
                CompNameOrDefault = Data.PreservedCompPropName.IsEmpty() ? CompNameOrDefault : Data.PreservedCompPropName;
                FString ExtractedOwner = MarkdownTracerUtils::ExtractSimpleNameFromPath(Data.BoundEventOwnerClassPath); // Context is not needed here as path is assumed absolute
                OwnerClassOrDefault = ExtractedOwner.IsEmpty() ? OwnerClassOrDefault : ExtractedOwner; 
            }
            else 
            {
                DelegateNameOrDefault = Node->PreservedDelPropName.IsEmpty() ? DelegateNameOrDefault : Node->PreservedDelPropName;
                CompNameOrDefault = Node->PreservedCompPropName.IsEmpty() ? CompNameOrDefault : Node->PreservedCompPropName;
                FString ExtractedOwner = MarkdownTracerUtils::ExtractSimpleNameFromPath(Node->BoundEventOwnerClassPath); // Context is not needed here as path is assumed absolute
                OwnerClassOrDefault = ExtractedOwner.IsEmpty() ? OwnerClassOrDefault : ExtractedOwner;
            }

            EventNameDisplay = FString::Printf(TEXT("%s (%s on %s)"),
                *FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateNameOrDefault)),
                *FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *CompNameOrDefault)),
                *FMarkdownSpan::ClassName(FString::Printf(TEXT("%s"), *OwnerClassOrDefault))
            );
            EventArgsStr = TEXT(""); 
        }
        else if (NodeType == TEXT("ActorBoundEvent"))
        {
            Keyword = FMarkdownSpan::Keyword(TEXT("Actor Bound Event"));
            FString DelegateNameOrDefault = TEXT("?Delegate?");
            FString OwnerClassOrDefault = TEXT("?Owner?");

            if (CapturedData.IsSet())
            {
                const FCapturedEventData& Data = CapturedData.GetValue();
                DelegateNameOrDefault = Data.PreservedDelPropName.IsEmpty() ? DelegateNameOrDefault : Data.PreservedDelPropName;
                FString ExtractedOwner = MarkdownTracerUtils::ExtractSimpleNameFromPath(Data.BoundEventOwnerClassPath); // Context is not needed here as path is assumed absolute
                OwnerClassOrDefault = ExtractedOwner.IsEmpty() ? OwnerClassOrDefault : ExtractedOwner;
            }
            else 
            {
                DelegateNameOrDefault = Node->PreservedDelPropName.IsEmpty() ? DelegateNameOrDefault : Node->PreservedDelPropName;
                FString ExtractedOwner = MarkdownTracerUtils::ExtractSimpleNameFromPath(Node->BoundEventOwnerClassPath); // Context is not needed here as path is assumed absolute
                OwnerClassOrDefault = ExtractedOwner.IsEmpty() ? OwnerClassOrDefault : ExtractedOwner;
            }

            EventNameDisplay = FString::Printf(TEXT("%s (on Actor of type %s)"),
                *FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateNameOrDefault)),
                *FMarkdownSpan::ClassName(FString::Printf(TEXT("%s"), *OwnerClassOrDefault))
            );
            EventArgsStr = TEXT("");
        }
        else 
        {
            const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("EventFunctionName"));
            const FString* CustomNamePtr = Node->RawProperties.Find(TEXT("CustomFunctionName"));
            const FString* ActionNamePtr = Node->RawProperties.Find(TEXT("InputActionName")); 
            const FString* AxisNamePtr = Node->RawProperties.Find(TEXT("InputAxisName"));
            const FString* KeyNamePtr = Node->RawProperties.Find(TEXT("InputKey")); 

            FString TempName = TEXT("Event"); 

            if (FuncNamePtr && !FuncNamePtr->IsEmpty()) { TempName = *FuncNamePtr; }
            else if (CustomNamePtr && !CustomNamePtr->IsEmpty()) { TempName = *CustomNamePtr; }
            else if (ActionNamePtr && !ActionNamePtr->IsEmpty()) {
                if (ActionNamePtr->Contains(TEXT("/"))) { TempName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*ActionNamePtr); } // No context needed here
                else { TempName = *ActionNamePtr; }
            }
            else if (AxisNamePtr && !AxisNamePtr->IsEmpty()) { TempName = *AxisNamePtr; }
            else if (KeyNamePtr && !KeyNamePtr->IsEmpty()) { TempName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*KeyNamePtr); } // No context needed here
            else if (!Node->Name.IsEmpty()) { TempName = Node->Name; } 
            else { TempName = Node->NodeType; } 


            if(TempName == TEXT("ReceiveBeginPlay")) TempName = TEXT("BeginPlay");
            else if(TempName == TEXT("ReceiveTick")) TempName = TEXT("Tick");
            else if(TempName == TEXT("ReceiveAnyDamage")) TempName = TEXT("AnyDamage");
            else if(TempName == TEXT("ReceiveEndPlay")) TempName = TEXT("EndPlay");
            else if(TempName == TEXT("ReceiveDestroyed")) TempName = TEXT("Destroyed");
            else if(TempName == TEXT("OnTakeAnyDamage")) TempName = TEXT("TakeAnyDamage"); 
            else if(TempName == TEXT("ReceiveDrawHUD")) TempName = TEXT("DrawHUD");

            EventNameDisplay = FMarkdownSpan::EventName(FString::Printf(TEXT("%s"), *TempName));
        }

        FString FinalFormattedString = FString::Printf(TEXT("%s %s%s"), *Keyword, *EventNameDisplay, *EventArgsStr);
        return FinalFormattedString;
    }


	// FULL FUNCTION REPLACEMENT
    FString FormatTimeline(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData, // Parameter received
        const FString& CurrentBlueprintContext // Parameter received, not used by this formatter
    )
    {
        const FString TimelineName = Node->RawProperties.FindRef(TEXT("TimelineName"));
        FString NameOrDefault = TimelineName.IsEmpty() ? Node->Name : TimelineName;
        if (NameOrDefault.IsEmpty()) NameOrDefault = TEXT("Timeline");

        FString Keyword = FMarkdownSpan::Keyword(TEXT("Timeline"));
        FString NameSpan = FMarkdownSpan::TimelineName(FString::Printf(TEXT("%s"), *NameOrDefault));
        return FString::Printf(TEXT("%s: %s"), *Keyword, *NameSpan);
    }

	// FULL FUNCTION REPLACEMENT
    FString FormatDelegateBinding(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData,
        const FString& CurrentBlueprintContext
    )
    {
        FString ActionKeyword;
        if(Node->NodeType == TEXT("AddDelegate")) ActionKeyword = TEXT("Bind");
        else if(Node->NodeType == TEXT("AssignDelegate")) ActionKeyword = TEXT("Assign");
        else if(Node->NodeType == TEXT("RemoveDelegate")) ActionKeyword = TEXT("Unbind");
        else ActionKeyword = TEXT("Delegate Op"); // Fallback

        // Find the delegate *input* pin (the event/function being bound)
        TSharedPtr<const FBlueprintPin> DelegateInputPin = Node->GetPin(TEXT("Delegate"), TEXT("EGPD_Input"));
        FString EventToBindStr = FMarkdownSpan::Error(TEXT("*(Missing Delegate Source)*"));
        if(DelegateInputPin.IsValid())
        {
            // Pass bSymbolicTraceForData and CurrentBlueprintContext
            EventToBindStr = DataTracer.TracePinValue(DelegateInputPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
        }

        // Find the delegate *property* pin (the multicast delegate variable on the target object)
        TSharedPtr<const FBlueprintPin> DelegatePropertyPin = nullptr;
        for(const auto& Pair : Node->Pins)
        {
            if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && Pair.Value->Category.EndsWith(TEXT("Delegate")) && Pair.Value->Name != TEXT("Delegate"))
            {
                DelegatePropertyPin = Pair.Value;
                break;
            }
        }
        FString DelegatePropName = DelegatePropertyPin.IsValid() ? DelegatePropertyPin->Name : Node->RawProperties.FindRef(TEXT("DelegateReference_MemberName")); 
        FString DelegateNameOrDefault = DelegatePropName.IsEmpty() ? TEXT("?Delegate?") : DelegatePropName;

        // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
        FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext);
        FString Keyword = FMarkdownSpan::Keyword(FString::Printf(TEXT("%s"), *ActionKeyword));
        FString DelegateNameSpan = FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateNameOrDefault));

        if (ActionKeyword == TEXT("Assign"))
        {
            return FString::Printf(TEXT("%s %s = %s%s"), *Keyword, *DelegateNameSpan, *EventToBindStr, *TargetStr);
        }
        else // Bind or Unbind
        {
            return FString::Printf(TEXT("%s %s to %s%s"), *Keyword, *EventToBindStr, *DelegateNameSpan, *TargetStr);
        }
    }

	// FULL FUNCTION REPLACEMENT
    FString FormatClearDelegate(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData, // Parameter received
        const FString& CurrentBlueprintContext // Context passed
    )
    {
        TSharedPtr<const FBlueprintPin> DelegatePropertyPin = nullptr;
        for(const auto& Pair : Node->Pins)
        {
            if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && Pair.Value->Category.EndsWith(TEXT("Delegate")))
            {
                DelegatePropertyPin = Pair.Value;
                break;
            }
        }
        FString DelegatePropName = DelegatePropertyPin.IsValid() ? DelegatePropertyPin->Name : Node->RawProperties.FindRef(TEXT("DelegateReference_MemberName")); 
        FString DelegateNameOrDefault = DelegatePropName.IsEmpty() ? TEXT("?Delegate?") : DelegatePropName;

        // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
        FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext);
        FString Keyword = FMarkdownSpan::Keyword(TEXT("Unbind All"));
        FString DelegateNameSpan = FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateNameOrDefault));

        return FString::Printf(TEXT("%s from %s%s"), *Keyword, *DelegateNameSpan, *TargetStr);
    }

	// FULL FUNCTION REPLACEMENT
    FString FormatCallDelegate(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData,
        const FString& CurrentBlueprintContext
    )
    {
        TSharedPtr<const FBlueprintPin> DelegateInputPin = Node->GetPin(TEXT("Delegate"), TEXT("EGPD_Input"));
        FString DelegateStr = FMarkdownSpan::Error(TEXT("?Delegate?"));
        if (DelegateInputPin.IsValid())
        {
            // Pass bSymbolicTraceForData and CurrentBlueprintContext when tracing the pin value
            DelegateStr = DataTracer.TracePinValue(DelegateInputPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
        }
        else
        {
            const FString DelegateName = Node->RawProperties.FindRef(TEXT("DelegateReference_MemberName"));
            if (!DelegateName.IsEmpty())
                DelegateStr = FMarkdownSpan::DelegateName(FString::Printf(TEXT("%s"), *DelegateName));
        }

        // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
        FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext);
        // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
        FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, {FName(TEXT("Delegate"))}, CurrentBlueprintContext);
        FString Keyword = FMarkdownSpan::Keyword(TEXT("Call Delegate"));

        return FString::Printf(TEXT("%s %s%s%s"), *Keyword, *DelegateStr, *ArgsStr, *TargetStr);
    }

	// FULL FUNCTION REPLACEMENT
    FString FormatComposite(
        TSharedPtr<const FBlueprintNode> Node,
        FMarkdownDataTracer& DataTracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
        TSet<FString>& VisitedDataPins,
        bool bSymbolicTraceForData, // Parameter received, not used
        const FString& CurrentBlueprintContext // Context received, not used
    )
    {
        const FString GraphName = Node->RawProperties.FindRef(TEXT("BoundGraphName"));
        FString NameOrDefault = GraphName.IsEmpty() ? Node->Name : GraphName;
        if (NameOrDefault.IsEmpty()) NameOrDefault = TEXT("Collapsed Graph");

        FString Keyword = FMarkdownSpan::Keyword(TEXT("Collapsed Graph"));
        FString NameSpan = FMarkdownSpan::MacroName(FString::Printf(TEXT("%s"), *NameOrDefault));
        return FString::Printf(TEXT("%s: %s"), *Keyword, *NameSpan);
    }

} // namespace MarkdownNodeFormatters_Private