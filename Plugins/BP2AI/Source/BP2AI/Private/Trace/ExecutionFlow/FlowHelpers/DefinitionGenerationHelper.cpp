/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/DefinitionGenerationHelper.cpp

#include "DefinitionGenerationHelper.h"

#include "Trace/FMarkdownPathTracer.h"
#include "Trace/MarkdownDataTracer.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/Generation/GenerationShared.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownTracerUtils.h"

#include "UObject/UObjectIterator.h"
#include "Engine/Blueprint.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "Kismet2/BlueprintEditorUtils.h"

FDefinitionGenerationHelper::FDefinitionGenerationHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FDefinitionGenerationHelper: Initialized"));
}

FDefinitionGenerationHelper::~FDefinitionGenerationHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FDefinitionGenerationHelper: Destroyed"));
}

FGraphDefinitionEntry FDefinitionGenerationHelper::CreateGraphDefinition(
    FMarkdownPathTracer& InPathTracer,
    FMarkdownDataTracer& InDataTracer,
    FBlueprintDataExtractor& InDataExtractor,
    const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
    const FString& CategoryKey,
    TSet<FString>& InOutProcessedSeparateGraphPaths,
    const FGenerationSettings& InSettings,
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
    TSet<FString>& InOutProcessedSeparateGraphPathsForDiscovery,
    const FString& RootBlueprintNameForTrace)
{
    const FString& GraphNameHint = GraphInfo.Get<0>();
    const FString& GraphPath = GraphInfo.Get<1>();
    FMarkdownPathTracer::EUserGraphType GraphType = GraphInfo.Get<2>();

    FGraphDefinitionEntry Definition;

    // ✅ FIX CASE 2: Use GraphNameHint for DefKey instead of GraphPath
    // This allows multiple custom events from the same EventGraph to each have their definition
    FString DefKey = MarkdownTracerUtils::GetCanonicalDefKey(GraphPath, GraphNameHint, GraphType);  // <<<< USING SHARED UTILITY
    
    if (InOutProcessedSeparateGraphPaths.Contains(DefKey))
    {
        UE_LOG(LogPathTracer, Log, TEXT("CreateGraphDefinition: Graph '%s' (DefKey: %s) already processed. Skipping."), 
            *GraphNameHint, *DefKey);
        return Definition; // Return empty definition
    }

    Definition.GraphName = GraphNameHint;
    Definition.Category = CategoryKey;
    Definition.AnchorId = FMarkdownPathTracer::SanitizeAnchorName(GraphNameHint);
    Definition.bIsPure = (CategoryKey.StartsWith(TEXT("Pure")));

    UE_LOG(LogPathTracer, Log, TEXT("CreateGraphDefinition: Creating definition for '%s' (DefKey: %s, Category: %s)"), 
        *Definition.GraphName, *DefKey, *Definition.Category);

    // Extract node data for this specific graph
    TMap<FString, TSharedPtr<FBlueprintNode>> GraphNodes;
    bool bExtracted = InDataExtractor.ExtractNodesFromGraph(GraphPath, GraphNodes);
    
    if (!bExtracted || GraphNodes.IsEmpty())
    {
        UE_LOG(LogPathTracer, Warning, TEXT("  CreateGraphDefinition: Could not extract nodes for graph '%s'. Adding error message."), *Definition.GraphName);
        Definition.ExecutionFlow.Add(TEXT("Error: Could not extract nodes for this graph."));
        InOutProcessedSeparateGraphPaths.Add(DefKey);
        return Definition;
    }

    // Determine context for symbolic tracing
    FString AssetContextForGraph = DetermineAssetContext(GraphNameHint, RootBlueprintNameForTrace);
    FString SimpleGraphName = ExtractSimpleNameFromHint(GraphNameHint);

    TSharedPtr<const FBlueprintNode> EntryNode = FindEntryNodeForInputs(GraphNodes, GraphType, SimpleGraphName);
    TSharedPtr<const FBlueprintNode> ExitNode = FindExitNodeForOutputs(GraphNodes, GraphType, SimpleGraphName);

    if (GraphType == FMarkdownPathTracer::EUserGraphType::Interface) {
        UE_LOG(LogPathTracer, Warning, TEXT("Interface signature extraction needed for: %s"), *GraphNameHint);
        ExtractInterfaceSignature(GraphNameHint, Definition.InputSpecs, Definition.OutputSpecs);
    } else {
        CollectInputSpecs(EntryNode, Definition.InputSpecs);
    }
    if (!Definition.bIsPure)
    {
        // ✅ FIX CASE 1: Ensure DataTracer points to correct collection for this definition
        UE_LOG(LogPathTracer, Error, TEXT("  CreateGraphDefinition: Starting DataTracer session for '%s'. OutGraphsToDefineSeparately addr: %p"), 
            *GraphNameHint, &OutGraphsToDefineSeparately);
            
        // CRITICAL: Point DataTracer to the correct collection for this definition phase
        InDataTracer.StartTraceSession(&OutGraphsToDefineSeparately, &InOutProcessedSeparateGraphPathsForDiscovery, InSettings.bShowTrivialDefaultParams, &InSettings);        
        // Call CollectExecutionFlow which will internally call TraceExecutionPath
        // Any pure macros discovered during data resolution will now go to the correct collection
        CollectExecutionFlow(
            InPathTracer, 
            InDataTracer, 
            EntryNode, 
            GraphNodes, 
            AssetContextForGraph, 
            InSettings, 
            Definition.ExecutionFlow,
            OutGraphsToDefineSeparately,           // This is the correct collection for next iteration
            InOutProcessedSeparateGraphPathsForDiscovery
        );
        
        InDataTracer.EndTraceSession();
        
        UE_LOG(LogPathTracer, Error, TEXT("  CreateGraphDefinition: Completed execution flow for '%s'. OutGraphsToDefineSeparately now has %d items"), 
            *GraphNameHint, OutGraphsToDefineSeparately.Num());
    }
    
    CollectOutputSpecs(ExitNode, GraphNodes, GraphType, GraphNameHint, AssetContextForGraph, InDataTracer, Definition.OutputSpecs);

    InOutProcessedSeparateGraphPaths.Add(DefKey);
    UE_LOG(LogPathTracer, Log, TEXT("  CreateGraphDefinition: Successfully created definition for '%s'. Marked DefKey '%s' as processed."), 
        *Definition.GraphName, *DefKey);
    
    return Definition;
}

