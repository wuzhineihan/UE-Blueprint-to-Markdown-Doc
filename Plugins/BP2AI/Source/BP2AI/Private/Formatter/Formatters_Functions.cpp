/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/Formatters_Functions.cpp
#include "Formatters_Private.h"
#include "Logging/BP2AILog.h"

#include "Models/BlueprintNode.h"     
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h" 
#include "Internationalization/Regex.h" 
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Kismet2/BlueprintEditorUtils.h" 
#include "Trace/FMarkdownPathTracer.h"
#include "Logging/LogMacros.h"


namespace MarkdownNodeFormatters_Private
{
	FString FormatCallFunction( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext // Context received
	)
    {
		
        check(Node.IsValid()); 
        UE_LOG(LogFormatter, Verbose, TEXT("  FormatCallFunction: Node %s (%s), Symbolic=%d, Context='%s'"), *Node->NodeType, *Node->Guid, bSymbolicTraceForData, *CurrentBlueprintContext);

        const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
        if (!FuncNamePtr && Node->NodeType == TEXT("CallParentFunction")) FuncNamePtr = Node->RawProperties.Find(TEXT("SuperFunctionName"));
        const FString FuncName = FuncNamePtr ? **FuncNamePtr : FString(TEXT("")); 

        if (FuncName.IsEmpty()) return FMarkdownSpan::Error(TEXT("[Unknown Function Call]"));

        // Check if it's a type conversion function by name heuristic (HandleOperator also does this check)
        const FString NormalizedFuncName = MarkdownTracerUtils::NormalizeConversionName(FuncName, DataTracer.GetTypeConversionMap());
        if (DataTracer.GetTypeConversionMap().Contains(NormalizedFuncName))
        {
            UE_LOG(LogFormatter, Verbose, TEXT("  FormatCallFunction: Detected as TYPE CONVERSION (%s). Context: '%s'."), *FuncName, *CurrentBlueprintContext);
            // Delegate formatting to MarkdownFormattingUtils, which will handle RVR and context
            // Use &DataTracer as the Tracer* parameter
            return MarkdownFormattingUtils::FormatConversion(Node, &DataTracer, AllNodes, 0, VisitedDataPins, nullptr, nullptr, bSymbolicTraceForData, CurrentBlueprintContext);
        }
		UE_LOG(LogFormatter, Error, TEXT("🔧 DEBUG FUNCTION: Node='%s', FuncName='%s'"), *Node->Name, *FuncName);

		
        // Check if it's a common unary operator (like Not_PreBool, handled by HandleUnaryOperator)
        if (FuncName == TEXT("Not_PreBool")) {
            UE_LOG(LogFormatter, Verbose, TEXT("  FormatCallFunction: Detected as Unary Operator (Not_PreBool). Context: '%s'."), *FuncName, *CurrentBlueprintContext);
        	UE_LOG(LogFormatter, Error, TEXT("🔧 DEBUG FUNCTION: MATCHED Not_PreBool - routing to UnaryOperator"));

        	
        	// Delegate formatting to MarkdownFormattingUtils, which will handle RVR and context
             // Use &DataTracer as the Tracer* parameter
            return MarkdownFormattingUtils::FormatUnaryOperator(Node, nullptr, &DataTracer, AllNodes, 0, VisitedDataPins, nullptr, nullptr, bSymbolicTraceForData, CurrentBlueprintContext);
        }
        // Check if it's a common binary operator (like Percent, handled by HandleOperator)
        if (FuncName == TEXT("Percent_FloatFloat") || FuncName == TEXT("Percent_IntInt")) {
             UE_LOG(LogFormatter, Verbose, TEXT("  FormatCallFunction: Detected as Binary Operator (Percent). Context: '%s'."), *FuncName, *CurrentBlueprintContext);
        	UE_LOG(LogFormatter, Error, TEXT("🔧 DEBUG FUNCTION: MATCHED Percent - routing to Operator"));

        	// Delegate formatting to MarkdownFormattingUtils, which will handle RVR and context
              // Use &DataTracer as the Tracer* parameter
             return MarkdownFormattingUtils::FormatOperator(Node, nullptr, &DataTracer, AllNodes, 0, VisitedDataPins, nullptr, nullptr, bSymbolicTraceForData, CurrentBlueprintContext);
        }

		UE_LOG(LogFormatter, Error, TEXT("🔧 DEBUG FUNCTION: No operator match - using standard function formatting"));

        // Standard Function Call Formatting
        UE_LOG(LogFormatter, Verbose, TEXT("  FormatCallFunction: Standard formatting for function %s. Context: '%s'"), *FuncName, *CurrentBlueprintContext);

        TSharedPtr<const FBlueprintPin> TargetPin;
        for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsInput() && (Pair.Value->Name == TEXT("self") || Pair.Value->Name == TEXT("Target"))) { TargetPin = Pair.Value; break; } }
        // Pass CurrentBlueprintContext to TraceTargetPin
        FString TargetStr = DataTracer.TraceTargetPin(TargetPin, AllNodes, 0, VisitedDataPins, CurrentBlueprintContext); // VisitedDataPins is correct here
        TSet<FName> Exclusions; if (TargetPin.IsValid()) { Exclusions.Add(FName(*TargetPin->Name)); }
        
        // Pass CurrentBlueprintContext to FormatArguments
        FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext); // VisitedDataPins is correct here

        FString CallPrefix = TEXT("");
        if (!TargetStr.IsEmpty() && TargetStr != FMarkdownSpan::Variable(TEXT("self"))) {
            if (TargetStr.Contains(TEXT("<span")) || TargetStr.Contains(TEXT("(")) || TargetStr.Contains(TEXT(")"))) { CallPrefix = FString::Printf(TEXT("(%s)."), *TargetStr); }
            else { CallPrefix = TargetStr + TEXT("."); }
        }

        FString FuncNameSpan = FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName));
        FString BaseCall = FString::Printf(TEXT("%s%s(%s)"), *CallPrefix, *FuncNameSpan, *ArgsStr);

        bool bIsLatent = Node->RawProperties.Contains(TEXT("bIsLatent")) && Node->RawProperties.FindChecked(TEXT("bIsLatent")) == TEXT("true");
        FString LatentInfo = bIsLatent ? FMarkdownSpan::Modifier(TEXT(" [(Latent)]")) : TEXT("");


        return BaseCall + LatentInfo;
    }

	// FULL FUNCTION REPLACEMENT - Implementation restored and fixed calls
