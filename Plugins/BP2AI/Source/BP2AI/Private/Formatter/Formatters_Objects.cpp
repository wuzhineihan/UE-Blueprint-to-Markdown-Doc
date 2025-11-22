/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// PROVIDE FULL FILE
// Source/BP2AI/Private/Formatter/Formatters_Objects.cpp
#include "Formatters_Private.h"

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "Logging/LogMacros.h"

namespace MarkdownNodeFormatters_Private
{
	FString FormatSpawnActor( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
		TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("Class"), TEXT("EGPD_Input"));
		FString ClassName = FMarkdownSpan::Error(TEXT("?Class?"));
		if (ClassPin.IsValid()) { 
            // Pass bSymbolicTraceForData and CurrentBlueprintContext
            ClassName = DataTracer.TracePinValue(ClassPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext); 
        }
        else { 
             const FString* ClassPath = Node->RawProperties.Find(TEXT("ClassToSpawn"));
             // Context is not needed for absolute path simplification
             if (ClassPath) ClassName = FMarkdownSpan::LiteralObject(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath)));
        }

		TSharedPtr<const FBlueprintPin> SpawnTransformPin = Node->GetPin(TEXT("SpawnTransform"), TEXT("EGPD_Input"));
		FString SpawnTransformStr = FMarkdownSpan::LiteralStructVal(TEXT("DefaultTransform"));
        if(SpawnTransformPin.IsValid() && !MarkdownTracerUtils::IsTrivialDefault(SpawnTransformPin)) {
             // Pass bSymbolicTraceForData and CurrentBlueprintContext
             SpawnTransformStr = DataTracer.TracePinValue(SpawnTransformPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
        }

		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString OtherArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, { FName(TEXT("Class")), FName(TEXT("SpawnTransform")) }, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Spawn Actor"));
		FString ClassNameSpan = FMarkdownSpan::ClassName(ClassName); 
		return FString::Printf(TEXT("%s %s at (%s)%s"), *Keyword, *ClassNameSpan, *SpawnTransformStr, *OtherArgsStr);
	}

	FString FormatAddComponent( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
    {
		FString CompName = FMarkdownSpan::Error(TEXT("?Component?"));
        TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("ComponentClass"), TEXT("EGPD_Input")); 
         if (!ClassPin) ClassPin = Node->GetPin(TEXT("Return Value Class")); 

        if (ClassPin.IsValid()) {
             // Pass bSymbolicTraceForData and CurrentBlueprintContext
             CompName = DataTracer.TracePinValue(ClassPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
             if (CompName == FMarkdownSpan::LiteralObject(TEXT("None")) || CompName.Contains(TEXT("Error"))) {
                 const FString* ClassPath = Node->RawProperties.Find(TEXT("ComponentClass"));
                 if (!ClassPath) ClassPath = Node->RawProperties.Find(TEXT("TemplateBlueprint")); 
                 // Context is not needed for absolute path simplification
                 if (ClassPath) CompName = FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath)));
             }
        } else { 
            const FString* ClassPath = Node->RawProperties.Find(TEXT("ComponentClass"));
            if (!ClassPath) ClassPath = Node->RawProperties.Find(TEXT("TemplateBlueprint"));
            // Context is not needed for absolute path simplification
            if (ClassPath) CompName = FMarkdownSpan::ComponentName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath)));
        }

        // Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
        FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext); 
        TSet<FName> Exclusions = { FName(TEXT("ComponentClass")), FName(TEXT("ReturnValueClass")), FName(TEXT("Target")), FName(TEXT("self")) };
		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString OtherArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Add Component"));
		FString CompNameSpan = CompName; 
		return FString::Printf(TEXT("%s %s%s%s"), *Keyword, *CompNameSpan, *TargetStr, *OtherArgsStr);
	}

	FString FormatCreateWidget( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
    {
		TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("Class"), TEXT("EGPD_Input"));
		FString WidgetName = FMarkdownSpan::Error(TEXT("?Widget?"));
        if (ClassPin.IsValid()) { 
            // Pass bSymbolicTraceForData and CurrentBlueprintContext
            WidgetName = DataTracer.TracePinValue(ClassPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext); 
        }
        else { 
            const FString* ClassPath = Node->RawProperties.Find(TEXT("WidgetClass"));
            // Context is not needed for absolute path simplification
            if (ClassPath) WidgetName = FMarkdownSpan::WidgetName(FString::Printf(TEXT("%s"), *MarkdownTracerUtils::ExtractSimpleNameFromPath(*ClassPath)));
        }

        TSharedPtr<const FBlueprintPin> OwnerPin = Node->GetPin(TEXT("OwningPlayer"), TEXT("EGPD_Input"));
		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString OwnerStr = OwnerPin.IsValid() ? DataTracer.TracePinValue(OwnerPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Variable(TEXT("DefaultPlayer"));

		TSet<FName> Exclusions = { FName(TEXT("Class")), FName(TEXT("OwningPlayer")) };
		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString OtherArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Create Widget"));
		FString WidgetNameSpan = WidgetName; 
		return FString::Printf(TEXT("%s %s for (%s)%s"), *Keyword, *WidgetNameSpan, *OwnerStr, *OtherArgsStr);
	}

	FString FormatGenericCreateObject( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
    {
        TSharedPtr<const FBlueprintPin> ClassPin = Node->GetPin(TEXT("Class"), TEXT("EGPD_Input"));
		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString ClassName = ClassPin.IsValid() ? DataTracer.TracePinValue(ClassPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("?Class?"));

        TSharedPtr<const FBlueprintPin> OuterPin = Node->GetPin(TEXT("Outer"), TEXT("EGPD_Input"));
		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString OuterStr = OuterPin.IsValid() ? DataTracer.TracePinValue(OuterPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Variable(TEXT("DefaultOuter"));

		TSet<FName> Exclusions = { FName(TEXT("Class")), FName(TEXT("Outer")) };
		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatArguments
		FString OtherArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Create Object"));
		FString ClassNameSpan = ClassName; 
		return FString::Printf(TEXT("%s %s Outer=(%s)%s"), *Keyword, *ClassNameSpan, *OuterStr, *OtherArgsStr);
	}

	FString FormatDynamicCast( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
    {
		TSharedPtr<const FBlueprintPin> ObjectPin = Node->GetPin(TEXT("ObjectToCast"), TEXT("EGPD_Input"));
        if (!ObjectPin) ObjectPin = Node->GetPin(TEXT("Object"), TEXT("EGPD_Input"));
		// Pass bSymbolicTraceForData and CurrentBlueprintContext
		FString ObjectStr = ObjectPin.IsValid() ? DataTracer.TracePinValue(ObjectPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("<?>"));
        
        FString TargetTypeName = TEXT("UnknownType");
        const FString* TargetTypePathProp = Node->RawProperties.Find(TEXT("TargetType"));
         if (TargetTypePathProp && !TargetTypePathProp->IsEmpty()){ TargetTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(*TargetTypePathProp); } // No context needed here
        if (TargetTypeName == TEXT("UnknownType") || TargetTypeName == TEXT("Object")) { 
             for(const auto& Pair : Node->Pins){ if(Pair.Value.IsValid() && Pair.Value->IsOutput() && Pair.Value->Category == TEXT("object")){ TargetTypeName = Pair.Value->GetTypeSignature(); break; }}
             TargetTypeName.RemoveFromEnd(TEXT("&")); 
        }
        if (TargetTypeName.IsEmpty()) TargetTypeName = TEXT("UnknownType");

		FString Keyword = FMarkdownSpan::Keyword(TEXT("Cast"));
		FString CastTypeSpan = FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *TargetTypeName));
		return FString::Printf(TEXT("%s (%s) To %s"), *Keyword, *ObjectStr, *CastTypeSpan);
	}

} // namespace MarkdownNodeFormatters_Private