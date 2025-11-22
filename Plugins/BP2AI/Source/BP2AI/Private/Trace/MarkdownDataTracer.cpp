/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/MarkdownDataTracer.cpp

#include "Trace/MarkdownDataTracer.h" // Include self header first

#include "Logging/BP2AILog.h"
// --- Core Engine Includes ---
#include "CoreMinimal.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "Internationalization/Regex.h"
#include "Logging/LogMacros.h"
#include "Misc/DefaultValueHelper.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/BP2AILog.h"

// --- Plugin Specific Includes ---
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
// Include Handlers
#include "Handlers/NodeTraceHandlers_Basic.h"
#include "Handlers/NodeTraceHandlers_Operators.h"
#include "Handlers/NodeTraceHandlers_Structs.h"
#include "Handlers/NodeTraceHandlers_Functions.h"
#include "Handlers/NodeTraceHandlers_Arrays.h"
#include "Handlers/NodeTraceHandlers_Delegates.h"
#include "Handlers/NodeTraceHandlers_Events.h"
#include "Handlers/NodeTraceHandlers_FlowControl.h"
#include "Handlers/NodeTraceHandlers_ObjectMgmt.h"
#include "Handlers/NodeTraceHandlers_Timeline.h"
#include "Handlers/NodeTraceHandlers_Latent.h"
#include "Handlers/NodeTraceHandlers_Enums.h"
#include "Handlers/NodeTraceHandlers_FunctionResult.h"
#include "Handlers/NodeTraceHandlers_Maps.h"
#include "Handlers/NodeTraceHandlers_Text.h"
#include "Handlers/NodeTraceHandlers_Sets.h"  
#include "Handlers/NodeTraceHandlers_DataTables.h"
#include "Handlers/NodeTraceHandlers_Composite.h" 


// --- Include Utility Headers ---
#include "Utils/MarkdownFormattingUtils.h" // <<<<< Include Formatting Utils
#include "Utils/MarkdownTracerUtils.h"     // <<<<< Include Tracer Utils


// Define NAME_ constants manually as a workaround if not defined elsewhere
// These are commonly defined in Engine headers, but defining them here
// ensures they are available for comparison with FName.
#ifndef NAME_Bool
#define NAME_Bool FName(TEXT("bool"))
#endif
#ifndef NAME_Byte
#define NAME_Byte FName(TEXT("byte"))
#endif
#ifndef NAME_Int
#define NAME_Int FName(TEXT("int"))
#endif
#ifndef NAME_Float
#define NAME_Float FName(TEXT("float"))
#endif
#ifndef NAME_Real
#define NAME_Real FName(TEXT("real")) // Typically float/double, but defined in some contexts
#endif
#ifndef NAME_String
#define NAME_String FName(TEXT("string"))
#endif
#ifndef NAME_Text
#define NAME_Text FName(TEXT("text"))
#endif
#ifndef NAME_Name
#define NAME_Name FName(TEXT("name"))
#endif
#ifndef NAME_Object
#define NAME_Object FName(TEXT("object"))
#endif
#ifndef NAME_Class
#define NAME_Class FName(TEXT("class"))
#endif
#ifndef NAME_Interface
#define NAME_Interface FName(TEXT("interface"))
#endif
#ifndef NAME_Struct
#define NAME_Struct FName(TEXT("struct"))
#endif
#ifndef NAME_Delegate
#define NAME_Delegate FName(TEXT("delegate"))
#endif