FString FormatMacroInstance( 
    TSharedPtr<const FBlueprintNode> Node, 
    FMarkdownDataTracer& DataTracer, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
    TSet<FString>& VisitedDataPins,
    bool bSymbolicTraceForData,
    const FString& CurrentBlueprintContext // Context received
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("MacroInstance"));
    
    UE_LOG(LogFormatter, Verbose, TEXT("  FormatMacroInstance: Node %s (%s), Symbolic=%d, Context='%s'"), *Node->Name, *Node->Guid, bSymbolicTraceForData, *CurrentBlueprintContext);

    const FString* MacroPathPtr = Node->RawProperties.Find(TEXT("MacroGraphReference"));
    // Pass CurrentBlueprintContext to ExtractSimpleNameFromPath
    FString SimpleMacroName = MacroPathPtr ? MarkdownTracerUtils::ExtractSimpleNameFromPath(**MacroPathPtr, CurrentBlueprintContext) : TEXT("UnknownMacro");
    if (SimpleMacroName.IsEmpty()) SimpleMacroName = Node->Name.IsEmpty() ? Node->NodeType : Node->Name;


    // Handle known standard engine macros that are executable and have specific formatting
     if (SimpleMacroName == TEXT("ForEachLoop") || SimpleMacroName == TEXT("ForEachLoopWithBreak") || SimpleMacroName == TEXT("ReverseForEachLoop"))
    {
        UE_LOG(LogFormatter, Log, TEXT("  FormatMacroInstance: Dispatching to FormatForEachLoop for node %s (SimpleMacroName: %s). Context: '%s'."), *Node->Guid, *SimpleMacroName, *CurrentBlueprintContext);
        // Pass VisitedDataPins and CurrentBlueprintContext
        return FormatForEachLoop(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext);
    }
    if (SimpleMacroName == TEXT("ForLoop") || SimpleMacroName == TEXT("ForLoopWithBreak"))
    {
        UE_LOG(LogFormatter, Log, TEXT("  FormatMacroInstance: Handling ForLoop/ForLoopWithBreak for node %s. Context: '%s'."), *Node->Guid, *CurrentBlueprintContext);
        TSharedPtr<const FBlueprintPin> FirstPinInstance = Node->GetPin(TEXT("FirstIndex"), TEXT("EGPD_Input"));
        TSharedPtr<const FBlueprintPin> LastPinInstance = Node->GetPin(TEXT("LastIndex"), TEXT("EGPD_Input"));

        FString FirstStr = FMarkdownSpan::Error(TEXT("?")); // Default to error string
        FString LastStr = FMarkdownSpan::Error(TEXT("?"));  // Default to error string

        if (FirstPinInstance.IsValid()) {
            FirstStr = DataTracer.TracePinValue(FirstPinInstance, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
        } else {
            UE_LOG(LogFormatter, Warning, TEXT("    FormatMacroInstance (ForLoop): 'FirstIndex' pin not found for node %s."), *Node->Guid);
        }
        if (LastPinInstance.IsValid()) {
            LastStr = DataTracer.TracePinValue(LastPinInstance, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext);
        } else {
            UE_LOG(LogFormatter, Warning, TEXT("    FormatMacroInstance (ForLoop): 'LastIndex' pin not found for node %s."), *Node->Guid);
        }
        
        FString Keyword = FMarkdownSpan::Keyword(TEXT("For Loop"));
        TSharedPtr<const FBlueprintPin> IndexPin = Node->GetPin(TEXT("Index"), TEXT("EGPD_Output"));
        FString IndexType = IndexPin.IsValid() ? IndexPin->GetTypeSignature() : TEXT("?"); // Default to ? if pin not found
        FString LoopVarStr = FString::Printf(TEXT("[%s:%s]"), 
            *FMarkdownSpan::ParamName(TEXT("Index")), 
            *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *IndexType))
        );
        return FString::Printf(TEXT("%s (%s to %s) %s"), *Keyword, *FirstStr, *LastStr, *LoopVarStr);
    }
    if (SimpleMacroName == TEXT("ForLoop") || SimpleMacroName == TEXT("ForLoopWithBreak"))
    {
         UE_LOG(LogFormatter, Error, TEXT("  FormatMacroInstance: Hit ForLoop branch for node %s. Should be handled in FormatNodeDescription."), *Node->Guid);
         // Pass VisitedDataPins and CurrentBlueprintContext
        return FormatForEachLoop(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext); // ForLoop format is similar, reusing
    }
    if (SimpleMacroName == TEXT("WhileLoop"))
    {
         UE_LOG(LogFormatter, Error, TEXT("  FormatMacroInstance: Hit WhileLoop branch for node %s. Should be handled in FormatNodeDescription."), *Node->Guid);
         TSharedPtr<const FBlueprintPin> CondPin = Node->GetPin(TEXT("Condition"), TEXT("EGPD_Input"));
         // Pass bSymbolicTraceForData and CurrentBlueprintContext
         FString CondStr = CondPin.IsValid() ? DataTracer.TracePinValue(CondPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : TEXT("?");
         FString Keyword = FMarkdownSpan::Keyword(TEXT("While Loop"));
         return FString::Printf(TEXT("%s (Condition=%s)"), *Keyword, *CondStr);
    }
    if (SimpleMacroName == TEXT("Gate")) { return FMarkdownSpan::Keyword(TEXT("Gate")); }
    if (SimpleMacroName == TEXT("MultiGate")) { return FMarkdownSpan::Keyword(TEXT("MultiGate")); }
    if (SimpleMacroName == TEXT("FlipFlop")) { return FMarkdownSpan::Keyword(TEXT("FlipFlop")); }
    if (SimpleMacroName == TEXT("DoOnce")) { return FMarkdownSpan::Keyword(TEXT("Do Once")); }
    if (SimpleMacroName == TEXT("DoN"))
    {
         TSharedPtr<const FBlueprintPin> NPin = Node->GetPin(TEXT("N"), TEXT("EGPD_Input"));
         // Pass bSymbolicTraceForData and CurrentBlueprintContext
         FString NStr = NPin.IsValid() ? DataTracer.TracePinValue(NPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : TEXT("?");
         FString Keyword = FMarkdownSpan::Keyword(TEXT("Do N"));
         return FString::Printf(TEXT("%s (N=%s)"), *Keyword, *NStr); 
    }
    if (SimpleMacroName == TEXT("IsValid"))
    {
         TSharedPtr<const FBlueprintPin> InputPin = Node->GetPin(TEXT("Input Object"), TEXT("EGPD_Input"));
         if (!InputPin) InputPin = Node->GetPin(TEXT("InputObject"), TEXT("EGPD_Input")); 
         // Pass bSymbolicTraceForData and CurrentBlueprintContext
         FString InputStr = InputPin.IsValid() ? DataTracer.TracePinValue(InputPin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext) : TEXT("?");
         FString Keyword = FMarkdownSpan::Keyword(TEXT("Is Valid"));
         return FString::Printf(TEXT("%s (%s)"), *Keyword, *InputStr);
    }


    // Default Macro Instance Formatting
    UE_LOG(LogFormatter, Verbose, TEXT("  FormatMacroInstance: Default formatting for macro %s. Context: '%s'"), *SimpleMacroName, *CurrentBlueprintContext);
    TSet<FName> Exclusions; 
    // Pass VisitedDataPins and CurrentBlueprintContext to FormatArguments
    FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext); 

    FString MacroNameSpan = FMarkdownSpan::MacroName(FString::Printf(TEXT("%s"), *SimpleMacroName));
    
    return FString::Printf(TEXT("%s%s"), *MacroNameSpan, *ArgsStr);
}


	// FULL FUNCTION REPLACEMENT - Implementation restored and fixed calls
