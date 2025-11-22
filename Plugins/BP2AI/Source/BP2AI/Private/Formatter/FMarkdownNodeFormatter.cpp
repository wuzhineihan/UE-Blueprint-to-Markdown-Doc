/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Formatter/FMarkdownNodeFormatter.cpp
#include "Formatter/FMarkdownNodeFormatter.h" // Public header
#include "Formatters_Private.h"             // Private declarations

#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Logging/LogMacros.h"
#include "Internationalization/Regex.h"
#include "Trace/FMarkdownPathTracer.h"
#include "Logging/BP2AILog.h"

namespace MarkdownNodeFormatters_Private
	
{
	FString FormatArguments( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins, 
		bool bSymbolicTraceForData, 
		const TSet<FName>& ExcludePinNames, // Default value is handled by declaration
		const FString& CurrentBlueprintContext // New parameter
	)
	{
		TArray<FString> ArgsList;
		TSet<FName> FinalExclusions = {
			FName(TEXT("self")), FName(TEXT("Target")), 
			FName(TEXT("WorldContextObject")), FName(TEXT("__WorldContext")), 
			FName(TEXT("LatentInfo")),                                     
			FName(TEXT("execute")), FName(TEXT("exec")), FName(TEXT("then")), FName(TEXT("__then__")), 
			FName(TEXT("ReturnValue"))                                     
		};
		FinalExclusions.Append(ExcludePinNames); 

        bool bIsDebuggingParentCall = (Node->NodeType == TEXT("CallParentFunction") || Node->RawProperties.Contains(TEXT("SuperFunctionName")));
        if (bIsDebuggingParentCall)
        {
            UE_LOG(LogFormatter, Error, TEXT("FormatArguments DEBUG for Node '%s' (Type: %s, GUID: %s) - Context: '%s', ShowTrivialDefaultsGlobal: %s, bSymbolicTraceForData: %s"), 
                *Node->Name, 
                *Node->NodeType, 
                *Node->Guid.Left(8),
                *CurrentBlueprintContext, // Log the context
                DataTracer.bCurrentShowTrivialDefaultParams ? TEXT("true") : TEXT("false"),
                bSymbolicTraceForData ? TEXT("true") : TEXT("false"));
        }

		for (const TSharedPtr<FBlueprintPin>& Pin : Node->GetInputPins(TEXT(""), false, true)) 
		{
			if (!Pin.IsValid()) continue;

            if (bIsDebuggingParentCall)
            {
                UE_LOG(LogFormatter, Error, TEXT("  Pin Iteration: Name='%s' (Friendly: '%s'), Category='%s', Direction='%s', IsHidden=%s, IsAdvanced=%s"), 
                    *Pin->Name, 
                    *Pin->FriendlyName, 
                    *Pin->Category,
                    *Pin->Direction,
                    Pin->IsHidden() ? TEXT("true") : TEXT("false"),
                    Pin->IsAdvancedView() ? TEXT("true") : TEXT("false"));
            }

			const FName PinFName(*Pin->Name); 
			if (!FinalExclusions.Contains(PinFName))
			{
                if (bIsDebuggingParentCall)
                {
                    UE_LOG(LogFormatter, Error, TEXT("    Pin '%s' NOT in FinalExclusions."), *Pin->Name);
                }

				bool bIsLinked = Pin->SourcePinFor.Num() > 0;
				bool bHasExplicitDefault = !Pin->DefaultValue.IsEmpty() || 
										   !Pin->DefaultObject.IsEmpty() || 
										   Pin->DefaultStruct.Num() > 0;
                
                bool bIsPinTrivialDefault = MarkdownTracerUtils::IsTrivialDefault(Pin);

                if (bIsDebuggingParentCall)
                {
                    UE_LOG(LogFormatter, Error, TEXT("      bIsLinked: %s"), bIsLinked ? TEXT("true") : TEXT("false"));
                    UE_LOG(LogFormatter, Error, TEXT("      bHasExplicitDefault: %s, IsPinTrivialDefault: %s"), 
                        bHasExplicitDefault ? TEXT("true") : TEXT("false"),
                        bIsPinTrivialDefault ? TEXT("true") : TEXT("false"));
                }
                
				if (DataTracer.bCurrentShowTrivialDefaultParams || bIsLinked || (bHasExplicitDefault && !bIsPinTrivialDefault))
				{
                    if (bIsDebuggingParentCall)
                    {
                        UE_LOG(LogFormatter, Error, TEXT("      CONDITION MET (ShowAllGlobal || Linked || (ExplicitDefault && !TrivialDefault)). Pin '%s' WILL BE TRACED AND ADDED."), *Pin->Name);
                    }

					// Pass CurrentBlueprintContext to ResolvePinValueRecursive
					FString PinValue = DataTracer.ResolvePinValueRecursive(Pin, AllNodes, 0 /*Depth 0 for direct args*/, VisitedDataPins, nullptr, nullptr, bSymbolicTraceForData, CurrentBlueprintContext);
					
					FString DisplayPinName = (!Pin->FriendlyName.IsEmpty() && Pin->FriendlyName != Pin->Name) ? Pin->FriendlyName : Pin->Name;
					DisplayPinName.TrimStartAndEndInline(); 

					ArgsList.Add(FString::Printf(TEXT("%s=%s"),
						*FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"), *DisplayPinName)),
						*PinValue
					));
					UE_LOG(LogFormatter, Verbose, TEXT("FormatArguments (Private): Added Pin '%s' (Displayed as '%s') with TracedValue '%s' to ArgsList for Node '%s'. ShowAllDefaults=%s, Linked=%s, ExplicitDefault=%s, Context='%s'"), 
						*Pin->Name, *DisplayPinName, *PinValue, *Node->Name,
						DataTracer.bCurrentShowTrivialDefaultParams ? TEXT("true"):TEXT("false"), 
                        bIsLinked?TEXT("true"):TEXT("false"), 
                        bHasExplicitDefault?TEXT("true"):TEXT("false"),
                        *CurrentBlueprintContext);
				}
				else 
				{
                    if (bIsDebuggingParentCall)
                    {
                        UE_LOG(LogFormatter, Error, TEXT("      CONDITION NOT MET. Pin '%s' SKIPPED. IsTrivialDefault(Pin) result: %s."), 
                            *Pin->Name, 
                            bIsPinTrivialDefault ? TEXT("true") : TEXT("false"));
                    }
					UE_LOG(LogFormatter, Verbose, TEXT("FormatArguments (Private): SKIPPED Pin '%s' on Node '%s'. ShowAllDefaults=%s, Linked=%s, ExplicitDefault=%s, Context='%s'"),
						*Pin->Name, *Node->Name,
						DataTracer.bCurrentShowTrivialDefaultParams ? TEXT("true"):TEXT("false"), 
                        bIsLinked?TEXT("true"):TEXT("false"), 
                        bHasExplicitDefault?TEXT("true"):TEXT("false"),
                        *CurrentBlueprintContext);
				}
			}
            else 
            {
                if (bIsDebuggingParentCall)
                {
                    UE_LOG(LogFormatter, Error, TEXT("    Pin '%s' IS in FinalExclusions. SKIPPED."), *Pin->Name);
                }
            }
		}
		return ArgsList.Num() > 0 ? FString::Printf(TEXT("(%s)"), *FString::Join(ArgsList, TEXT(", "))) : TEXT("()");
	}