void FDefinitionGenerationHelper::CollectInputSpecs(TSharedPtr<const FBlueprintNode> EntryNode, TArray<FString>& OutInputSpecs)
{
    if (EntryNode.IsValid())
    {
        // For FunctionEntry, CustomEvent, or Macro Input Tunnel, output pins are the graph's inputs
        for (const auto& PinPair : EntryNode->Pins) 
        {
            const TSharedPtr<FBlueprintPin>& Pin = PinPair.Value;
            if (Pin.IsValid() && Pin->Direction == TEXT("EGPD_Output") && !Pin->IsExecution())
            {
                OutInputSpecs.Add(FString::Printf(TEXT("`%s` (%s)"), *Pin->Name, *Pin->GetTypeSignature()));
            }
        }
    }
    else
    {
        // EntryNode is null - this might be an interface that needs special handling
        UE_LOG(LogPathTracer, Warning, TEXT("CollectInputSpecs: EntryNode is null - interface signature extraction may be needed"));
    }
    OutInputSpecs.Sort();
}

void FDefinitionGenerationHelper::CollectExecutionFlow(
    FMarkdownPathTracer& InPathTracer,
    FMarkdownDataTracer& InDataTracer,
    TSharedPtr<const FBlueprintNode> EntryNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
    const FString& AssetContext,
    const FGenerationSettings& InSettings,
    TArray<FString>& OutExecutionFlow,
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
    TSet<FString>& InOutProcessedSeparateGraphPaths)
{
    UE_LOG(LogPathTracer, Verbose, TEXT("CollectExecutionFlow: Starting execution flow collection for context '%s'"), *AssetContext);
    
    if (!EntryNode.IsValid())
    {
        UE_LOG(LogPathTracer, Warning, TEXT("CollectExecutionFlow: No valid entry node provided"));
        OutExecutionFlow.Add(TEXT("No valid entry node found for execution flow"));
        return;
    }

    // Find the output execution pin from the entry node
    TSharedPtr<FBlueprintPin> StartExecPin = nullptr;
    for (const auto& PinPair : EntryNode->Pins)
    {
        if (PinPair.Value.IsValid() && 
            PinPair.Value->IsExecution() && 
            PinPair.Value->Direction == TEXT("EGPD_Output"))
        {
            StartExecPin = PinPair.Value;
            break;
        }
    }

    if (!StartExecPin.IsValid())
    {
        UE_LOG(LogPathTracer, Warning, TEXT("CollectExecutionFlow: No output execution pin found on entry node '%s'"), *EntryNode->Name);
        OutExecutionFlow.Add(TEXT("No output execution pin found on entry node"));
        return;
    }

    // Create local processed nodes set for this graph's execution trace
    // This is separate from the global processed set to allow proper tracing within the graph
    TSet<FString> ProcessedNodesInThisGraph;
    
    UE_LOG(LogPathTracer, Log, TEXT("CollectExecutionFlow: Tracing execution from entry node '%s' in context '%s'"), 
           *EntryNode->Name, *AssetContext);

    // Call TraceExecutionPath with the same parameters as the legacy implementation
    TArray<FString> ExecutionLines = InPathTracer.TraceExecutionPath(
        EntryNode,                              // StartNode - the entry node for this graph
        TOptional<FCapturedEventData>(),        // CapturedData - empty for graph definitions
        GraphNodes,                             // AllNodes - nodes within this specific graph
        ProcessedNodesInThisGraph,              // InOutProcessedGlobally - local to this graph
        true,                                   // bInShouldTraceSymbolicallyForData - use symbolic tracing for definitions
        InSettings.bDefineUserGraphsSeparately, // bInDefineUserGraphsSeparately - pass through setting
        false,                                  // bInExpandCompositesInline - don't expand for sub-traces
        OutGraphsToDefineSeparately,            // OutGraphsToDefineSeparately - collect newly discovered graphs
        InOutProcessedSeparateGraphPaths,       // InOutProcessedSeparateGraphPaths - track processed graph paths
        AssetContext                            // CurrentBlueprintContext - context for this graph
    );

    // Append the traced execution lines to the output
    OutExecutionFlow.Append(ExecutionLines);
    
    UE_LOG(LogPathTracer, Verbose, TEXT("CollectExecutionFlow: Completed execution flow collection. Generated %d lines, discovered %d new graphs"), 
           ExecutionLines.Num(), OutGraphsToDefineSeparately.Num());
}

