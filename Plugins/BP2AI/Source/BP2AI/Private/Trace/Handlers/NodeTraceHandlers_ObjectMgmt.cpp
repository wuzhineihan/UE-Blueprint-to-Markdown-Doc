/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_ObjectMgmt.cpp


#include "NodeTraceHandlers_ObjectMgmt.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_ObjectMgmt::HandleSpawnActor(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
    )
{
    check(Node.IsValid() && Node->NodeType == TEXT("SpawnActorFromClass")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSpawnActor: Processing node %s"), *Node->Guid);

    if (OutputPin->Name == TEXT("ReturnValue"))
    {
        TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("Class"));
        FString ClassNameStr = FMarkdownSpan::Error(TEXT("?Class?"));
        if (ClassPin.IsValid())
        {
            ClassNameStr = Tracer->ResolvePinValueRecursive(ClassPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
        }
        else
        {
             const FString* ClassPath = Node->RawProperties.Find(TEXT("ClassToSpawn"));
             ClassNameStr = ClassPath ? FMarkdownSpan::LiteralObject(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath))) : ClassNameStr;
        }
        return FMarkdownSpan::FunctionName(TEXT("SpawnedActor")) + FString::Printf(TEXT("(%s)"), *ClassNameStr);
    }
    return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(SpawnActor.%s)"), *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
}

FString FNodeTraceHandlers_ObjectMgmt::HandleAddComponent(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
    )
{
    check(Node.IsValid() && Node->NodeType == TEXT("AddComponent")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleAddComponent: Processing node %s"), *Node->Guid);

    if (OutputPin->Name == TEXT("ReturnValue"))
    {
        FString CompNameStr = FMarkdownSpan::Error(TEXT("?Component?"));
        const FString* ClassPath = Node->RawProperties.Find(TEXT("ComponentClass"));
        if (!ClassPath) ClassPath = Node->RawProperties.Find(TEXT("TemplateBlueprint"));

        if (ClassPath)
        {
            CompNameStr = FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath)));
        }
        else
        {
             TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("ComponentClass")); // Check common pin name
             if (!ClassPin) ClassPin = Node->GetPin(TEXT("Return Value Class")); // Fallback

             if(ClassPin.IsValid())
             {
                // If pin is linked, trace it; otherwise, try its default or subcategory object
                if (ClassPin->SourcePinFor.Num() > 0) {
                     CompNameStr = Tracer->ResolvePinValueRecursive(ClassPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
                } else if (!ClassPin->DefaultObject.IsEmpty()) {
                    CompNameStr = MarkdownFormattingUtils::FormatLiteralValue(ClassPin, ClassPin->DefaultObject, Tracer);
                } else if (!ClassPin->SubCategoryObject.IsEmpty()){
                    CompNameStr = FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(ClassPin->SubCategoryObject)));
                }
             }
        }
        return FMarkdownSpan::Keyword(TEXT("AddedComponent")) + FString::Printf(TEXT("(%s)"), *CompNameStr);
    }
    return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(AddComponent.%s)"), *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
}

FString FNodeTraceHandlers_ObjectMgmt::HandleCreateWidget(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
    )
{
    check(Node.IsValid() && Node->NodeType == TEXT("CreateWidget")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCreateWidget: Processing node %s"), *Node->Guid);

    if (OutputPin->Name == TEXT("ReturnValue"))
    {
        TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("Class"));
        FString ClassNameStr = FMarkdownSpan::Error(TEXT("?Widget?"));
        if (ClassPin.IsValid()) {
            ClassNameStr = Tracer->ResolvePinValueRecursive(ClassPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
        }
        else {
            const FString* ClassPath = Node->RawProperties.Find(TEXT("WidgetClass"));
            ClassNameStr = ClassPath ? FMarkdownSpan::WidgetName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath))) : ClassNameStr;
        }
        return FMarkdownSpan::Keyword(TEXT("CreatedWidget")) + FString::Printf(TEXT("(%s)"), *ClassNameStr);
    }
     return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(CreateWidget.%s)"), *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
}

FString FNodeTraceHandlers_ObjectMgmt::HandleDynamicCast(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext)
{
    check(Node.IsValid() && Node->NodeType == TEXT("DynamicCast")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleDynamicCast: Processing node %s for pin %s"), *Node->Guid, *OutputPin->Name);

    TSharedPtr<const FBlueprintPin> ObjectPin = Node->GetPin(TEXT("ObjectToCast"));
    if (!ObjectPin.IsValid()) { ObjectPin = Node->GetPin(TEXT("Object")); }

    FString ObjectStr = ObjectPin.IsValid() ? Tracer->ResolvePinValueRecursive(ObjectPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("?Object?"));

    TSharedPtr<const FBlueprintPin> AsPin; // Find the 'As ...' pin
    for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsOutput() && Pair.Value->Category == TEXT("object")) { AsPin = Pair.Value; break; } }

    if (OutputPin == AsPin)
    {
         const FString* TargetTypePathProp = Node->RawProperties.Find(TEXT("TargetType"));
         FString TargetTypeName = TEXT("UnknownType");
         if (TargetTypePathProp && !TargetTypePathProp->IsEmpty()) { TargetTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*TargetTypePathProp); }
         else if(AsPin.IsValid() && !AsPin->SubCategoryObject.IsEmpty()) { TargetTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(AsPin->SubCategoryObject); }
         if(TargetTypeName.IsEmpty()) TargetTypeName = TEXT("UnknownType");
         return FMarkdownSpan::FunctionName(TEXT("Cast")) + FString::Printf(TEXT("<%s>(%s)"), *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *TargetTypeName)), *ObjectStr);
    }
    else if (OutputPin->Name == TEXT("Success"))
    {
         return FMarkdownSpan::Keyword(TEXT("CastSucceeded")) + FString::Printf(TEXT("(%s)"), *ObjectStr);
    }
    UE_LOG(LogDataTracer, Warning, TEXT("  HandleDynamicCast: Tracing unexpected output pin '%s'."), *OutputPin->Name);
    return FMarkdownSpan::Error(TEXT("[Invalid Cast Output Trace]"));
}