FString MarkdownNodeFormatters_Private::FormatCallParentFunction( 
    TSharedPtr<const FBlueprintNode> Node, 
    FMarkdownDataTracer& DataTracer, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
    TSet<FString>& VisitedDataPins,
    bool bSymbolicTraceForData,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid()); 
    // Use Node->Name or Node->NodeType directly for logging
    UE_LOG(LogFormatter, Error, TEXT("FormatCallParentFunction BEGIN for Node '%s' (Node Type: '%s', GUID: %s). Context: '%s'"), 
        Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name, // Corrected logging
        *Node->NodeType,
        *Node->Guid.Left(8), 
        *CurrentBlueprintContext);

    // --- Log ALL RawProperties available on this FBlueprintNode model ---
    UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Dumping RawProperties for Node '%s':"), Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name);
    for (const auto& Pair : Node->RawProperties)
    {
        UE_LOG(LogFormatter, Error, TEXT("    RawProp: Key='%s', Value='%s'"), *Pair.Key, *Pair.Value);
    }
    UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: End RawProperties Dump for Node '%s'."), Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name);
    // --- End Log ALL RawProperties ---

    const FString ParentFuncName = Node->RawProperties.FindRef(TEXT("SuperFunctionName"));
    UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Retrieved 'SuperFunctionName' from RawProperties: '%s'"), *ParentFuncName);

    if (ParentFuncName.IsEmpty()) 
    {
        UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: 'SuperFunctionName' is EMPTY for Node '%s'. Falling back to FormatGeneric. THIS IS LIKELY AN ISSUE IN THE FACTORY OR NODE MODEL POPULATION."), Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name);
        return FormatGeneric(Node);
    }

    TSet<FName> Exclusions; 
	FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);
    
    FString ParentBlueprintNameForAnchor = TEXT("UnknownParentBP_FormatterFallback");
    const FString* ParentClassPathPtr = Node->RawProperties.Find(TEXT("FunctionParentClassPath"));

    if (ParentClassPathPtr && !ParentClassPathPtr->IsEmpty()) {
        UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Found 'FunctionParentClassPath': '%s'"), **ParentClassPathPtr);
        ParentBlueprintNameForAnchor = FPaths::GetBaseFilename(**ParentClassPathPtr); 
        if (ParentBlueprintNameForAnchor.EndsWith(TEXT("_C"))) {
            ParentBlueprintNameForAnchor.LeftChopInline(2);
        }
        UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Derived ParentBlueprintNameForAnchor: '%s'"), *ParentBlueprintNameForAnchor);
    } else {
        UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Could NOT find 'FunctionParentClassPath' in RawProperties for Node '%s'. Using fallback anchor name."), Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name);
    }

    FString UniqueHintForParentFunc = FString::Printf(TEXT("%s.%s"), *ParentBlueprintNameForAnchor, *ParentFuncName);
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueHintForParentFunc);
    UE_LOG(LogFormatter, Error, TEXT("  FormatCallParentFunction: Constructed UniqueHintForParentFunc: '%s', Sanitized AnchorName: '%s'"), *UniqueHintForParentFunc, *AnchorName);
    
    FString Keyword = FMarkdownSpan::Keyword(TEXT("Call Parent"));
    FString FuncNameLinked = FString::Printf(TEXT(" [`%s`](#%s)"), *ParentFuncName, *AnchorName);

    FString Result = FString::Printf(TEXT("%s%s%s"), *Keyword, *FuncNameLinked, *ArgsStr);
    UE_LOG(LogFormatter, Error, TEXT("FormatCallParentFunction END for Node '%s'. Returning: '%s'"), Node->Name.IsEmpty() ? *Node->NodeType : *Node->Name, *Result);
    return Result;
}
	
	// FULL FUNCTION REPLACEMENT - Implementation restored and fixed calls
	FString FormatReturnNode( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext 
	)
    {
        check(Node.IsValid()); 
		// Pass VisitedDataPins and CurrentBlueprintContext to FormatArguments
		FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, {}, CurrentBlueprintContext);
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Return"));
		return FString::Printf(TEXT("%s%s"), *Keyword, *ArgsStr);
	}

	// FULL FUNCTION REPLACEMENT - Implementation restored and fixed calls
	FString FormatLatentAction( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	 )
     {
        check(Node.IsValid()); 
		FString ActionName = Node->NodeType; 
		if (Node->NodeType == TEXT("Delay")) ActionName = TEXT("Delay");
        else if (Node->NodeType == TEXT("AIMoveTo")) ActionName = TEXT("AI MoveTo");
        
        TSet<FName> Exclusions = {FName(TEXT("LatentInfo")), FName(TEXT("WorldContextObject")), FName(TEXT("Completed"))};
        // Pass VisitedDataPins and CurrentBlueprintContext to FormatArguments
		FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext);

		FString Keyword = FMarkdownSpan::Keyword(TEXT("Latent Action"));
		FString ActionNameSpan = FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *ActionName));
		return FString::Printf(TEXT("%s %s%s"), *Keyword, *ActionNameSpan, *ArgsStr) + FMarkdownSpan::Modifier(TEXT(" [(Latent)]")); // Corrected missing ArgsStr in Printf
     }

	// FULL FUNCTION REPLACEMENT - Implementation restored and fixed calls
	FString FormatPlayMontage( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	 )
     {
        check(Node.IsValid()); 
		TSharedPtr<const FBlueprintPin> MontagePin = Node->GetPin(TEXT("MontageToPlay"));
		FString MontageStr = FMarkdownSpan::Error(TEXT("<?>"));
		if(MontagePin.IsValid()) { 
            // Pass bSymbolicTraceForData and CurrentBlueprintContext
            MontageStr = DataTracer.TracePinValue(MontagePin, AllNodes, bSymbolicTraceForData, CurrentBlueprintContext); 
        }

		// Pass bSymbolicTraceForData and CurrentBlueprintContext to FormatTarget
		FString TargetStr = FormatTarget(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, CurrentBlueprintContext); // VisitedDataPins is correct here
        TSet<FName> Exclusions = { FName(TEXT("MontageToPlay")), FName(TEXT("Target")), FName(TEXT("self")), FName(TEXT("InSkeletalMeshComponent")) };
        // Pass VisitedDataPins and CurrentBlueprintContext to FormatArguments
		FString ArgsStr = FormatArguments(Node, DataTracer, AllNodes, VisitedDataPins, bSymbolicTraceForData, Exclusions, CurrentBlueprintContext); // VisitedDataPins is correct here
		FString Keyword = FMarkdownSpan::Keyword(TEXT("Play Montage"));
		FString MontageNameSpan = FMarkdownSpan::MacroName(MontageStr); 
		return FString::Printf(TEXT("%s %s%s%s"), *Keyword, *MontageNameSpan, *TargetStr, *ArgsStr) + FMarkdownSpan::Modifier(TEXT(" [(Latent)]"));
	}

} // namespace MarkdownNodeFormatters_Private