// Constructor - Registering all handlers
FMarkdownDataTracer::FMarkdownDataTracer(const FBlueprintDataExtractor& InDataExtractor) : DataExtractorRef(InDataExtractor), MaxTraceDepth(15)
{
	// --- Map initializations ---
	// Map node function names (or base names) to their symbolic representation
	MathOperatorMap.Add(TEXT("Divide"), TEXT("/"));
	MathOperatorMap.Add(TEXT("Add"), TEXT("+"));
	MathOperatorMap.Add(TEXT("Subtract"), TEXT("-"));
	MathOperatorMap.Add(TEXT("Multiply"), TEXT("*"));
	MathOperatorMap.Add(TEXT("Less"), TEXT("<"));
	MathOperatorMap.Add(TEXT("Greater"), TEXT(">"));
	MathOperatorMap.Add(TEXT("LessEqual"), TEXT("<="));
	MathOperatorMap.Add(TEXT("GreaterEqual"), TEXT(">="));
	MathOperatorMap.Add(TEXT("EqualEqual"), TEXT("=="));
	MathOperatorMap.Add(TEXT("NotEqual"), TEXT("!="));
	// MathOperatorMap.Add(TEXT("BooleanAND"), TEXT("AND")); // Using text for boolean ops
	// MathOperatorMap.Add(TEXT("BooleanOR"), TEXT("OR"));   // Using text for boolean ops
	// MathOperatorMap.Add(TEXT("BooleanXOR"), TEXT("XOR")); // Using text for boolean ops
	// MathOperatorMap.Add(TEXT("BooleanNAND"), TEXT("NAND")); // Using text for boolean ops
	// MathOperatorMap.Add(TEXT("Max"), TEXT("MAX")); // Using text for Max/Min
	// MathOperatorMap.Add(TEXT("Min"), TEXT("MIN")); // Using text for Max/Min
	// MathOperatorMap.Add(TEXT("FMax"), TEXT("MAX")); // Using text for Max/Min
	// MathOperatorMap.Add(TEXT("FMin"), TEXT("MIN")); // Using text for Max/Min
	MathOperatorMap.Add(TEXT("Percent"), TEXT("%")); // Use base name for modulo
	MathOperatorMap.Add(TEXT("BooleanNot"), TEXT("!")); // Keep '!' symbol for BooleanNot
	MathOperatorMap.Add(TEXT("Concat"), TEXT("+")); // Added for FormatOperator, using '+' for string concat
	MathOperatorMap.Add(TEXT("Not"), TEXT("!"));
	MathOperatorMap.Add(TEXT("Percent_FloatFloat"), TEXT("%"));
	MathOperatorMap.Add(TEXT("Not_PreBool"), TEXT("!"));
	

	// === STRING COMPARISON OPERATORS ===
	// Case Sensitive
	MathOperatorMap.Add(TEXT("EqualEqual_StrStr"), TEXT("==="));
	MathOperatorMap.Add(TEXT("NotEqual_StrStr"), TEXT("!=="));

	// Case Insensitive (StriStri = String Insensitive)
	MathOperatorMap.Add(TEXT("EqualEqual_StriStri"), TEXT("==")); 
	MathOperatorMap.Add(TEXT("NotEqual_StriStri"), TEXT("!="));

	// === TEXT COMPARISON OPERATORS ===
	// Case Sensitive  
	MathOperatorMap.Add(TEXT("EqualEqual_TextText"), TEXT("==="));
	MathOperatorMap.Add(TEXT("NotEqual_TextText"), TEXT("!=="));

	// Case Insensitive
	MathOperatorMap.Add(TEXT("EqualEqual_IgnoreCase_TextText"), TEXT("=="));
	MathOperatorMap.Add(TEXT("NotEqual_IgnoreCase_TextText"), TEXT("!="));

	
	// Type Conversion Map
	// Maps conversion node function names to a simplified target type name
	TypeConversionMap.Add(TEXT("Conv_BoolToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_BoolToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_BoolToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_ByteToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_ByteToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_IntToByte"), TEXT("byte"));
	TypeConversionMap.Add(TEXT("Conv_IntToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_IntToDouble"), TEXT("double"));
	TypeConversionMap.Add(TEXT("Conv_IntToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_IntToInt64"), TEXT("int64"));
	TypeConversionMap.Add(TEXT("Conv_Int64ToByte"), TEXT("byte"));
	TypeConversionMap.Add(TEXT("Conv_Int64ToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_Int64ToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_FloatToBool"), TEXT("bool"));
	TypeConversionMap.Add(TEXT("Conv_FloatToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_FloatToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_FloatToDouble"), TEXT("double"));
	TypeConversionMap.Add(TEXT("Conv_DoubleToBool"), TEXT("bool"));
	TypeConversionMap.Add(TEXT("Conv_DoubleToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_DoubleToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_DoubleToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_StringToBool"), TEXT("bool"));
	TypeConversionMap.Add(TEXT("Conv_StringToInt"), TEXT("int"));
	TypeConversionMap.Add(TEXT("Conv_StringToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_StringToName"), TEXT("name"));
	TypeConversionMap.Add(TEXT("Conv_NameToBool"), TEXT("bool"));
	TypeConversionMap.Add(TEXT("Conv_NameToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_ObjectToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_VectorToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_IntToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_FloatToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_StringToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_NameToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_ByteToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_BoolToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_VectorToStringNormalized"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_RotatorToStringNormalized"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_ObjectToStringNormalized"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_ObjectToText"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("Conv_TextToString"), TEXT("string"));
	TypeConversionMap.Add(TEXT("Conv_TextToName"), TEXT("name"));
	TypeConversionMap.Add(TEXT("Conv_TextToFloat"), TEXT("float"));
	TypeConversionMap.Add(TEXT("Conv_TextToInt"), TEXT("int"));

	// Common function names that act as conversions (e.g., "ToString (Vector)")
	TypeConversionMap.Add(TEXT("ToString (Vector)"), TEXT("string"));
	TypeConversionMap.Add(TEXT("ToString (Rotator)"), TEXT("string"));
	TypeConversionMap.Add(TEXT("ToString (Object)"), TEXT("string"));
	TypeConversionMap.Add(TEXT("ToText (int)"), TEXT("Text"));
	TypeConversionMap.Add(TEXT("ToText (float)"), TEXT("Text"));

	// --- Register All Handlers ---
	NodeHandlers.Add(TEXT("VariableGet"), &FNodeTraceHandlers_Basic::HandleVariableGet);
	NodeHandlers.Add(TEXT("Literal"), &FNodeTraceHandlers_Basic::HandleLiteral);
	NodeHandlers.Add(TEXT("Self"), &FNodeTraceHandlers_Basic::HandleSelf);
	NodeHandlers.Add(TEXT("VariableSet"), &FNodeTraceHandlers_Basic::HandleVariableSet);
	NodeHandlers.Add(TEXT("PromotableOperator"), &FNodeTraceHandlers_Operators::HandleOperator);
	NodeHandlers.Add(TEXT("CommutativeAssociativeBinaryOperator"), &FNodeTraceHandlers_Operators::HandleOperator);
	NodeHandlers.Add(TEXT("BooleanNot"), &FNodeTraceHandlers_Operators::HandleUnaryOperator); // Assumes BooleanNot NodeType
	NodeHandlers.Add(TEXT("BreakStruct"), &FNodeTraceHandlers_Structs::HandleBreakStruct);
	NodeHandlers.Add(TEXT("MakeStruct"), &FNodeTraceHandlers_Structs::HandleMakeStruct);
	NodeHandlers.Add(TEXT("SetFieldsInStruct"), &FNodeTraceHandlers_Structs::HandleSetFieldsInStruct);
	NodeHandlers.Add(TEXT("CallFunction"), &FNodeTraceHandlers_Functions::HandleCallFunction);
	NodeHandlers.Add(TEXT("CallParentFunction"), &FNodeTraceHandlers_Functions::HandleCallFunction);
	NodeHandlers.Add(TEXT("MacroInstance"), &FNodeTraceHandlers_Functions::HandleCallMacro);
	NodeHandlers.Add(TEXT("MakeArray"), &FNodeTraceHandlers_Arrays::HandleMakeArray);
	NodeHandlers.Add(TEXT("GetArrayItem"), &FNodeTraceHandlers_Arrays::HandleGetArrayItem);
	NodeHandlers.Add(TEXT("CallArrayFunction"), &FNodeTraceHandlers_Arrays::HandleCallArrayFunction);
	NodeHandlers.Add(TEXT("CreateDelegate"), &FNodeTraceHandlers_Delegates::HandleCreateDelegate);
	NodeHandlers.Add(TEXT("Select"), &FNodeTraceHandlers_FlowControl::HandleSelect);
	NodeHandlers.Add(TEXT("SpawnActorFromClass"), &FNodeTraceHandlers_ObjectMgmt::HandleSpawnActor);
	NodeHandlers.Add(TEXT("AddComponent"), &FNodeTraceHandlers_ObjectMgmt::HandleAddComponent);
	NodeHandlers.Add(TEXT("CreateWidget"), &FNodeTraceHandlers_ObjectMgmt::HandleCreateWidget);
	NodeHandlers.Add(TEXT("DynamicCast"), &FNodeTraceHandlers_ObjectMgmt::HandleDynamicCast);
	NodeHandlers.Add(TEXT("GetClassDefaults"), &FNodeTraceHandlers_ObjectMgmt::HandleGetClassDefaults);
	NodeHandlers.Add(TEXT("GetSubsystem"), &FNodeTraceHandlers_ObjectMgmt::HandleGetSubsystem);
	NodeHandlers.Add(TEXT("GetEngineSubsystem"), &FNodeTraceHandlers_ObjectMgmt::HandleGetSubsystem);
	NodeHandlers.Add(TEXT("GetSubsystemFromPC"), &FNodeTraceHandlers_ObjectMgmt::HandleGetSubsystem);
	NodeHandlers.Add(TEXT("Timeline"), &FNodeTraceHandlers_Timeline::HandleTimeline);
	NodeHandlers.Add(TEXT("Delay"), &FNodeTraceHandlers_Latent::HandleLatentAction);
	NodeHandlers.Add(TEXT("MoveComponentTo"), &FNodeTraceHandlers_Latent::HandleLatentAction);
	NodeHandlers.Add(TEXT("AIMoveTo"), &FNodeTraceHandlers_Latent::HandleLatentAction);
	NodeHandlers.Add(TEXT("EnumEquality"), &FNodeTraceHandlers_Enums::HandleEnumComparison);
	NodeHandlers.Add(TEXT("EnumInequality"), &FNodeTraceHandlers_Enums::HandleEnumComparison);
	NodeHandlers.Add(TEXT("CastByteToEnum"), &FNodeTraceHandlers_Enums::HandleCastByteToEnum);
	NodeHandlers.Add(TEXT("EnumLiteral"), &FNodeTraceHandlers_Enums::HandleEnumLiteral);
    NodeHandlers.Add(TEXT("FunctionResult"), &FNodeTraceHandlers_FunctionResult::HandleFunctionResult); // Added FunctionResult
    NodeHandlers.Add(TEXT("MakeMap"), &FNodeTraceHandlers_Maps::HandleMakeMap);
	NodeHandlers.Add(TEXT("MakeSet"), &FNodeTraceHandlers_Sets::HandleMakeSet); 
    NodeHandlers.Add(TEXT("FormatText"), &FNodeTraceHandlers_Text::HandleFormatText); // Added FormatText

    // --- CORRECTED Event Handler Registration ---
    // Directly register common event types and the bound event types to the SAME handler.
	NodeHandlers.Add(TEXT("Tunnel"), &FNodeTraceHandlers_Basic::HandleTunnel);
    NodeHandlers.Add(TEXT("Event"), &FNodeTraceHandlers_Events::HandleEventOutputParam);
    NodeHandlers.Add(TEXT("CustomEvent"), &FNodeTraceHandlers_Events::HandleEventOutputParam);
    NodeHandlers.Add(TEXT("ComponentBoundEvent"), &FNodeTraceHandlers_Events::HandleEventOutputParam); // ADDED
    NodeHandlers.Add(TEXT("ActorBoundEvent"), &FNodeTraceHandlers_Events::HandleEventOutputParam);     // ADDED
    // Add other specific event node types if needed (e.g., InputAxisEvent, EnhancedInputAction)
    NodeHandlers.Add(TEXT("InputAxisEvent"), &FNodeTraceHandlers_Events::HandleEventOutputParam);
    NodeHandlers.Add(TEXT("EnhancedInputAction"), &FNodeTraceHandlers_Events::HandleEventOutputParam);
    NodeHandlers.Add(TEXT("InputAction"), &FNodeTraceHandlers_Events::HandleEventOutputParam);
    NodeHandlers.Add(TEXT("InputKey"), &FNodeTraceHandlers_Events::HandleEventOutputParam);

	NodeHandlers.Add(TEXT("GetDataTableRow"), &FNodeTraceHandlers_DataTables::HandleGetDataTableRow);
	NodeHandlers.Add(TEXT("Composite"), &FNodeTraceHandlers_Composite::HandleCompositeOutputPinValue);
	NodeHandlers.Add(TEXT("FunctionEntry"), &FNodeTraceHandlers_Functions::HandleFunctionEntryPin);

	// ... add others as identified ...
	


	// --- END CORRECTION ---


	UE_LOG(LogDataTracer, Log, TEXT("FMarkdownDataTracer initialized. Registered %d handlers."), NodeHandlers.Num());
}


// --- Public Methods ---

// Entry point for tracing a specific pin's value
FString FMarkdownDataTracer::TracePinValue(
	TSharedPtr<const FBlueprintPin> PinToResolve,
	const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
	bool bSymbolicTrace,
	const FString& CurrentBlueprintContext
)
{
	if (!PinToResolve.IsValid()) { 
		UE_LOG(LogDataTracer, Warning, TEXT("TracePinValue: Invalid Pin!")); 
		return FMarkdownSpan::Error(TEXT("[Missing Pin Object]")); 
	}

	
	UE_LOG(LogDataTracer, Log, TEXT("TracePinValue START: Pin=%s (%s), Node=%s, Symbolic=%d"), 
		   *PinToResolve->Name, *PinToResolve->Id, *PinToResolve->NodeGuid, bSymbolicTrace);
           
	// Basic validation for pin data
	if (PinToResolve->Id.IsEmpty() || PinToResolve->NodeGuid.IsEmpty()) { 
		return FMarkdownSpan::Error(TEXT("[Pin missing ID or Node GUID]")); 
	}
    
	TSet<FString> VisitedPins; // Set to track visited pins during recursion to detect cycles
    
	// Pass the bSymbolicTrace flag AND the CurrentBlueprintContext to the recursive helper
	return ResolvePinValueRecursive(
		PinToResolve, 
		AllNodes, 
		0, 
		VisitedPins, 
		nullptr, 
		nullptr, 
		bSymbolicTrace, 
		CurrentBlueprintContext  // Pass this parameter
	);
}


// Clears the cache of resolved pin values
void FMarkdownDataTracer::ClearCache()
{
	ResolvedPinCache.Empty();
}

// --- Core Recursive Logic ---
FString FMarkdownDataTracer::ResolvePinValueRecursive(
    TSharedPtr<const FBlueprintPin> PinToResolve,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(PinToResolve.IsValid());

    const TSharedPtr<FBlueprintNode>* OwningNodeForLogPtr = CurrentNodesMap.Find(PinToResolve->NodeGuid);
    FString OwningNodeNameForLog = TEXT("UnknownNodeInCurrentMap");
    FString OwningNodeTypeForLog = TEXT("UnknownTypeInCurrentMap");

	
    if (OwningNodeForLogPtr && OwningNodeForLogPtr->IsValid())
    {
        OwningNodeNameForLog = (*OwningNodeForLogPtr)->Name.IsEmpty() ? (*OwningNodeForLogPtr)->NodeType : (*OwningNodeForLogPtr)->Name;
        OwningNodeTypeForLog = (*OwningNodeForLogPtr)->NodeType;
    }
    else if (OuterNodesMap) 
    {
        OwningNodeForLogPtr = OuterNodesMap->Find(PinToResolve->NodeGuid);
        if (OwningNodeForLogPtr && OwningNodeForLogPtr->IsValid())
        {
            OwningNodeNameForLog = ((*OwningNodeForLogPtr)->Name.IsEmpty() ? (*OwningNodeForLogPtr)->NodeType : (*OwningNodeForLogPtr)->Name) + TEXT(" (From OuterMap)");
            OwningNodeTypeForLog = (*OwningNodeForLogPtr)->NodeType + TEXT(" (From OuterMap)");
        }
    }
    
    const FString CacheKey = FString::Printf(TEXT("%s_%s_Ctx%s_Sym%d"), 
        *PinToResolve->NodeGuid, 
        *PinToResolve->Id, 
        *CurrentBlueprintContext,
        bSymbolicTrace);

    UE_LOG(LogDataTracer, Error, TEXT("RVR Enter: Pin='%s' (ID:%s) on Node='%s' (GUID:%s, Type:%s), Depth:%d, Symbolic:%d. CtxRecv:'%s'. CacheKey:'%s'. CallingNode: %s (%s), OuterMap: %p, LinksIn:%d, DefVal:'%s', DefObj:'%s'"),
        *PinToResolve->Name,
        *PinToResolve->Id.Left(8),
        *OwningNodeNameForLog,
        *PinToResolve->NodeGuid.Left(8),
        *OwningNodeTypeForLog,
        Depth,
        bSymbolicTrace,
        *CurrentBlueprintContext,
        *CacheKey,
        CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL_Node"),
        CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID"),
        OuterNodesMap,
        PinToResolve->SourcePinFor.Num(),
        *PinToResolve->DefaultValue,
        *PinToResolve->DefaultObject
    );

	// 🔧 NEW DEBUG: Show ALL possible source pins
	UE_LOG(LogDataTracer, Error, TEXT("🔧 PIN RECURSION DEBUG: Pin='%s' has %d source connections:"), *PinToResolve->Name, PinToResolve->SourcePinFor.Num());
	for (int32 i = 0; i < PinToResolve->SourcePinFor.Num(); ++i)
	{
		if (PinToResolve->SourcePinFor[i].IsValid())
		{
			// Get the source node info
			FString SourceNodeGuid = PinToResolve->SourcePinFor[i]->NodeGuid;
			TSharedPtr<FBlueprintNode> SourceNode = CurrentNodesMap.FindRef(SourceNodeGuid);
			FString SourceNodeName = SourceNode.IsValid() ? SourceNode->Name : TEXT("UNKNOWN_NODE");
			FString SourceNodeType = SourceNode.IsValid() ? SourceNode->NodeType : TEXT("UNKNOWN_TYPE");
        
			UE_LOG(LogDataTracer, Error, TEXT("🔧   Source[%d]: NodeGUID='%s', NodeName='%s', NodeType='%s', PinName='%s' (ID:%s)"), 
				i, *SourceNodeGuid.Left(8), *SourceNodeName, *SourceNodeType, 
				*PinToResolve->SourcePinFor[i]->Name, *PinToResolve->SourcePinFor[i]->Id.Left(8));
		}
		else
		{
			UE_LOG(LogDataTracer, Error, TEXT("🔧   Source[%d]: INVALID"), i);
		}
	}
	

	
    if (const FString* CachedValue = ResolvedPinCache.Find(CacheKey)) {
        UE_LOG(LogDataTracer, Log, TEXT("RVR Cache Hit: Pin=%s (%s), CacheKey='%s' -> Result='%s'"), *PinToResolve->Name, *PinToResolve->Id.Left(8), *CacheKey, **CachedValue);
        return *CachedValue;
    }
    if (Depth > MaxTraceDepth) { 
        UE_LOG(LogDataTracer, Warning, TEXT("RVR Max Depth: Pin=%s (%s), CacheKey='%s'"), *PinToResolve->Name, *PinToResolve->Id.Left(8), *CacheKey);
        return FMarkdownSpan::Error(TEXT("[Trace Depth Limit]")); 
    }

    if (VisitedPins.Contains(CacheKey)) {
        UE_LOG(LogDataTracer, Warning, TEXT("RVR Cycle Detected: Pin=%s (%s), CacheKey='%s'"), *PinToResolve->Name, *PinToResolve->Id.Left(8), *CacheKey);
        const TSharedPtr<FBlueprintNode>* OwningNodePtrCycle = CurrentNodesMap.Find(PinToResolve->NodeGuid); // Re-fetch with CurrentNodesMap
        if (OwningNodePtrCycle && OwningNodePtrCycle->IsValid() && (*OwningNodePtrCycle)->NodeType == TEXT("VariableGet")){
             const FString* VarName = (*OwningNodePtrCycle)->RawProperties.Find(TEXT("VariableName"));
             return FMarkdownSpan::Variable(FString::Printf(TEXT("%s"), VarName ? **VarName : TEXT("Var?")));
        }
        return FMarkdownSpan::Error(FString::Printf(TEXT("[Cycle->%s]"), *PinToResolve->Name));
    }
    VisitedPins.Add(CacheKey);

    FString Result = FMarkdownSpan::Error(TEXT("[Failed Trace]"));

    try {
        TSharedPtr<const FBlueprintPin> SourceDataPin = nullptr;
        // CRITICAL DEBUG POINT: Log what SourcePinFor contains for this PinToResolve
        if (PinToResolve->IsInput()) {
            if (PinToResolve->SourcePinFor.Num() > 0) {
                for (int32 Idx = 0; Idx < PinToResolve->SourcePinFor.Num(); ++Idx) {
                    if (PinToResolve->SourcePinFor[Idx].IsValid()) {
                        UE_LOG(LogDataTracer, Error, TEXT("  RVR: Input Pin '%s' (ID:%s) SourcePinFor[%d]: NodeGUID=%s, PinID=%s, PinName='%s'"),
                            *PinToResolve->Name, *PinToResolve->Id.Left(8), Idx,
                            *PinToResolve->SourcePinFor[Idx]->NodeGuid.Left(8),
                            *PinToResolve->SourcePinFor[Idx]->Id.Left(8),
                            *PinToResolve->SourcePinFor[Idx]->Name);
                    } else {
                        UE_LOG(LogDataTracer, Error, TEXT("  RVR: Input Pin '%s' (ID:%s) SourcePinFor[%d] is INVALID"), *PinToResolve->Name, *PinToResolve->Id.Left(8), Idx);
                    }
                }
                if (PinToResolve->SourcePinFor[0].IsValid()) { // Original logic to pick first valid
                    SourceDataPin = PinToResolve->SourcePinFor[0];
                }
            } else {
                 UE_LOG(LogDataTracer, Error, TEXT("  RVR: Input Pin '%s' (ID:%s) has SourcePinFor.Num() == 0."), *PinToResolve->Name, *PinToResolve->Id.Left(8));
            }
        }
        else if (PinToResolve->IsOutput()) { // PinToResolve is an output pin we are trying to get the value of
           SourceDataPin = PinToResolve; // The output pin itself is the source of its value (from its node)
             UE_LOG(LogDataTracer, Error, TEXT("  RVR: Pin '%s' (ID:%s) is Output. Setting SourceDataPin to itself."), *PinToResolve->Name, *PinToResolve->Id.Left(8));
        }

        if (SourceDataPin.IsValid() && !SourceDataPin->NodeGuid.IsEmpty()) {
           const TSharedPtr<FBlueprintNode>* SourceNodePtr = CurrentNodesMap.Find(SourceDataPin->NodeGuid);
           if (!SourceNodePtr && OuterNodesMap) { // Try finding in outer map if not in current
                SourceNodePtr = OuterNodesMap->Find(SourceDataPin->NodeGuid);
                if(SourceNodePtr) { UE_LOG(LogDataTracer, Warning, TEXT("  RVR: SourceNode for SourceDataPin '%s' found in OuterNodesMap."), *SourceDataPin->Name); }
           }

           if (SourceNodePtr && SourceNodePtr->IsValid()) {
              UE_LOG(LogDataTracer, Error, TEXT("  RVR: Pin '%s' (ID:%s) has SourceDataPin '%s' (ID:%s) on Node '%s' (GUID:%s, Type:%s). Calling TraceSourceNode. CtxToPass:'%s'"),
                  *PinToResolve->Name, *PinToResolve->Id.Left(8),
                  *SourceDataPin->Name, *SourceDataPin->Id.Left(8),
                  *(*SourceNodePtr)->Name, *(*SourceNodePtr)->Guid.Left(8), *(*SourceNodePtr)->NodeType,
                  *CurrentBlueprintContext);
              Result = TraceSourceNode(*SourceNodePtr, SourceDataPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
           } else {
              UE_LOG(LogDataTracer, Error, TEXT("  RVR: Pin '%s' (ID:%s) - SourceNode for SourceDataPin '%s' (NodeGUID:%s) NOT FOUND in CurrentNodesMap or OuterNodesMap. Defaulting value."),
                *PinToResolve->Name, *PinToResolve->Id.Left(8), *SourceDataPin->Name, *SourceDataPin->NodeGuid.Left(8));
              Result = MarkdownFormattingUtils::FormatDefaultValue(PinToResolve, this); // Fallback to default if source node missing
           }
        }
        else { 
           UE_LOG(LogDataTracer, Error, TEXT("  RVR: Pin '%s' (ID:%s) has NO valid SourceDataPin (SourcePinFor.Num(): %d, IsOutput: %d). Using FormatDefaultValue. DefVal:'%s', DefObj:'%s', CtxRecv:'%s'"),
               *PinToResolve->Name, *PinToResolve->Id.Left(8), PinToResolve->SourcePinFor.Num(), PinToResolve->IsOutput(), *PinToResolve->DefaultValue, *PinToResolve->DefaultObject, *CurrentBlueprintContext);
           Result = MarkdownFormattingUtils::FormatDefaultValue(PinToResolve, this);
        }
    } 
    catch (...) {
        UE_LOG(LogDataTracer, Error, TEXT("RVR Exception occurred for Pin: %s (%s), CacheKey='%s'"), *PinToResolve->Name, *PinToResolve->Id.Left(8), *CacheKey);
        Result = FMarkdownSpan::Error(TEXT("[Trace Exception]"));
    }

    VisitedPins.Remove(CacheKey);
    ResolvedPinCache.Add(CacheKey, Result);
    UE_LOG(LogDataTracer, Log, TEXT("RVR Exit : Pin=%s (ID:%s), Depth=%d, ContextIn='%s', CacheKey='%s' -> Result='%s'. Caching."), *PinToResolve->Name, *PinToResolve->Id.Left(8), Depth, *CurrentBlueprintContext, *CacheKey, *Result);
    return Result;
}


// --- Helper to trace the Target pin of a function call ---
// In Private\Trace\MarkdownDataTracer.cpp
FString FMarkdownDataTracer::TraceTargetPin(
    TSharedPtr<const FBlueprintPin> TargetPin,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FString& CurrentBlueprintContext)
{
    if (!TargetPin.IsValid())
    {
        FString ReturnValue = FMarkdownSpan::Variable(TEXT("`self`"));
        UE_LOG(LogDataTracer, Warning, TEXT("  TraceTargetPin: TargetPin is null. Context='%s'. Returning: '%s'"), *CurrentBlueprintContext, *ReturnValue);
        return ReturnValue;
    }

    UE_LOG(LogDataTracer, Log, TEXT("  TraceTargetPin: Processing PinName='%s', NodeGuid='%s', Direction='%s', Category='%s', DefaultObject='%s', Context='%s'"),
        *TargetPin->Name, *TargetPin->NodeGuid, *TargetPin->Direction, *TargetPin->Category, *TargetPin->DefaultObject, *CurrentBlueprintContext);

    if (TargetPin->SourcePinFor.Num() == 0) // Unlinked pin
    {
        if (!TargetPin->DefaultObject.IsEmpty() && 
            !TargetPin->DefaultObject.Equals(TEXT("None"), ESearchCase::IgnoreCase) && 
            !TargetPin->DefaultObject.Equals(TEXT("NULL"), ESearchCase::IgnoreCase))
        {
            const FString& DefaultObjPath = TargetPin->DefaultObject;
            FString RawSimplifiedName = MarkdownTracerUtils::ExtractSimpleNameFromPath(DefaultObjPath, CurrentBlueprintContext);
            
            UE_LOG(LogDataTracer, Warning, TEXT("    TraceTargetPin (Unlinked/DefaultObject): DefaultObjPath='%s', RawSimplifiedName='%s'"), *DefaultObjPath, *RawSimplifiedName);

            if (!RawSimplifiedName.IsEmpty() && MarkdownTracerUtils::GetKnownStaticBlueprintLibraries().Contains(RawSimplifiedName))
            {
                UE_LOG(LogDataTracer, Warning, TEXT("    TraceTargetPin: Matched KnownStaticBlueprintLibrary: '%s'. Returning TEXT(\"\")."), *RawSimplifiedName);
                return TEXT(""); 
            }
            
            FString FormattedReturn = FMarkdownSpan::LiteralObject(FString::Printf(TEXT("%s"), *RawSimplifiedName));
            UE_LOG(LogDataTracer, Warning, TEXT("    TraceTargetPin: NOT a known static lib to omit OR RawSimplifiedName was empty. DefaultObject path. Returning formatted: '%s'"), *FormattedReturn);
            return FormattedReturn;
        }

        if (TargetPin->Name == TEXT("self") || TargetPin->Name == TEXT("Target") || TargetPin->Name == TEXT("WorldContextObject"))
        {
            FString ReturnValue = FMarkdownSpan::Variable(TEXT("`self`"));
            UE_LOG(LogDataTracer, Log, TEXT("  TraceTargetPin: Unlinked, self-like pin ('%s'). Returning: '%s'"), *TargetPin->Name, *ReturnValue);
            return ReturnValue;
        }
        
        FString DefaultValFormatted = MarkdownFormattingUtils::FormatDefaultValue(TargetPin, this);
        UE_LOG(LogDataTracer, Warning, TEXT("  TraceTargetPin: Unlinked, no recognized DefaultObject or self-like name ('%s'). Returning FormatDefaultValue: '%s'"), *TargetPin->Name, *DefaultValFormatted);
        return DefaultValFormatted;
    }

    // Linked pin
    UE_LOG(LogDataTracer, Log, TEXT("  TraceTargetPin: Linked pin '%s'. Resolving recursively with context '%s'."), *TargetPin->Name, *CurrentBlueprintContext);
    // Pass CurrentBlueprintContext to the recursive call.
    // bSymbolicTrace is false as we want the actual representation of the target.
    FString RecursiveResult = ResolvePinValueRecursive(TargetPin, AllNodes, Depth, VisitedPins, nullptr, nullptr, false, CurrentBlueprintContext);
    UE_LOG(LogDataTracer, Log, TEXT("  TraceTargetPin: Linked pin '%s' resolved to: '%s'"), *TargetPin->Name, *RecursiveResult);
    return RecursiveResult;
}
// --- Core Dispatcher (Private) ---

// Dispatches the tracing logic based on the source node type
// PROVIDE FULL FUNCTION
FString FMarkdownDataTracer::TraceSourceNode(
    TSharedPtr<const FBlueprintNode> SourceNode,
    TSharedPtr<const FBlueprintPin> SourcePin, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext // New parameter received
)
{
	UE_LOG(LogDataTracer, Error, TEXT("    TraceSourceNode ENTER: NodeType='%s', PinName='%s', CurrentBlueprintContext RECEIVED: '%s'"), 
	 SourceNode.IsValid() ? *SourceNode->NodeType : TEXT("INVALID_NODE"), 
	 SourcePin.IsValid() ? *SourcePin->Name : TEXT("INVALID_PIN"), 
	 *CurrentBlueprintContext);
    check(SourceNode.IsValid());
    check(SourcePin.IsValid());

    const FString& NodeTypeToLookup = SourceNode->NodeType; 

    UE_LOG(LogDataTracer, Verbose, TEXT("TraceSourceNode Dispatch: Node Type = %s, Pin = %s (Depth %d), Symbolic=%d, Context='%s'"), 
        *NodeTypeToLookup, 
        *SourcePin->Name, 
        Depth, 
        bSymbolicTrace,
        *CurrentBlueprintContext); // Log context

    if (NodeTypeToLookup == TEXT("Knot") || NodeTypeToLookup == TEXT("NiagaraReroute"))
    {
        UE_LOG(LogDataTracer, Verbose, TEXT("  TraceSourceNode: Detected Reroute Node '%s'. Finding input pin to trace..."), *SourceNode->Name);
        TSharedPtr<const FBlueprintPin> RerouteInputPin;
        for (const auto& Pair : SourceNode->Pins) {
            if (Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution()) {
                RerouteInputPin = Pair.Value; break; 
            }
        }
        if(RerouteInputPin.IsValid())
        {
            UE_LOG(LogDataTracer, Verbose, TEXT("  TraceSourceNode: Found reroute input '%s'. Tracing recursively with context '%s'."), *RerouteInputPin->Name, *CurrentBlueprintContext);
            // Pass CurrentBlueprintContext to ResolvePinValueRecursive
            return ResolvePinValueRecursive(RerouteInputPin, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
        }
        else
        {
            UE_LOG(LogDataTracer, Warning, TEXT("  TraceSourceNode: Reroute node %s has no input data pin!"), *SourceNode->Guid);
            return FMarkdownSpan::Error(TEXT("[Reroute Input Missing]"));
        }
    }

	UE_LOG(LogDataTracer, Log, TEXT("    TraceSourceNode Dispatch: Trying Node='%s' (Type:'%s', GUID:%s), Pin='%s'. CtxRecv:'%s', Symbolic:%d"),
		*SourceNode->Name,
		*NodeTypeToLookup,
		*SourceNode->Guid.Left(8),
		*SourcePin->Name,
		*CurrentBlueprintContext,
		bSymbolicTrace);

	if (NodeHandlers.Contains(NodeTypeToLookup))
	{
		FNodeTraceHandlerFunc* Handler = NodeHandlers.Find(NodeTypeToLookup);
		UE_LOG(LogDataTracer, Log, TEXT("      TraceSourceNode: Found handler for '%s'. CtxToPass:'%s'. CallingNode: %s (%s)"),
			*NodeTypeToLookup,
			*CurrentBlueprintContext,
			CallingNode.IsValid() ? *CallingNode->Name : TEXT("NULL"),
			CallingNode.IsValid() ? *CallingNode->Guid.Left(8) : TEXT("NULL_GUID")
			);
		return (*Handler)(SourceNode, SourcePin, this, CurrentNodesMap, Depth, VisitedPins, DataExtractorRef, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
	}

    UE_LOG(LogDataTracer, Warning, TEXT("    TraceSourceNode: No handler registered for Node Type = '%s' (Node Name: '%s'). Using fallback format."), *NodeTypeToLookup, *SourceNode->Name);
    FString PinNameStr = SourcePin->Name;
    return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(%s.`%s`)"),
        *NodeTypeToLookup, 
        *PinNameStr
    );
}

      
void FMarkdownDataTracer::StartTraceSession(
	TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>* OutGraphsToDefineSeparatelyPtr,
	TSet<FString>* ProcessedSeparateGraphPathsPtr,
	bool bInShowTrivialDefaultParams,
	const FGenerationSettings* InSettings // Added parameter
)
{
	CurrentGraphsToDefineSeparatelyPtr = OutGraphsToDefineSeparatelyPtr;
	CurrentProcessedSeparateGraphPathsPtr = ProcessedSeparateGraphPathsPtr;
	this->bCurrentShowTrivialDefaultParams = bInShowTrivialDefaultParams;
	this->CurrentSettings = InSettings; // Store the settings

	UE_LOG(LogDataTracer, Log, TEXT("DataTracer::StartTraceSession: Context pointers set. Queue: %p, ProcessedSet: %p, ShowTrivialDefaults: %s, Settings Ptr: %p"),
		   CurrentGraphsToDefineSeparatelyPtr,
		   CurrentProcessedSeparateGraphPathsPtr,
		   bCurrentShowTrivialDefaultParams ? TEXT("true") : TEXT("false"),
		   CurrentSettings);
}

void FMarkdownDataTracer::EndTraceSession()
{
	CurrentGraphsToDefineSeparatelyPtr = nullptr;
	CurrentProcessedSeparateGraphPathsPtr = nullptr;
	CurrentSettings = nullptr; 
	UE_LOG(LogDataTracer, Log, TEXT("DataTracer::EndTraceSession: Queue, ProcessedSet, and Settings context pointers cleared."));
}


const FGenerationSettings& FMarkdownDataTracer::GetSettings() const
{
	// Provide a default FGenerationSettings if CurrentSettings is null
	// This ensures the function always returns a valid reference,
	// though in practice, CurrentSettings should be set by StartTraceSession.
	static const FGenerationSettings DefaultSettings;
	if (CurrentSettings)
	{
		return *CurrentSettings;
	}
	UE_LOG(LogDataTracer, Warning, TEXT("FMarkdownDataTracer::GetSettings() called when CurrentSettings is null. Returning default settings."));
	return DefaultSettings;
}


// --- Formatting Helper Implementations REMOVED ---
// All implementations (FormatOperator, FormatUnaryOperator, FormatConversion, FormatPureMacroCall,
// FormatDefaultValue, FormatLiteralValue, ParseStructDefaultValue, FormatArgumentsForTrace)
// have been moved to MarkdownFormattingUtils.cpp
// --- END REMOVED ---