void FDefinitionGenerationHelper::CollectOutputSpecs(
    TSharedPtr<const FBlueprintNode> ExitNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
    FMarkdownPathTracer::EUserGraphType GraphType,
    const FString& GraphNameHint,
    const FString& DefiningGraphAssetContext,
    FMarkdownDataTracer& InDataTracer,
    TArray<FString>& OutOutputSpecs)
{
    if (ExitNode.IsValid())
    {
        TArray<TSharedPtr<FBlueprintPin>> PinsToProcess;
        if (GraphType == FMarkdownPathTracer::EUserGraphType::CustomEventGraph)
        {
            PinsToProcess = ExitNode->GetOutputPins(TEXT(""), false);
        }
        else
        {
            PinsToProcess = ExitNode->GetInputPins(TEXT(""), true);
        }

        for (const TSharedPtr<FBlueprintPin>& Pin : PinsToProcess)
        {
            if (Pin.IsValid() && !Pin->IsExecution())
            {
                TSet<FString> VisitedNodesForPinValue;
                auto PreviousCallsiteArgs = InDataTracer.GetCurrentCallsiteArguments();
                InDataTracer.SetCurrentCallsiteArguments(nullptr);
                
                // ✅ Fix method call - remove extra parameters that don't exist in ResolvePinValueRecursive signature
                FString ValueRepresentation = InDataTracer.ResolvePinValueRecursive(
                    Pin, 
                    GraphNodes, 
                    0, 
                    VisitedNodesForPinValue, 
                    nullptr, // CallingNode
                    nullptr, // OuterNodesMap
                    true,    // bSymbolicTrace
                    DefiningGraphAssetContext
                ); 
                
                InDataTracer.SetCurrentCallsiteArguments(PreviousCallsiteArgs);
                OutOutputSpecs.Add(FString::Printf(TEXT("`%s` (%s) = %s"), 
                    *Pin->Name, *Pin->GetTypeSignature(), *ValueRepresentation));
            }
        }
    }
    OutOutputSpecs.Sort();
}