	FString FormatTarget( 
		TSharedPtr<const FBlueprintNode> Node, 
		FMarkdownDataTracer& DataTracer, 
		const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
		TSet<FString>& VisitedDataPins,
		bool bSymbolicTraceForData,
		const FString& CurrentBlueprintContext
	)
	{
		TSharedPtr<const FBlueprintPin> TargetPin = Node->GetPin(TEXT("self"), TEXT("EGPD_Input"));
		if (!TargetPin) TargetPin = Node->GetPin(TEXT("Target"), TEXT("EGPD_Input"));
		if (!TargetPin) TargetPin = Node->GetPin(TEXT("WorldContextObject"), TEXT("EGPD_Input"));

		if (!TargetPin.IsValid()) return TEXT("");
		
		FString TargetValueStr = DataTracer.TracePinValue(TargetPin, AllNodes, bSymbolicTraceForData); 

		if (TargetValueStr == FMarkdownSpan::Variable(TEXT("self"))) { return TEXT(""); }
		else if (TargetValueStr.Contains(TEXT("Default__")) || TargetValueStr.Contains(TEXT("Library")) || TargetValueStr == FMarkdownSpan::LiteralObject(TEXT("None"))) { return TEXT(""); }
		else if (TargetValueStr.Contains(TEXT("<span")) || TargetValueStr.Contains(TEXT("("))) { return FString::Printf(TEXT(" on (%s)"), *TargetValueStr); }
		else { return FString::Printf(TEXT(" on %s"), *TargetValueStr); }
	}