FString FNodeTraceHandlers_ObjectMgmt::HandleGetClassDefaults(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext)
{
    check(Node.IsValid() && Node->NodeType == TEXT("GetClassDefaults")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleGetClassDefaults: Processing node %s for pin %s"), *Node->Guid, *OutputPin->Name);

    FString ClassName = TEXT("UnknownClass");
    // Prioritize TargetClassPath from raw properties if available
    const FString* TargetClassPathProp = Node->RawProperties.Find(TEXT("TargetClassPath"));
    if (TargetClassPathProp && !TargetClassPathProp->IsEmpty())
    {
        ClassName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*TargetClassPathProp);
    }
    // Fallback to output pin's subcategory object
    else if (!OutputPin->SubCategoryObject.IsEmpty())
    {
         ClassName = MarkdownTracerUtils::ExtractSimpleNameFromPath(OutputPin->SubCategoryObject);
    }
    // Another fallback (less reliable for class name, might be struct name)
    else {
         const FString* ClassPathFromShowPin = Node->RawProperties.Find(TEXT("ShowPinForProperties")); // This might be a struct path for "ShowPinForProperties"
         if(ClassPathFromShowPin) ClassName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPathFromShowPin);
    }
    if(ClassName.IsEmpty()) ClassName = TEXT("UnknownClass");


    FString MemberName = OutputPin->Name; // The name of the output pin *is* the member name
    return FMarkdownSpan::ClassName(FString::Printf(TEXT("%s"), *ClassName))
           + TEXT("::") + TEXT("Default")
           + TEXT(".") + FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *MemberName));
}

FString FNodeTraceHandlers_ObjectMgmt::HandleGetSubsystem(
    TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin, FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
    )
{
    check(Node.IsValid()); check(Tracer); check(OutputPin.IsValid());
     UE_LOG(LogDataTracer, Verbose, TEXT("  HandleGetSubsystem: Processing node %s (%s)"), *Node->NodeType, *Node->Guid);

     if (OutputPin->Name == TEXT("ReturnValue"))
     {
        FString SubsystemName = TEXT("UnknownSubsystem");
        const FString* ClassPath = Node->RawProperties.Find(TEXT("CustomClass")); // Stored by factory for GetSubsystem nodes
        if (ClassPath && !ClassPath->IsEmpty())
        {
            SubsystemName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath);
        }
        else if (!OutputPin->SubCategoryObject.IsEmpty())
        {
             SubsystemName = MarkdownTracerUtils::ExtractSimpleNameFromPath(OutputPin->SubCategoryObject);
        }
        if(SubsystemName.IsEmpty()) SubsystemName = TEXT("UnknownSubsystem");


        FString TargetStr = TEXT("");
        TSharedPtr<const FBlueprintPin> TargetPin; // The context pin (e.g. PlayerController)
        if (Node->NodeType == TEXT("GetSubsystemFromPC"))
        {
            TargetPin = Node->GetPin(TEXT("PlayerController"));
        }
        // Add other GetSubsystem variants if they have context pins, e.g., GetWorldSubsystem takes WorldContextObject
        else if (Node->NodeType == TEXT("GetWorldSubsystem") || Node->NodeType == TEXT("GetGameInstanceSubsystem") || Node->NodeType == TEXT("GetEngineSubsystem") || Node->NodeType == TEXT("GetLocalPlayerSubsystem"))
        {
             TargetPin = Node->GetPin(TEXT("WorldContextObject")); // Common context pin
        }


        if (TargetPin.IsValid())
        {
            FString TargetValue = Tracer->ResolvePinValueRecursive(TargetPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
            // Only show context if it's not implicit self or None
            if (TargetValue != FMarkdownSpan::Variable(TEXT("`self`")) && TargetValue != FMarkdownSpan::LiteralObject(TEXT("`None`")) && !TargetValue.Contains(TEXT("Default__")))
            {
                TargetStr = FString::Printf(TEXT(" from (%s)"), *TargetValue);
            }
        }

        return FMarkdownSpan::Keyword(TEXT("GetSubsystem"))
               + FString::Printf(TEXT("(%s)%s"), *FMarkdownSpan::ClassName(FString::Printf(TEXT("%s"), *SubsystemName)), *TargetStr);
     }

     return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(%s.%s)"), *Node->NodeType, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
}