void FDefinitionGenerationHelper::ExtractInterfaceSignature(
    const FString& GraphNameHint,
    TArray<FString>& OutInputSpecs,
    TArray<FString>& OutOutputSpecs)
{
    UE_LOG(LogPathTracer, Warning, TEXT("ExtractInterfaceSignature: Attempting to extract signature for '%s'"), *GraphNameHint);
    
    // Parse interface name and function name from hint
    auto [InterfaceName, FunctionName] = MarkdownTracerUtils::FGraphNameNormalizer::SplitContextAndItem(GraphNameHint);
    
    if (InterfaceName.IsEmpty() || FunctionName.IsEmpty()) {
        UE_LOG(LogPathTracer, Error, TEXT("ExtractInterfaceSignature: Could not parse interface hint '%s'"), *GraphNameHint);
        OutInputSpecs.Add(TEXT("*Could not parse interface hint*"));
        OutOutputSpecs.Add(TEXT("*Could not parse interface hint*"));
        return;
    }
    

    FString CleanInterfaceName = InterfaceName;
    
    
    UE_LOG(LogPathTracer, Warning, TEXT("ExtractInterfaceSignature: Looking for interface '%s', function '%s'"), *CleanInterfaceName, *FunctionName);
    
    // Find the UBlueprint asset for the interface
    UBlueprint* BPIAsset = nullptr;
    
    // Search for the blueprint asset by name
    for (TObjectIterator<UBlueprint> BlueprintIt; BlueprintIt; ++BlueprintIt) {
        UBlueprint* Blueprint = *BlueprintIt;
        if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Interface) {
            continue; // Skip non-interface blueprints
        }
        
        FString BlueprintName = Blueprint->GetName();
        // Remove Default__ prefix if present
        BlueprintName.RemoveFromStart(TEXT("Default__"));
        // Remove _C suffix if present  
        if (BlueprintName.EndsWith(TEXT("_C"))) {
            BlueprintName.LeftChopInline(2);
        }
        
        if (BlueprintName.Equals(CleanInterfaceName, ESearchCase::IgnoreCase)) {
            BPIAsset = Blueprint;
            UE_LOG(LogPathTracer, Warning, TEXT("ExtractInterfaceSignature: Found BPI asset '%s'"), *Blueprint->GetPathName());
            break;
        }
    }
    
    if (!BPIAsset) {
        UE_LOG(LogPathTracer, Error, TEXT("ExtractInterfaceSignature: Could not find BPI asset for '%s'"), *CleanInterfaceName);
        OutInputSpecs.Add(TEXT("*BPI asset not found - ensure interface is compiled*"));
        OutOutputSpecs.Add(TEXT("*BPI asset not found - ensure interface is compiled*"));
        return;
    }
    
    // Find the UFunction within the interface
    UClass* InterfaceClass = BPIAsset->GeneratedClass;
    if (!InterfaceClass) {
        UE_LOG(LogPathTracer, Error, TEXT("ExtractInterfaceSignature: No GeneratedClass for BPI '%s'"), *CleanInterfaceName);
        OutInputSpecs.Add(TEXT("*No generated class - compile the interface*"));
        OutOutputSpecs.Add(TEXT("*No generated class - compile the interface*"));
        return;
    }
    
    UFunction* InterfaceFunction = InterfaceClass->FindFunctionByName(FName(*FunctionName));
    if (!InterfaceFunction) {
        UE_LOG(LogPathTracer, Error, TEXT("ExtractInterfaceSignature: Function '%s' not found in interface '%s'"), *FunctionName, *CleanInterfaceName);
        OutInputSpecs.Add(FString::Printf(TEXT("*Function '%s' not found in interface*"), *FunctionName));
        OutOutputSpecs.Add(FString::Printf(TEXT("*Function '%s' not found in interface*"), *FunctionName));
        return;
    }
    
    UE_LOG(LogPathTracer, Warning, TEXT("ExtractInterfaceSignature: Found function '%s' in interface '%s'"), *FunctionName, *CleanInterfaceName);
    
    // Extract function parameters
    for (TFieldIterator<FProperty> PropIt(InterfaceFunction); PropIt; ++PropIt) {
        FProperty* Property = *PropIt;
        if (!Property) continue;
        
        FString PropertyName = Property->GetName();
        
        // Determine if this is input or output parameter
        bool bIsReturnParam = Property->HasAnyPropertyFlags(CPF_ReturnParm);
        bool bIsOutParam = Property->HasAnyPropertyFlags(CPF_OutParm) && !bIsReturnParam;
        bool bIsInputParam = !bIsReturnParam && !bIsOutParam;
        
        // Get detailed type description using UE property system
        FString TypeDescription = GetPropertyTypeDescription(Property);
        
        // Format the parameter entry
        FString ParamEntry = FString::Printf(TEXT("`%s` (%s)"), *PropertyName, *TypeDescription);
        
        if (bIsInputParam) {
            OutInputSpecs.Add(ParamEntry);
            UE_LOG(LogPathTracer, Verbose, TEXT("ExtractInterfaceSignature: Added input parameter: %s"), *ParamEntry);
        }
        
        if (bIsOutParam || bIsReturnParam) {
            OutOutputSpecs.Add(ParamEntry);
            UE_LOG(LogPathTracer, Verbose, TEXT("ExtractInterfaceSignature: Added output parameter: %s"), *ParamEntry);
        }
    }
    
    // Sort the specs for consistency
    OutInputSpecs.Sort();
    OutOutputSpecs.Sort();
    
    UE_LOG(LogPathTracer, Warning, TEXT("ExtractInterfaceSignature: Completed extraction for '%s' - %d inputs, %d outputs"), 
        *GraphNameHint, OutInputSpecs.Num(), OutOutputSpecs.Num());
    
    // Add fallback message if no parameters found
    if (OutInputSpecs.IsEmpty()) {
        OutInputSpecs.Add(TEXT("*No input parameters*"));
    }
    if (OutOutputSpecs.IsEmpty()) {
        OutOutputSpecs.Add(TEXT("*No output parameters*"));
    }
}