	FString FormatGeneric(TSharedPtr<const FBlueprintNode> Node)
	{
		FString NodeName = Node->Name.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Node->Name);
        FString NodeTypeStr = Node->NodeType.IsEmpty() ? TEXT("UnknownType") : Node->NodeType;
		return FString::Printf(TEXT("**%s**%s"), *NodeTypeStr, *NodeName);
	}

} 


FString FMarkdownNodeFormatter::FormatNodeDescription(
	TSharedPtr<const FBlueprintNode> Node,
	const TOptional<FCapturedEventData>& CapturedData,
	FMarkdownDataTracer& DataTracer, 
	const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
	TSet<FString>& VisitedPins, 
	bool bGenerateLinkOnly, 
	bool bSymbolicTraceForData,
    const FString& CurrentBlueprintContext // New parameter
)
{
	UE_LOG(LogFormatter, Error, TEXT("FMarkdownNodeFormatter::FormatNodeDescription: Node='%s' (Type:%s), CtxRecv:'%s', LinkOnly:%d, Symbolic:%d"),
		*Node->Name, *Node->NodeType, *CurrentBlueprintContext, bGenerateLinkOnly, bSymbolicTraceForData);
	// The 'using namespace' is fine inside the function body if it refers to actual private helpers.
	// Or, if all private helpers are fully qualified, it's not needed.
	using namespace MarkdownNodeFormatters_Private;

	if (!Node.IsValid()) return FMarkdownSpan::Error(TEXT("[Null Node]"));

	// ... (rest of the existing function body for FormatNodeDescription) ...
    // Ensure any calls it makes to private formatters are correctly qualified if this 'using' directive
    // is removed or if there's ambiguity.
    // For example, instead of just FormatVariableSet(...), it would be MarkdownNodeFormatters_Private::FormatVariableSet(...)
    // if the using namespace line is removed.
    // Given the current structure, keeping `using namespace MarkdownNodeFormatters_Private;`
    // at the start of this function body is okay.

    // Example call to a private formatter (assuming it's defined in MarkdownNodeFormatters_Private):
    // if (NodeType == TEXT("VariableSet")) { return FormatVariableSet(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
    // This would correctly call MarkdownNodeFormatters_Private::FormatVariableSet due to the using directive.


	if (bGenerateLinkOnly)
	{
		FString GraphName = TEXT("UnknownGraph");
		FString NodeTypeKeyword = Node->NodeType;
		FString BaseLinkText; 

		FString GraphOwnerBPName;
		// Determine owning BP name for context (assuming Node is valid)
		if(Node->UEClass.Contains(TEXT("/"))) // Heuristic: UEClass often contains full path for nodes within BPs
		{
			GraphOwnerBPName = FPaths::GetBaseFilename(Node->UEClass);
			int32 DotPos;
			if(GraphOwnerBPName.FindChar(TEXT('.'), DotPos))
			{
				GraphOwnerBPName = GraphOwnerBPName.Left(DotPos);
			}
		}


		if (Node->NodeType == TEXT("CallFunction")) {
			GraphName = Node->RawProperties.FindRef(TEXT("FunctionName"));
			NodeTypeKeyword = TEXT("Call Function");
		} else if (Node->NodeType == TEXT("MacroInstance")) {
			// For MacroInstance, GraphName should be the full path for ExtractSimpleNameFromPath
			GraphName = Node->RawProperties.FindRef(TEXT("MacroGraphReference"));
			NodeTypeKeyword = TEXT("Macro");
		} else if (Node->NodeType == TEXT("Composite")) {
			GraphName = Node->RawProperties.FindRef(TEXT("BoundGraphName"));
			NodeTypeKeyword = TEXT("Collapsed Graph");
			
			FString LinkTextToShow = MarkdownTracerUtils::ExtractSimpleNameFromPath(GraphName, CurrentBlueprintContext);
			if (LinkTextToShow.IsEmpty()) LinkTextToShow = TEXT("CollapsedGraph");
			FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(GraphName); // Anchor uses full path
			FString LinkText = FString::Printf(TEXT("[%s](#%s)"), *LinkTextToShow, *AnchorName);
			return FString::Printf(TEXT("%s: %s"), *NodeTypeKeyword, *LinkText);
		} else {
			FString LinkTextToShow = Node->Name.IsEmpty() ? Node->NodeType : Node->Name;
            FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(LinkTextToShow); // Simple anchor for non-callables
            FString LinkText = FString::Printf(TEXT("[%s](#%s)"), *LinkTextToShow, *AnchorName);
            return FString::Printf(TEXT("%s: %s"), *NodeTypeKeyword, *LinkText);
        }
		
		FString LinkTextToShow = MarkdownTracerUtils::ExtractSimpleNameFromPath(GraphName, CurrentBlueprintContext);
		if (LinkTextToShow.IsEmpty()) LinkTextToShow = GraphName; 
		if (LinkTextToShow.IsEmpty()) LinkTextToShow = Node->Name.IsEmpty() ? Node->NodeType : Node->Name;


		FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(GraphName); 
		FString LinkText = FString::Printf(TEXT("[%s](#%s)"), *LinkTextToShow, *AnchorName);
		BaseLinkText = FString::Printf(TEXT("%s: %s"), *NodeTypeKeyword, *LinkText);

		if (Node->NodeType == TEXT("CallFunction") || Node->NodeType == TEXT("MacroInstance"))
		{
			TSet<FString> ArgsVisitedPins; 
			// Pass CurrentBlueprintContext to FormatArguments
			FString ArgsStr = MarkdownNodeFormatters_Private::FormatArguments(Node, DataTracer, AllNodes, ArgsVisitedPins, bSymbolicTraceForData, {}, CurrentBlueprintContext);
			return BaseLinkText + ArgsStr;
		}
		return BaseLinkText;
	}
	
	if (Node->IsPure() || Node->NodeType == TEXT("Knot") || Node->NodeType == TEXT("Comment"))
	{
		return FString(); 
	}

	const FString& NodeType = Node->NodeType;
	
	// Pass CurrentBlueprintContext to all private formatters
	if (NodeType == TEXT("VariableSet")) { return FormatVariableSet(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("CallParentFunction")) { return FormatCallParentFunction(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("CallFunction")) { return FormatCallFunction(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("IfThenElse")) { return FormatIfThenElse(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType.StartsWith(TEXT("Switch"))) { return FormatSwitch(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("MacroInstance")) { return FormatMacroInstance(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("ExecutionSequence")) { return FormatSequence(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("DynamicCast")) { return FormatDynamicCast(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("AddDelegate") || NodeType == TEXT("AssignDelegate") || NodeType == TEXT("RemoveDelegate")) { return FormatDelegateBinding(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("ClearDelegate")) { return FormatClearDelegate(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("CallDelegate")) { return FormatCallDelegate(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("Timeline")) { return FormatTimeline(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("SetFieldsInStruct")) { return FormatSetFieldsInStruct(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("FunctionResult")) { return FormatReturnNode(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("SpawnActorFromClass")) { return FormatSpawnActor(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("AddComponent")) { return FormatAddComponent(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("CreateWidget")) { return FormatCreateWidget(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("GenericCreateObject")) { return FormatGenericCreateObject(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("CallArrayFunction")) { return FormatCallArrayFunction(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("FormatText")) { return FormatFormatText(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("PlayMontage")) { return FormatPlayMontage(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("AIMoveTo") || NodeType == TEXT("Delay")) { return FormatLatentAction(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("Composite")) { return FormatComposite(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); }
	if (NodeType == TEXT("GetDataTableRow")) {return FormatGetDataTableRow_NodeDescription(Node, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext);	}
	
	// Special NodeType Multigate formatting
	if (Node->NodeType == TEXT("K2Node_MultiGate")) // <<< NEWLY ADDED BLOCK START
	{
		UE_LOG(LogFormatter, Log, TEXT("FormatNodeDescription: Matched K2Node_MultiGate for node %s. Context: '%s'."), *Node->Guid, *CurrentBlueprintContext);
		return FMarkdownSpan::Keyword(TEXT("MultiGate"));
	}
	if (NodeType == TEXT("Event") || NodeType == TEXT("CustomEvent") || NodeType == TEXT("FunctionEntry") || NodeType.Contains(TEXT("Input")) || NodeType.Contains(TEXT("BoundEvent")))
	{
		return FormatEvent(Node, CapturedData, DataTracer, AllNodes, VisitedPins, bSymbolicTraceForData, CurrentBlueprintContext); 
	}
	 if (Node->NodeType == TEXT("Tunnel"))
    {
        FString BaseTunnelFormat = MarkdownNodeFormatters_Private::FormatGeneric(Node); 

        // Check if it's an "Outputs" tunnel and if we are in a context where call-site args are available (implies inline expansion)
        bool bIsOutputTunnel = Node->Name.Contains(TEXT("Outputs")) || Node->Name.Contains(TEXT("Output"));
        const TMap<FName, FString>* CallSiteArgsForOutputs = DataTracer.GetCurrentCallsiteArguments();

        if (bIsOutputTunnel && CallSiteArgsForOutputs != nullptr)
        {
            UE_LOG(LogFormatter, Log, TEXT("FormatNodeDescription: Formatting OUTPUTS for Tunnel '%s' (GUID %s). CallSiteArgs are set."), *Node->Name, *Node->Guid.Left(8));
            TArray<FString> OutputValueStrings;
            // Iterate INPUT data pins of this "Outputs" tunnel
            for (const TSharedPtr<FBlueprintPin>& TunnelInputPin : Node->GetInputPins(TEXT(""), false, true)) 
            {
                if (TunnelInputPin.IsValid())
                {
                    UE_LOG(LogFormatter, Log, TEXT("  FormatNodeDescription (Tunnel Outputs): Tracing TunnelInputPin '%s' for value. Context: '%s'"), *TunnelInputPin->Name, *CurrentBlueprintContext);
                    // Resolve this pin's value. RVR will use CurrentCallsiteArgumentsPtr internally
                    // via HandleTunnel (for input tunnels) or HandleFunctionEntryPin.
                    FString ResolvedValue = DataTracer.ResolvePinValueRecursive(
                        TunnelInputPin,
                        AllNodes, // Node map for the internal graph (e.g., composite's internal nodes)
                        0,        // Depth for this specific value trace
                        VisitedPins, // Main visited set for the description formatting
                        nullptr,  // CallingNode for RVR
                        nullptr,  // OuterNodesMap for RVR
                        true,     // bSymbolicTrace = true for output display (consistent with definition blocks)
                        CurrentBlueprintContext // Context of the internal graph
                    );
                    OutputValueStrings.Add(FString::Printf(TEXT("`%s`=%s"), *TunnelInputPin->Name, *ResolvedValue));
                }
            }
            if (OutputValueStrings.Num() > 0)
            {
                // BaseTunnelFormat might be "Tunnel (Outputs)"
                // We want to insert ": Pin=Val, ..." after "(Outputs" and before ")"
                int32 ParenIndex = BaseTunnelFormat.Find(TEXT("("));
                if (ParenIndex != INDEX_NONE)
                {
                    int32 ClosingParenIndex = BaseTunnelFormat.Find(TEXT(")"), ESearchCase::IgnoreCase, ESearchDir::FromEnd, ParenIndex);
                    if (ClosingParenIndex != INDEX_NONE && ClosingParenIndex > ParenIndex)
                    {
                         FString NamePart = BaseTunnelFormat.Mid(ParenIndex + 1, ClosingParenIndex - (ParenIndex + 1) );
                         return FString::Printf(TEXT("%s(%s: %s)"), 
                            *BaseTunnelFormat.Left(ParenIndex), // e.g., "Tunnel"
                            *NamePart.TrimEnd(), // e.g., "Outputs"
                            *FString::Join(OutputValueStrings, TEXT(", "))
                         );
                    }
                }
                // Fallback if parentheses not found or mismatched in BaseTunnelFormat
                return BaseTunnelFormat + TEXT(": ") + FString::Join(OutputValueStrings, TEXT(", "));
            }
        }
        return BaseTunnelFormat; // Return simple format if not an "Outputs" tunnel with call-site args
    }
	return FormatGeneric(Node); // FormatGeneric doesn't need context as it doesn't trace or call other utils
}