FString FDefinitionGenerationHelper::GetPropertyTypeDescription(FProperty* Property) const
{
    if (!Property) return TEXT("unknown");
    
    // Handle common property types with detailed descriptions
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property)) {
        return TEXT("bool");
    }
    else if (FByteProperty* ByteProp = CastField<FByteProperty>(Property)) {
        if (ByteProp->Enum) {
            return ByteProp->Enum->GetName(); // Enum type
        }
        return TEXT("byte");
    }
    else if (FIntProperty* IntProp = CastField<FIntProperty>(Property)) {
        return TEXT("int");
    }
    else if (FInt64Property* Int64Prop = CastField<FInt64Property>(Property)) {
        return TEXT("int64");
    }
    else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property)) {
        return TEXT("float");
    }
    else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property)) {
        return TEXT("double");
    }
    else if (FStrProperty* StrProp = CastField<FStrProperty>(Property)) {
        return TEXT("string");
    }
    else if (FNameProperty* NameProp = CastField<FNameProperty>(Property)) {
        return TEXT("name");
    }
    else if (FTextProperty* TextProp = CastField<FTextProperty>(Property)) {
        return TEXT("text");
    }
    else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property)) {
        if (ObjProp->PropertyClass) {
            FString ClassName = ObjProp->PropertyClass->GetName();
            // Clean up common UE class name patterns
            ClassName.RemoveFromStart(TEXT("Default__"));
            if (ClassName.EndsWith(TEXT("_C"))) {
                ClassName.LeftChopInline(2);
            }
            return ClassName;
        }
        return TEXT("object");
    }
    else if (FClassProperty* ClassProp = CastField<FClassProperty>(Property)) {
        if (ClassProp->MetaClass) {
            FString ClassName = ClassProp->MetaClass->GetName();
            ClassName.RemoveFromStart(TEXT("Default__"));
            if (ClassName.EndsWith(TEXT("_C"))) {
                ClassName.LeftChopInline(2);
            }
            return FString::Printf(TEXT("Class<%s>"), *ClassName);
        }
        return TEXT("class");
    }
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property)) {
        if (StructProp->Struct) {
            return StructProp->Struct->GetName();
        }
        return TEXT("struct");
    }
    else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property)) {
        FString InnerType = GetPropertyTypeDescription(ArrayProp->Inner);
        return FString::Printf(TEXT("Array<%s>"), *InnerType);
    }
    else if (FSetProperty* SetProp = CastField<FSetProperty>(Property)) {
        FString ElementType = GetPropertyTypeDescription(SetProp->ElementProp);
        return FString::Printf(TEXT("Set<%s>"), *ElementType);
    }
    else if (FMapProperty* MapProp = CastField<FMapProperty>(Property)) {
        FString KeyType = GetPropertyTypeDescription(MapProp->KeyProp);
        FString ValueType = GetPropertyTypeDescription(MapProp->ValueProp);
        return FString::Printf(TEXT("Map<%s, %s>"), *KeyType, *ValueType);
    }
    else if (FDelegateProperty* DelegateProp = CastField<FDelegateProperty>(Property)) {
        return TEXT("delegate");
    }
    else if (FMulticastDelegateProperty* MultiDelegateProp = CastField<FMulticastDelegateProperty>(Property)) {
        return TEXT("multicast delegate");
    }
    
    // Fallback to class name
    return Property->GetClass()->GetName();
}

TSharedPtr<const FBlueprintNode> FDefinitionGenerationHelper::FindEntryNodeForInputs(
    const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
    FMarkdownPathTracer::EUserGraphType GraphType,
    const FString& SimpleNameFromHint) // Simple name, e.g., "MyFunction", "MyEvent"
{
    switch (GraphType)
    {
    case FMarkdownPathTracer::EUserGraphType::Interface:
        {
            // Interfaces don't have traditional entry nodes in their graphs
            // We need to extract signature from UFunction metadata instead
            UE_LOG(LogPathTracer, Warning, TEXT("FindEntryNodeForInputs: Interface type detected for '%s' - signature extraction needed"), *SimpleNameFromHint);
            return nullptr; // Signal that we need special interface handling
        }
        
        case FMarkdownPathTracer::EUserGraphType::Function:
            for (const auto& Pair : GraphNodes)
            {
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("FunctionEntry"))
                {
                    // Match by function name property if available, or by node's display name as fallback
                    const FString* FuncNameProp = Pair.Value->RawProperties.Find(TEXT("Function Name")); // K2Node_FunctionEntry
                    if (FuncNameProp && FuncNameProp->Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value;
                    // Fallback to node name (though FunctionEntry nodes are often just "Function Entry")
                    // Or if the SimpleNameFromHint is what UEdGraph::GetFName() would return for the UFunction
                    if (Pair.Value->Name.Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value; 
                }
            }
            // If specific name match fails, return first FunctionEntry found (less ideal)
            for (const auto& Pair : GraphNodes) if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("FunctionEntry")) return Pair.Value;
            break;

        case FMarkdownPathTracer::EUserGraphType::CustomEventGraph: // Custom Event itself is the entry
            for (const auto& Pair : GraphNodes)
            {
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("CustomEvent"))
                {
                    const FString* EventNameProp = Pair.Value->RawProperties.Find(TEXT("CustomFunctionName")); // K2Node_CustomEvent
                    if (EventNameProp && EventNameProp->Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value;
                    if (Pair.Value->Name.Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value;
                }
            }
            // Fallback to first CustomEvent node
            for (const auto& Pair : GraphNodes) if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("CustomEvent")) return Pair.Value;
            break;

        case FMarkdownPathTracer::EUserGraphType::Macro: // Macro uses Tunnel node for inputs
        case FMarkdownPathTracer::EUserGraphType::CollapsedGraph: // Collapsed graph also uses Tunnel
            for (const auto& Pair : GraphNodes)
            {
                // Tunnel nodes for inputs usually have "Input" in their name or specific metadata
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("Tunnel"))
                {
                    // Check if it's an input tunnel (e.g., K2Node_Tunnel has GetGraphEntryPointHandle)
                    // Heuristic: input tunnels usually have output data pins, no input exec.
                    bool bHasOutputData = false; bool bHasInputExec = false;
                    for(const auto& PinPair : Pair.Value->Pins) {
                        if(PinPair.Value->IsExecution() && PinPair.Value->Direction == TEXT("EGPD_Input")) bHasInputExec = true;
                        if(!PinPair.Value->IsExecution() && PinPair.Value->Direction == TEXT("EGPD_Output")) bHasOutputData = true;
                    }

                    if(bHasOutputData && !bHasInputExec) return Pair.Value; // Good candidate for input tunnel

                    // Fallback name check (less reliable)
                    if (Pair.Value->Name.Contains(TEXT("Inputs")) || Pair.Value->Name.Contains(TEXT("Input"))) return Pair.Value;
                }
            }
            break;
        default:
            break;
    }
    UE_LOG(LogPathTracer, Warning, TEXT("FindEntryNodeForInputs: Could not find entry node for graph type %d, name '%s'"), (int32)GraphType, *SimpleNameFromHint);
    return nullptr;
}

TSharedPtr<const FBlueprintNode> FDefinitionGenerationHelper::FindExitNodeForOutputs(
    const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
    FMarkdownPathTracer::EUserGraphType GraphType,
    const FString& SimpleNameFromHint)
{
    switch (GraphType)
    {   case FMarkdownPathTracer::EUserGraphType::Interface:
        {
            // Interfaces don't have traditional exit nodes in their graphs
            // We need to extract signature from UFunction metadata instead
            UE_LOG(LogPathTracer, Warning, TEXT("FindExitNodeForOutputs: Interface type detected for '%s' - signature extraction needed"), *SimpleNameFromHint);
            return nullptr; // Signal that we need special interface handling
        }
        
        case FMarkdownPathTracer::EUserGraphType::Function:
            for (const auto& Pair : GraphNodes)
            {
                // FunctionResult node is the standard exit for functions with outputs
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("FunctionResult")) return Pair.Value;
            }
            break;

        case FMarkdownPathTracer::EUserGraphType::CustomEventGraph:
            // For Custom Events, outputs are on the CustomEvent node itself. So, "exit" is same as "entry".
            for (const auto& Pair : GraphNodes)
            {
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("CustomEvent"))
                {
                    const FString* EventNameProp = Pair.Value->RawProperties.Find(TEXT("CustomFunctionName"));
                    if (EventNameProp && EventNameProp->Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value;
                     if (Pair.Value->Name.Equals(SimpleNameFromHint, ESearchCase::IgnoreCase)) return Pair.Value;
                }
            }
            for (const auto& Pair : GraphNodes) if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("CustomEvent")) return Pair.Value; // Fallback
            break;

        case FMarkdownPathTracer::EUserGraphType::Macro:
        case FMarkdownPathTracer::EUserGraphType::CollapsedGraph:
            for (const auto& Pair : GraphNodes)
            {
                if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("Tunnel"))
                {
                    // Output tunnels usually have input data pins and an output exec pin.
                    bool bHasInputData = false; bool bHasOutputExec = false;
                     for(const auto& PinPair : Pair.Value->Pins) {
                         if(PinPair.Value->IsExecution() && PinPair.Value->IsOutput()) bHasOutputExec = true;
                         if(!PinPair.Value->IsExecution() && PinPair.Value->IsInput()) bHasInputData = true;
                     }
                    if(bHasInputData && bHasOutputExec) return Pair.Value; // Good candidate for output tunnel

                    // Fallback name check
                    if (Pair.Value->Name.Contains(TEXT("Outputs")) || Pair.Value->Name.Contains(TEXT("Output"))) return Pair.Value;
                }
            }
            break;
        default:
            break;
    }
    UE_LOG(LogPathTracer, Warning, TEXT("FindExitNodeForOutputs: Could not find exit node for graph type %d, name '%s'"), (int32)GraphType, *SimpleNameFromHint);
    return nullptr;
}

FString FDefinitionGenerationHelper::ExtractSimpleNameFromHint(const FString& GraphNameHint) const
{
    FString Name = GraphNameHint;
    int32 Pos = -1;
    // Try to find the last '.' which typically separates Blueprint name from Function/Event/Macro name
    Name.FindLastChar(TEXT('.'), Pos);
    if (Pos == INDEX_NONE) 
    {
        // If no '.', it might be a graph within the current BP context, like a Collapsed Graph or Macro not in a library
        // Or it could be an event name directly if RootBlueprintNameForTrace is the context.
        // Try ':' for other path-like structures if any.
        Name.FindLastChar(TEXT(':'), Pos); 
    }

    if (Pos != INDEX_NONE) 
    {
        Name = GraphNameHint.Mid(Pos + 1);
    }
    // If still no separator, the hint itself might be the simple name (e.g. "MyCollapsedGraph")
    return Name;
}

FString FDefinitionGenerationHelper::DetermineAssetContext(const FString& GraphNameHint, const FString& RootBlueprintNameForTrace) const
{
    FString Context = GraphNameHint;
    int32 Pos = -1;
    Context.FindLastChar(TEXT('.'), Pos); // Functions/Events in other BPs: BlueprintName.FunctionName
    // Macros can be BlueprintGeneratedClass'/Game/Path/To/MacroLib.MacroLib_C':MacroName
    // Or sometimes just MacroLibraryName.MacroName if path stripping happened.
    
    if (Pos == INDEX_NONE) 
    {
        Context.FindLastChar(TEXT(':'), Pos); // Check for ':' if '.' not found
    }

    if (Pos != INDEX_NONE) 
    {
        Context = Context.Left(Pos);
        // Further clean up if it's a full path like BlueprintGeneratedClass'/Game/Path/To/MacroLib.MacroLib_C'
        if (Context.StartsWith(TEXT("BlueprintGeneratedClass'")))
        {
            Context.RemoveFromStart(TEXT("BlueprintGeneratedClass'"));
            Context.RemoveFromEnd(TEXT("'"));
            FString PackagePath, AssetName, AssetSubPath;
            FString PathToSplit = Context;
            if (PathToSplit.EndsWith(TEXT("_C"))) PathToSplit.LeftChopInline(2); // Remove _C suffix
            
            FPackageName::SplitLongPackageName(PathToSplit, PackagePath, AssetName, AssetSubPath);
            Context = AssetName; // Get just the asset name
        }

    }
    else // No separator, assume it's within the current root blueprint or it's a name that is its own context
    {
        Context = RootBlueprintNameForTrace.IsEmpty() ? TEXT("UnknownContext") : RootBlueprintNameForTrace;
        // If GraphNameHint itself looks like an asset (e.g. "MyMacroLibrary"), that could be the context.
        // This part might need more sophisticated logic if hints are very diverse.
        if (RootBlueprintNameForTrace.IsEmpty() && !GraphNameHint.IsEmpty()) Context = GraphNameHint; // Fallback
    }
    return Context;
}