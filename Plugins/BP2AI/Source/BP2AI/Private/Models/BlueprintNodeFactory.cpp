/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Models/BlueprintNodeFactory.cpp


#include "Models/BlueprintNodeFactory.h" // Include self header

// Engine Includes for Node Data/Types
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/Class.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h" // For FMemberReference etc.
#include "EdGraph/EdGraph.h" // For UEdGraph (Macro/Composite)

// Specific K2Node Includes Needed by Handlers
#include "K2Node.h" // Base class for IsA check
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_ComponentBoundEvent.h"
#include "K2Node_ActorBoundEvent.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_PromotableOperator.h"
#include "K2Node_CommutativeAssociativeBinaryOperator.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_Message.h" // For Interface calls
#include "K2Node_MacroInstance.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_AssignDelegate.h"
#include "K2Node_RemoveDelegate.h"
#include "K2Node_ClearDelegate.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_InputKey.h"             // <-- ADDED
#include "K2Node_InputAxisEvent.h"       // <-- ADDED
#include "K2Node_InputAction.h"          // <-- ADDED
#include "K2Node_InputTouchEvent.h"      // <-- ADDED (Correct class name)
#include "K2Node_InputAxisKeyEvent.h"    // <-- ADDED
#include "Kismet/KismetSystemLibrary.h" // <-- Might be needed for Enhanced Input Action path
#include "EnhancedInputComponent.h"     // <-- Might be needed for Enhanced Input Action path
#include "InputMappingContext.h"        // <-- Might be needed for Enhanced Input Action path
#include "InputAction.h"                // <-- ADDED (For Enhanced Input Action)
#include "K2Node_EnhancedInputAction.h" // <-- ADDED
#include "K2Node_MakeStruct.h"          // <-- ADDED
#include "K2Node_BreakStruct.h"         // <-- ADDED
#include "K2Node_SwitchEnum.h"          // <-- ADDED
#include "K2Node_SpawnActorFromClass.h" // <-- ADDED
#include "K2Node_AddComponent.h"        // <-- ADDED
#include "Editor/UMGEditor/Private/Nodes/K2Node_CreateWidget.h"
#include "K2Node_GetClassDefaults.h"     // <-- ADDED
#include "K2Node_GetSubsystem.h"         // <-- ADDED
#include "K2Node_CallParentFunction.h"   // <-- ADDED
#include "K2Node_CallArrayFunction.h"    // <-- ADDED
#include "K2Node_Composite.h"            // <-- ADDED
#include "K2Node_Timeline.h"             // <-- ADDED
#include "UObject/NameTypes.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/BlueprintGeneratedClass.h"


// Utility Includes
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Containers/Map.h"
#include "Logging/BP2AILog.h"


#include "Kismet2/BlueprintEditorUtils.h"  // For FBlueprintEditorUtils::FindBlueprintForNode
#include "Engine/DataAsset.h"


#ifndef NAME_Struct
#define NAME_Struct FName(TEXT("struct"))
#endif

#ifndef NAME_Enum
#define NAME_Enum FName(TEXT("enum"))
#endif


// --- Hex Encode/Decode Helpers ---
namespace BlueprintNodeFactory_Private
{
	// ... (HexEncode and HexDecode functions remain unchanged) ...
	FString HexEncode(const FString& Input)
	{
		FString Result;
		Result.Reserve(Input.Len() * 4); // Reserve space for 4 hex chars per TCHAR
		for (TCHAR Char : Input)
		{
			// %04X formats the character code as a 4-digit uppercase hexadecimal number, padding with 0 if needed
			Result += FString::Printf(TEXT("%04X"), (int32)Char);
		}
		return Result;
	}

	// Helper to Hex Decode
	FString HexDecode(const FString& HexString)
	{
		FString Result;
		// Reserve space assuming 4 hex chars per original TCHAR
		Result.Reserve(HexString.Len() / 4);
		for (int32 i = 0; i < HexString.Len(); i += 4)
		{
			// Ensure we have a full 4-character sequence
			if (i + 4 <= HexString.Len())
			{
				FString HexChar = HexString.Mid(i, 4);
				// Convert hex string to integer
				int32 CharValue = FCString::Strtoi(*HexChar, nullptr, 16);
				// Basic validation: Ensure the decoded value is within the valid range for TCHAR (typically 16-bit for UE)
				if (CharValue >= 0 && CharValue <= 0xFFFF)
				{
					Result.AppendChar(static_cast<TCHAR>(CharValue));
				}
				else
				{
					UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("HexDecode: Invalid hex character sequence '%s' resulted in out-of-range value %d"), *HexChar, CharValue);
				}
			}
			else
			{
				// Log if the hex string length isn't a multiple of 4
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("HexDecode: Incomplete hex sequence at end of string '%s'. String length %d not multiple of 4."), *HexString, HexString.Len());
				break; // Stop processing if we hit an incomplete sequence
			}
		}
		return Result;
	}
}
// --- END Helper Functions ---


// --- Define static members ---
TMap<FName, FBlueprintNodeFactory::FNodePropertyExtractorFunc> FBlueprintNodeFactory::PropertyExtractors;
bool FBlueprintNodeFactory::bExtractorsInitialized = false;


// --- Namespace for Handler Implementations ---
namespace NodeFactoryExtractors
{
	// --- Function/Event Handlers ---
	static void ExtractProps_CallFunction(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
{
    if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(GraphNode))
    {
        // --- EXISTING PROPERTY EXTRACTION (kept from your snippet) ---
        if (CallFuncNode->GetFunctionName() != NAME_None) {
            OutNodeModel->RawProperties.Add(TEXT("FunctionName"), CallFuncNode->GetFunctionName().ToString());
        }
        OutNodeModel->RawProperties.Add(TEXT("bIsPureFunc"), CallFuncNode->IsNodePure() ? TEXT("true") : TEXT("false"));
        OutNodeModel->RawProperties.Add(TEXT("bIsLatent"), CallFuncNode->IsLatentFunction() ? TEXT("true") : TEXT("false"));

        // --- DETAILED DEBUG LOGGING FOR FUNCTION REFERENCE RESOLUTION ---
        const FMemberReference& FuncRef = CallFuncNode->FunctionReference;
        UClass* BlueprintClassFromNode = CallFuncNode->GetBlueprintClassFromNode(); // Class of the BP graph the node is in
        // For GetMemberParentClass, it's important to pass the class of the Blueprint graph
        // that contains this K2Node_CallFunction. This provides the correct 'self' context for the call.
        UClass* ResolvedMemberParentClass = FuncRef.GetMemberParentClass(BlueprintClassFromNode); 

        FString NodeTitleForLog = GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString();
        FString NodeTypeForLog = GraphNode->GetClass()->GetName(); // e.g., K2Node_CallFunction, K2Node_CallParentFunction

        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("FactoryDebug: Processing Node Title: '%s', Node Type: '%s', Node GUID: %s"),
            *NodeTitleForLog, *NodeTypeForLog, *GraphNode->NodeGuid.ToString());
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: FuncRef MemberName: '%s'"), *FuncRef.GetMemberName().ToString());
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: Context BP Class From Node (for GetMemberParentClass): '%s'"),
            BlueprintClassFromNode ? *BlueprintClassFromNode->GetPathName() : TEXT("NULL_CONTEXT_CLASS"));
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: Resolved MemberParentClass (where function is declared by FMemberReference): '%s'"),
            ResolvedMemberParentClass ? *ResolvedMemberParentClass->GetPathName() : TEXT("NULL_MEMBER_PARENT_CLASS"));

        // This is where 'FunctionParentClassPath' is stored if ResolvedMemberParentClass is valid.
        // This property is what FormatCallParentFunction tries to use.
        if (ResolvedMemberParentClass) // This 'if' checks if GetMemberParentClass returned a valid UClass*
        {
            OutNodeModel->RawProperties.Add(TEXT("FunctionParentClassPath"), ResolvedMemberParentClass->GetPathName());
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: Stored 'FunctionParentClassPath': '%s'"), *ResolvedMemberParentClass->GetPathName());
        }
        else
        {
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: 'FunctionParentClassPath' NOT stored because ResolvedMemberParentClass was NULL."));
        }

        // Specifically for K2Node_CallParentFunction, also log the SuperFunctionName that its dedicated handler might store.
        // Note: ExtractProps_CallParentFunction in your factory ALREADY stores "SuperFunctionName".
        // This log helps see if it's consistent with FuncRef.GetMemberName().
        if (NodeTypeForLog == TEXT("K2Node_CallParentFunction"))
        {
            const FString* SuperFuncNameFromProps = OutNodeModel->RawProperties.Find(TEXT("SuperFunctionName"));
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: (For K2Node_CallParentFunction) SuperFunctionName from props (if already set by its specific handler): '%s'"),
                SuperFuncNameFromProps ? **SuperFuncNameFromProps : TEXT("Not yet set or not applicable here"));
            // Also log what FuncRef.GetMemberName() is for this CallParentFunction node
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug: (For K2Node_CallParentFunction) FuncRef.GetMemberName() is: '%s'"), *FuncRef.GetMemberName().ToString());
        }
    }
}
	
	// --- ADDED: Call Parent Function ---
	static void ExtractProps_CallParentFunction(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
{
    if (UK2Node_CallParentFunction* CallParentFuncNode = Cast<UK2Node_CallParentFunction>(GraphNode)) // Renamed Node to CallParentFuncNode for clarity
    {
        // --- Store SuperFunctionName (existing logic) ---
        FName ParentFunctionName = CallParentFuncNode->FunctionReference.GetMemberName();
        if (ParentFunctionName != NAME_None)
        {
            OutNodeModel->RawProperties.Add(TEXT("SuperFunctionName"), ParentFunctionName.ToString());
            UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor (CallParentFunction): Stored SuperFunctionName: '%s' for Node %s"), 
                *ParentFunctionName.ToString(), *OutNodeModel->Guid.Left(8));
        }
        else
        {
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor (CallParentFunction): Failed to extract SuperFunctionName for Node %s"), *OutNodeModel->Guid.Left(8));
        }

        // --- ADDED: Store FunctionParentClassPath for the parent function ---
        const FMemberReference& FuncRef = CallParentFuncNode->FunctionReference;
        UClass* BlueprintClassFromNode = CallParentFuncNode->GetBlueprintClassFromNode(); // Context: Class of the BP containing this CallParentFunction node
        UClass* ResolvedMemberParentClass = FuncRef.GetMemberParentClass(BlueprintClassFromNode); // This should resolve to the actual parent class declaring the function

        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("FactoryDebug (CallParent): Node '%s' (GUID: %s)"), *CallParentFuncNode->GetNodeTitle(ENodeTitleType::ListView).ToString(), *GraphNode->NodeGuid.ToString());
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug (CallParent): FuncRef MemberName: '%s'"), *FuncRef.GetMemberName().ToString());
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug (CallParent): Context BP Class From Node: '%s'"), BlueprintClassFromNode ? *BlueprintClassFromNode->GetPathName() : TEXT("NULL_CONTEXT_CLASS"));
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug (CallParent): Resolved MemberParentClass for SuperCall: '%s'"), ResolvedMemberParentClass ? *ResolvedMemberParentClass->GetPathName() : TEXT("NULL_MEMBER_PARENT_CLASS"));

        if (ResolvedMemberParentClass)
        {
            OutNodeModel->RawProperties.Add(TEXT("FunctionParentClassPath"), ResolvedMemberParentClass->GetPathName());
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug (CallParent): Stored 'FunctionParentClassPath': '%s' for SuperCall"), *ResolvedMemberParentClass->GetPathName());
        }
        else
        {
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDebug (CallParent): 'FunctionParentClassPath' NOT stored for SuperCall because ResolvedMemberParentClass was NULL."));
        }
    }
}
	// --- ADDED: Call Array Function ---
	static void ExtractProps_CallArrayFunction(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_CallArrayFunction* Node = Cast<UK2Node_CallArrayFunction>(GraphNode))
		{
			// Stores the specific array function (e.g., Array_Add, Array_Get) in FunctionReference
			if (Node->FunctionReference.GetMemberName() != NAME_None)
			{
				OutNodeModel->RawProperties.Add(TEXT("FunctionName"), Node->FunctionReference.GetMemberName().ToString());
			}
		}
	}


	// --- Delegate Binding Handlers ---
	static void ExtractProps_DelegateBinding(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		// Common handler for Add, Assign, Remove, Clear, Call which use DelegateReference
		FName DelegateMemberName = NAME_None;
		FString DelegateMemberNameStr;

		if (UK2Node_AddDelegate* AddDelegateNode = Cast<UK2Node_AddDelegate>(GraphNode)) { DelegateMemberName = AddDelegateNode->DelegateReference.GetMemberName(); }
		else if (UK2Node_AssignDelegate* AssignDelegateNode = Cast<UK2Node_AssignDelegate>(GraphNode)) { DelegateMemberName = AssignDelegateNode->DelegateReference.GetMemberName(); }
		else if (UK2Node_RemoveDelegate* RemoveDelegateNode = Cast<UK2Node_RemoveDelegate>(GraphNode)) { DelegateMemberName = RemoveDelegateNode->DelegateReference.GetMemberName(); }
		else if (UK2Node_ClearDelegate* ClearDelegateNode = Cast<UK2Node_ClearDelegate>(GraphNode)) { DelegateMemberName = ClearDelegateNode->DelegateReference.GetMemberName(); } // Still potentially incorrect for Clear
		else if (UK2Node_CallDelegate* CallDelegateNode = Cast<UK2Node_CallDelegate>(GraphNode)) { DelegateMemberName = CallDelegateNode->DelegateReference.GetMemberName(); }
		
		if (DelegateMemberName != NAME_None)
		{
			DelegateMemberNameStr = DelegateMemberName.ToString();
			OutNodeModel->RawProperties.Add(TEXT("DelegateReference_MemberName"), DelegateMemberNameStr);
			UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: Extracted DelegateReference_MemberName '%s' for Node %s"), *DelegateMemberNameStr, *OutNodeModel->Guid.Left(8));
		}
		else
		{
			if (Cast<UK2Node_ClearDelegate>(GraphNode)) {
				UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: DelegateReference_MemberName not found or N/A for Node %s (UK2Node_ClearDelegate). This might be expected."), *OutNodeModel->Guid.Left(8));
			} else {
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to extract DelegateReference_MemberName for Node %s (%s)"), *OutNodeModel->Guid.Left(8), *OutNodeModel->NodeType);
			}
		}
	}

	static void ExtractProps_CreateDelegate(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_CreateDelegate* Node = Cast<UK2Node_CreateDelegate>(GraphNode))
		{
			if (Node->SelectedFunctionName != NAME_None) // Correct property name
			{
				OutNodeModel->RawProperties.Add(TEXT("SelectedFunctionName"), Node->SelectedFunctionName.ToString());
				UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: Extracted SelectedFunctionName '%s' for Node %s"), *Node->SelectedFunctionName.ToString(), *OutNodeModel->Guid.Left(8));
			}
			 else { UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to extract SelectedFunctionName for Node %s (CreateDelegate)"), *OutNodeModel->Guid.Left(8)); }
		}
	}

	// --- Event Handlers (Includes Bound Events) ---
	static void ExtractProps_Event(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(GraphNode))
		{
			if (EventNode->EventReference.GetMemberName() != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("EventFunctionName"), EventNode->EventReference.GetMemberName().ToString()); }
			if (UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(GraphNode))
			{
				if (CustomEventNode->CustomFunctionName != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("CustomFunctionName"), CustomEventNode->CustomFunctionName.ToString()); }
			}
		}
	}

	static void ExtractProps_ComponentBoundEvent(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		 if (UK2Node_ComponentBoundEvent* CompEventNode = Cast<UK2Node_ComponentBoundEvent>(GraphNode))
		 {
			 UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: CompBoundEvent %s"), *OutNodeModel->Guid.Left(8));
			 FString OriginalCompStr = CompEventNode->ComponentPropertyName.ToString();
			 FString OriginalDelStr = CompEventNode->DelegatePropertyName.ToString();
			 // FString OriginalOwnerStr_DEPRECATED = CompEventNode->DelegateOwnerClass ? CompEventNode->DelegateOwnerClass->GetPathName() : FString(TEXT("")); // No longer primary source for owning BP
			 // UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("    Extractor: CompEvent %s - Raw Values (DelegateOwnerClass path): Owner='%s'"), *OutNodeModel->Guid.Left(8), *OriginalOwnerStr_DEPRECATED);

			 OutNodeModel->PreservedCompPropName = BlueprintNodeFactory_Private::HexDecode(BlueprintNodeFactory_Private::HexEncode(OriginalCompStr));
			 OutNodeModel->PreservedDelPropName = BlueprintNodeFactory_Private::HexDecode(BlueprintNodeFactory_Private::HexEncode(OriginalDelStr));
		 	
			 if (!OutNodeModel->PreservedDelPropName.IsEmpty()) { OutNodeModel->RawProperties.Add(TEXT("DelegatePropertyName"), OutNodeModel->PreservedDelPropName); }
			 if (!OutNodeModel->PreservedCompPropName.IsEmpty()) { OutNodeModel->RawProperties.Add(TEXT("ComponentPropertyName"), OutNodeModel->PreservedCompPropName); }

			// --- NEW LOGIC FOR OWNING BLUEPRINT ---
		 	UK2Node* K2GraphNode = Cast<UK2Node>(GraphNode); // Cast to UK2Node to access GetBlueprint()
		 	UBlueprint* OwningBlueprint = K2GraphNode ? K2GraphNode->GetBlueprint() : nullptr;
			if (OwningBlueprint)
			{
				OutNodeModel->BoundEventOwnerClassPath = OwningBlueprint->GetPathName(); // Store the full path
				UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor (CompEvent %s): Correctly set BoundEventOwnerClassPath to: '%s'"), *OutNodeModel->Guid.Left(8), *OutNodeModel->BoundEventOwnerClassPath);
			}
			else
			{
				// Keep the default "InitialTestOwnerPath" or set a specific fallback if GetBlueprint() fails
				OutNodeModel->BoundEventOwnerClassPath = TEXT("UnknownOwningBlueprint_CompEvent");
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("    Extractor (CompEvent %s): Could not get OwningBlueprint via GraphNode->GetBlueprint(). BoundEventOwnerClassPath set to fallback '%s'."), *OutNodeModel->Guid.Left(8), *OutNodeModel->BoundEventOwnerClassPath);
			}
			// --- END NEW LOGIC ---
		 }
	}

	static void ExtractProps_ActorBoundEvent(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_ActorBoundEvent* ActorEventNode = Cast<UK2Node_ActorBoundEvent>(GraphNode))
		{
			UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: ActorBoundEvent %s"), *OutNodeModel->Guid.Left(8));
			
			FString OriginalDelStr = ActorEventNode->DelegatePropertyName.ToString();
			OutNodeModel->PreservedDelPropName = BlueprintNodeFactory_Private::HexDecode(BlueprintNodeFactory_Private::HexEncode(OriginalDelStr));
			if (!OutNodeModel->PreservedDelPropName.IsEmpty()) { OutNodeModel->RawProperties.Add(TEXT("DelegatePropertyName"), OutNodeModel->PreservedDelPropName); }

			// --- NEW LOGIC FOR OWNING BLUEPRINT ---
			UK2Node* K2GraphNode = Cast<UK2Node>(GraphNode); // Cast to UK2Node to access GetBlueprint()
			UBlueprint* OwningBlueprint = K2GraphNode ? K2GraphNode->GetBlueprint() : nullptr;
			if (OwningBlueprint)
			{
				OutNodeModel->BoundEventOwnerClassPath = OwningBlueprint->GetPathName(); // Store the full path
				UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor (ActorEvent %s): Correctly set BoundEventOwnerClassPath to: '%s'"), *OutNodeModel->Guid.Left(8), *OutNodeModel->BoundEventOwnerClassPath);
			}
			else
			{
				// Keep the default "InitialTestOwnerPath" or set a specific fallback
				OutNodeModel->BoundEventOwnerClassPath = TEXT("UnknownOwningBlueprint_ActorEvent");
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("    Extractor (ActorEvent %s): Could not get OwningBlueprint via GraphNode->GetBlueprint(). BoundEventOwnerClassPath set to fallback '%s'."), *OutNodeModel->Guid.Left(8), *OutNodeModel->BoundEventOwnerClassPath);
			}
			// --- END NEW LOGIC ---
		}
	}
	
	// --- ADDED: Input Event Handlers ---
	static void ExtractProps_EnhancedInputAction(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_EnhancedInputAction* Node = Cast<UK2Node_EnhancedInputAction>(GraphNode))
		{
			if (Node->InputAction)
			{
				OutNodeModel->RawProperties.Add(TEXT("InputActionPath"), Node->InputAction->GetPathName());
			}
		}
	}
	static void ExtractProps_InputAxisEvent(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_InputAxisEvent* Node = Cast<UK2Node_InputAxisEvent>(GraphNode))
		{
			if (Node->InputAxisName != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("InputAxisName"), Node->InputAxisName.ToString()); }
		}
	}
	static void ExtractProps_InputAction(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_InputAction* Node = Cast<UK2Node_InputAction>(GraphNode))
		{
			if (Node->InputActionName != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("InputActionName"), Node->InputActionName.ToString()); }
		}
	}
	static void ExtractProps_InputKey(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		// Handles InputKey and InputDebugKey as they share the 'InputKey' property
		if (UK2Node_InputKey* Node = Cast<UK2Node_InputKey>(GraphNode))
		{
			OutNodeModel->RawProperties.Add(TEXT("InputKey"), Node->InputKey.ToString()); // FKey needs ToString()
		}
		// Cast to InputDebugKey explicitly if needed, but InputKey property is likely the same
		// else if (UK2Node_InputDebugKey* DebugNode = Cast<UK2Node_InputDebugKey>(GraphNode)) { ... }
	}
	static void ExtractProps_InputAxisKeyEvent(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_InputAxisKeyEvent* Node = Cast<UK2Node_InputAxisKeyEvent>(GraphNode))
		{
			OutNodeModel->RawProperties.Add(TEXT("AxisKey"), Node->AxisKey.ToString()); // FKey needs ToString()
		}
	}
	// InputTouchEvent has no specific named properties to extract beyond standard event pins

	// --- Struct Handlers ---
	static void ExtractProps_MakeStruct(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_MakeStruct* Node = Cast<UK2Node_MakeStruct>(GraphNode))
		{
			if (Node->StructType) { OutNodeModel->RawProperties.Add(TEXT("StructType"), Node->StructType->GetPathName()); }
		}
	}
	static void ExtractProps_BreakStruct(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_BreakStruct* Node = Cast<UK2Node_BreakStruct>(GraphNode))
		{
			if (Node->StructType) { OutNodeModel->RawProperties.Add(TEXT("StructType"), Node->StructType->GetPathName()); }
		}
	}
	// SetFieldsInStruct doesn't have a direct StructType property, usually inferred from pins

	// --- Switch Handlers ---
	static void ExtractProps_SwitchEnum(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_SwitchEnum* Node = Cast<UK2Node_SwitchEnum>(GraphNode))
		{
			if (Node->Enum) { OutNodeModel->RawProperties.Add(TEXT("Enum"), Node->Enum->GetPathName()); }
		}
	}
	// SwitchName, SwitchString, SwitchInteger don't have specific type properties on the node itself

	// --- Object Creation/Management ---
	static void ExtractProps_SpawnActorFromClass(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_SpawnActorFromClass* Node = Cast<UK2Node_SpawnActorFromClass>(GraphNode))
		{
			if (Node->GetClassToSpawn()) { OutNodeModel->RawProperties.Add(TEXT("ClassToSpawn"), Node->GetClassToSpawn()->GetPathName()); }
		}
	}
	static void ExtractProps_AddComponent(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_AddComponent* Node = Cast<UK2Node_AddComponent>(GraphNode))
		{
			// Use the public helper GetTemplateFromNode() which resolves the component class/template
			UActorComponent* TemplateComponent = Node->GetTemplateFromNode();
			UClass* ComponentClass = TemplateComponent ? TemplateComponent->GetClass() : nullptr;

			if (ComponentClass)
			{
				OutNodeModel->RawProperties.Add(TEXT("ComponentClass"), ComponentClass->GetPathName());
			}
			// Fallback: Check TemplateBlueprint property if GetTemplateFromNode failed (for BP components)
			else if (!Node->TemplateBlueprint.IsEmpty())
			{
				OutNodeModel->RawProperties.Add(TEXT("TemplateBlueprint"), Node->TemplateBlueprint);
			}
			// Fallback: check TemplateType (less common?)
			else if (Node->TemplateType)
			{
				OutNodeModel->RawProperties.Add(TEXT("TemplateType"), Node->TemplateType->GetPathName());
			}
			else
			{
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine Component Class for AddComponent node %s"), *OutNodeModel->Guid.Left(8));
			}
		}
	}
	
	static void ExtractProps_CreateWidget(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_CreateWidget* Node = Cast<UK2Node_CreateWidget>(GraphNode))
		{
			// K2Node_CreateWidget inherits from UK2Node_ConstructObjectFromClass.
			// We need to get the class from the 'Class' pin's type or default object.
			UClass* WidgetUClass = nullptr;
			UEdGraphPin* ClassPin = Node->GetClassPin(); // Use the base class getter
			if (ClassPin)
			{
				if (ClassPin->DefaultObject != nullptr && ClassPin->DefaultObject->IsA<UClass>())
				{
					WidgetUClass = Cast<UClass>(ClassPin->DefaultObject);
				}
				// If not set by default object, try the pin type's subcategory object
				else if (ClassPin->PinType.PinSubCategoryObject.IsValid())
				{
					WidgetUClass = Cast<UClass>(ClassPin->PinType.PinSubCategoryObject.Get());
				}
			}

			if (WidgetUClass)
			{
				OutNodeModel->RawProperties.Add(TEXT("WidgetClass"), WidgetUClass->GetPathName());
			}
			else
			{
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to get WidgetClass for CreateWidget node %s"), *OutNodeModel->Guid.Left(8));
			}
		}
	}
	static void ExtractProps_GenericCreateObject(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_GenericCreateObject* Node = Cast<UK2Node_GenericCreateObject>(GraphNode))
		{
			// GenericCreateObject relies on the input "Class" pin.
			// Use the GetClassPin() helper inherited from UK2Node_ConstructObjectFromClass.
			UEdGraphPin* ClassPin = Node->GetClassPin();
			UClass* ObjectClass = nullptr;

			if (ClassPin)
			{
				// Check the pin's default object first
				if (ClassPin->DefaultObject && ClassPin->DefaultObject->IsA<UClass>())
				{
					ObjectClass = Cast<UClass>(ClassPin->DefaultObject);
				}
				// If no default object, check the pin type's subcategory object
				else if (ClassPin->PinType.PinSubCategoryObject.IsValid())
				{
					ObjectClass = Cast<UClass>(ClassPin->PinType.PinSubCategoryObject.Get());
				}
			}

			if (ObjectClass)
			{
				// Store the found class path under the "Class" key
				OutNodeModel->RawProperties.Add(TEXT("Class"), ObjectClass->GetPathName());
			}
			else
			{
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine Class for GenericCreateObject node %s"), *OutNodeModel->Guid.Left(8));
			}
		}
	}

	
	// GenericCreateObject relies on pin types, no specific node property
	static void ExtractProps_GetClassDefaults(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_GetClassDefaults* Node = Cast<UK2Node_GetClassDefaults>(GraphNode))
		{
			UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Extractor: Processing GetClassDefaults Node %s"), *OutNodeModel->Guid.Left(8));

			// Attempt 1: Use the public helper GetInputClass()
			UClass* TargetClass = Node->GetInputClass();

			// --- ADDED DIAGNOSTIC LOG ---
			UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor: GetInputClass() returned: %s"), TargetClass ? *TargetClass->GetName() : TEXT("nullptr"));
			// --- END DIAGNOSTIC LOG ---

			if (TargetClass)
			{
				OutNodeModel->RawProperties.Add(TEXT("TargetClassPath"), TargetClass->GetPathName());
				UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor: Stored TargetClassPath '%s' from GetInputClass()"), *TargetClass->GetPathName());
			}
			else
			{
				// Attempt 2: Fallback - Manually check the class pin if GetInputClass failed
				UEdGraphPin* ClassPin = Node->FindClassPin(); // Use the node's helper to find the pin

				// --- ADDED DIAGNOSTIC LOG ---
				if (ClassPin) {
					UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor: GetInputClass() failed. Found ClassPin '%s'. DefaultObject: %s. PinSubCategoryObject: %s"),
        				*ClassPin->GetName(),
						ClassPin->DefaultObject ? *ClassPin->DefaultObject->GetName() : TEXT("nullptr"),
						ClassPin->PinType.PinSubCategoryObject.IsValid() ? *ClassPin->PinType.PinSubCategoryObject->GetName() : TEXT("Invalid/None")
					);
					if (ClassPin->DefaultObject && ClassPin->DefaultObject->IsA<UClass>()) {
						TargetClass = Cast<UClass>(ClassPin->DefaultObject);
						if(TargetClass) {
							OutNodeModel->RawProperties.Add(TEXT("TargetClassPath"), TargetClass->GetPathName());
							UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor: Stored TargetClassPath '%s' from ClassPin->DefaultObject"), *TargetClass->GetPathName());
						}
					} else if (ClassPin->PinType.PinSubCategoryObject.IsValid()) {
                        // This is less likely for GetClassDefaults' input pin, but check anyway
						TargetClass = Cast<UClass>(ClassPin->PinType.PinSubCategoryObject.Get());
                        if(TargetClass) {
							OutNodeModel->RawProperties.Add(TEXT("TargetClassPath"), TargetClass->GetPathName());
							UE_LOG(LogBlueprintNodeFactory, Log, TEXT("    Extractor: Stored TargetClassPath '%s' from ClassPin->PinType.PinSubCategoryObject"), *TargetClass->GetPathName());
                        }
                    }
				} else {
					UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("    Extractor: GetInputClass() failed AND could not find ClassPin for GetClassDefaults node %s"), *OutNodeModel->Guid.Left(8));
				}
				// --- END DIAGNOSTIC LOG ---

				// Final check if TargetClass is still null after fallbacks
				if (!TargetClass)
				{
					UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine TargetClassPath for GetClassDefaults node %s after all checks."), *OutNodeModel->Guid.Left(8));
				}
			}
		}
	}
	
	static void ExtractProps_GetSubsystem(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		// Common handler for GetSubsystem variants
		if (UK2Node_GetSubsystem* Node = Cast<UK2Node_GetSubsystem>(GraphNode))
		{
			// Get the result pin using the provided public getter
			UEdGraphPin* ResultPin = Node->GetResultPin();
			UClass* SubsystemClass = nullptr;

			if (ResultPin && ResultPin->PinType.PinSubCategoryObject.IsValid())
			{
				// The PinSubCategoryObject on the result pin should hold the specific subsystem class
				SubsystemClass = Cast<UClass>(ResultPin->PinType.PinSubCategoryObject.Get());
			}

			if (SubsystemClass)
			{
				// Store the found class path under the "CustomClass" key for consistency
				OutNodeModel->RawProperties.Add(TEXT("CustomClass"), SubsystemClass->GetPathName());
				UE_LOG(LogBlueprintNodeFactory, Verbose, TEXT("  Extractor: Extracted Subsystem Class '%s' via Result Pin for Node %s"), *SubsystemClass->GetName(), *OutNodeModel->Guid.Left(8));
			}
			else
			{
				// Log a warning if we couldn't determine the class
				UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine Subsystem Class for GetSubsystem node %s"), *OutNodeModel->Guid.Left(8));
			}
		}
	}
	// --- Visual/Structure Nodes ---
	static void ExtractProps_Composite(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
{
    if (UK2Node_Composite* Node = Cast<UK2Node_Composite>(GraphNode))
    {
        // ADDED LOGGING: At the beginning of the handler
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("ExtractProps_Composite: Entered for Composite Node '%s' (GUID: %s). NodeModel has %d pins BEFORE this handler's specific logic."),
            *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
            *GraphNode->NodeGuid.ToString().Left(8),
            OutNodeModel->Pins.Num()
        );
        for (const auto& PinPair : OutNodeModel->Pins)
        {
            if(PinPair.Value.IsValid())
            {
                UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  ExtractProps_Composite: Pre-existing Pin on Model: Name='%s', ID='%s', Direction='%s', Category='%s', TypeSig='%s'"),
                    *PinPair.Value->Name, 
                    *PinPair.Value->Id.Left(8), 
                    *PinPair.Value->Direction, 
                    *PinPair.Value->Category,
                    *PinPair.Value->GetTypeSignature() // Added TypeSignature for more detail
                );
            }
        }
        // END ADDED LOGGING

        // Access the public BoundGraph member (UEdGraph*)
        UEdGraph* BoundGraph = Node->BoundGraph;
        if (BoundGraph)
        {
            OutNodeModel->RawProperties.Add(TEXT("BoundGraphName"), BoundGraph->GetPathName());
            // ADDED LOGGING: After adding BoundGraphName
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("ExtractProps_Composite: After adding BoundGraphName for Composite Node '%s'. NodeModel still has %d pins."),
                *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
                OutNodeModel->Pins.Num()
            );
            // END ADDED LOGGING
        }
        else
        {
            UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor (ExtractProps_Composite): Failed to get BoundGraph for Composite node %s"), *OutNodeModel->Guid.Left(8));
        }

        // ADDED LOGGING: At the end of the handler
        UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("ExtractProps_Composite: Exiting for Composite Node '%s'. NodeModel now has %d pins."),
            *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
            OutNodeModel->Pins.Num()
        );
        // END ADDED LOGGING
    }
}
	
	static void ExtractProps_Timeline(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_Timeline* Node = Cast<UK2Node_Timeline>(GraphNode))
		{
			if (Node->TimelineName != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("TimelineName"), Node->TimelineName.ToString()); }
		}
	}


	static void ExtractProps_Message(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_Message* MessageNode = Cast<UK2Node_Message>(GraphNode))
		{
			if (MessageNode->FunctionReference.GetMemberName() != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("InterfaceFunctionName"), MessageNode->FunctionReference.GetMemberName().ToString()); }
		}
	}

	static void ExtractProps_MacroInstance(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_MacroInstance* MacroNode = Cast<UK2Node_MacroInstance>(GraphNode))
		{
			UEdGraph* MacroGraph = MacroNode->GetMacroGraph();
			if (MacroGraph) { OutNodeModel->RawProperties.Add(TEXT("MacroGraphReference"), MacroGraph->GetPathName()); }
			else { UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Could not get MacroGraph object directly for MacroInstance %s."), *OutNodeModel->Guid); }
		}
	}

	/* --- Variable Handlers ---
	static void ExtractProps_VariableGet(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(GraphNode))
		{
			if (VarGetNode->VariableReference.GetMemberName() != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("VariableName"), VarGetNode->VariableReference.GetMemberName().ToString()); }
		}
	}
*/

	// Enhanced ExtractProps_VariableGet function with comprehensive debugging
// Enhanced ExtractProps_VariableGet function - UE 5.5 Compatible
static void ExtractProps_VariableGet(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
{
    if (UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(GraphNode))
    {
        // Basic variable name extraction
        FString VarName;
        if (VarGetNode->VariableReference.GetMemberName().IsValid()) 
        { 
            VarName = VarGetNode->VariableReference.GetMemberName().ToString();
            OutNodeModel->RawProperties.Add(TEXT("VariableName"), VarName);
        }
    	// 🔧 NEW: Check for Target pin connections
    	UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   Checking for Target pin connections:"));
    	for (UEdGraphPin* Pin : VarGetNode->Pins)
    	{
    		if (Pin && Pin->Direction == EGPD_Input)
    		{
    			UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧     Input Pin: '%s', LinkedTo.Num=%d"), 
					*Pin->PinName.ToString(), Pin->LinkedTo.Num());
            
    			for (int32 i = 0; i < Pin->LinkedTo.Num(); ++i)
    			{
    				if (Pin->LinkedTo[i] && Pin->LinkedTo[i]->GetOwningNode())
    				{
    					UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧       Target[%d]: Node='%s', Pin='%s'"), 
							i, *Pin->LinkedTo[i]->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString(),
							*Pin->LinkedTo[i]->PinName.ToString());
    				}
    			}
    		}
    	}

    	
        // Check if we can get the Blueprint that owns this variable
        UBlueprint* OwningBlueprint = FBlueprintEditorUtils::FindBlueprintForNode(VarGetNode);
        if (OwningBlueprint)
        {
            // Try to find the variable property in the Blueprint
            FName VarNameFName = VarGetNode->VariableReference.GetMemberName();
            if (VarNameFName != NAME_None)
            {
                // Look for the variable in the Blueprint's NewVariables array
                for (int32 i = 0; i < OwningBlueprint->NewVariables.Num(); ++i)
                {
                    const FBPVariableDescription& VarDesc = OwningBlueprint->NewVariables[i];
                    if (VarDesc.VarName == VarNameFName)
                    {
                        FString PinCategory = VarDesc.VarType.PinCategory.ToString();
                        
                        // 🔧 ESSENTIAL: Check Blueprint's CDO for Data Asset default values
                        if (PinCategory == TEXT("object")) // Only check object properties
                        {
                            if (UClass* GeneratedClass = OwningBlueprint->GeneratedClass)
                            {
                                if (UObject* CDO = GeneratedClass->GetDefaultObject())
                                {
                                    if (FProperty* VarProperty = GeneratedClass->FindPropertyByName(VarNameFName))
                                    {
                                        if (FObjectProperty* ObjProp = CastField<FObjectProperty>(VarProperty))
                                        {
                                            UObject* DefaultValueObject = ObjProp->GetObjectPropertyValue_InContainer(CDO);
                                            if (DefaultValueObject)
                                            {
                                                FString CDO_DefaultObjectName = DefaultValueObject->GetName();
                                                FString CDO_DefaultObjectPath = DefaultValueObject->GetPathName();
                                                
                                                UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧 FOUND: Variable='%s' → DataAsset='%s'"), *VarName, *CDO_DefaultObjectName);
                                                
                                                // Store the essential Data Asset information
                                                OutNodeModel->RawProperties.Add(TEXT("CDO_DefaultObject"), CDO_DefaultObjectPath);
                                                OutNodeModel->RawProperties.Add(TEXT("CDO_DefaultObjectName"), CDO_DefaultObjectName);
                                                
                                                // Check if it's a Data Asset class
                                                if (ObjProp->PropertyClass && ObjProp->PropertyClass->IsChildOf(UDataAsset::StaticClass()))
                                                {
                                                    OutNodeModel->RawProperties.Add(TEXT("IsDataAssetProperty"), TEXT("true"));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break; // Exit the loop since we found our variable
                    }
                }
            }
        }
        
        // Store pin type information for reference
        for (UEdGraphPin* Pin : VarGetNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Output)
            {
                if (Pin->PinType.PinSubCategoryObject.IsValid())
                {
                    UObject* SubCategoryObj = Pin->PinType.PinSubCategoryObject.Get();
                    FString SubCategoryObjPath = SubCategoryObj->GetPathName();
                    OutNodeModel->RawProperties.Add(TEXT("PinSubCategoryObject"), SubCategoryObjPath);
                }
                break; // We only care about the first output pin
            }
        }
    }
}
	static void ExtractProps_VariableSet(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_VariableSet* VarSetNode = Cast<UK2Node_VariableSet>(GraphNode))
		{
			if (VarSetNode->VariableReference.GetMemberName() != NAME_None) { OutNodeModel->RawProperties.Add(TEXT("VariableName"), VarSetNode->VariableReference.GetMemberName().ToString()); }
		}
	}

	// --- Operator/Cast Handlers ---
	static void ExtractProps_Operator(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		// --- Combine Promotable and Commutative into one handler ---
		FString FunctionName;
		FString DisplayNameStr;
		bool bFoundName = false;

		// Check Promotable first as it's common
		if (UK2Node_PromotableOperator* OperatorNode = Cast<UK2Node_PromotableOperator>(GraphNode))
		{
			const FMemberReference& FuncRef = OperatorNode->FunctionReference;
			FName MemberName = FuncRef.GetMemberName();
			if (MemberName != NAME_None) { FunctionName = MemberName.ToString(); bFoundName = true;}
			else { DisplayNameStr = OperatorNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString(); }
		}
		// Only check Commutative if Promotable cast failed or didn't find name
		else if (UK2Node_CommutativeAssociativeBinaryOperator* CommutativeOpNode = Cast<UK2Node_CommutativeAssociativeBinaryOperator>(GraphNode))
		{
			const FMemberReference& FuncRef = CommutativeOpNode->FunctionReference;
			FName MemberName = FuncRef.GetMemberName();
			if (MemberName != NAME_None) { FunctionName = MemberName.ToString(); bFoundName = true; }
			else { DisplayNameStr = CommutativeOpNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString(); }
		}

		// Store extracted name or fallback
		if (bFoundName)
		{
			OutNodeModel->RawProperties.Add(TEXT("FunctionName"), FunctionName);
		}
		else if (!DisplayNameStr.IsEmpty()) // Fallback based on display name symbol
		{
			// Basic fallback based on common symbols
			if (DisplayNameStr.Contains(TEXT("/"))) { OutNodeModel->RawProperties.Add(TEXT("FunctionName"), TEXT("Divide_FloatFloat")); } // Default to float?
			else if (DisplayNameStr.Contains(TEXT("+"))) { OutNodeModel->RawProperties.Add(TEXT("FunctionName"), TEXT("Add_FloatFloat")); }
			else if (DisplayNameStr.Contains(TEXT("-"))) { OutNodeModel->RawProperties.Add(TEXT("FunctionName"), TEXT("Subtract_FloatFloat")); }
			else if (DisplayNameStr.Contains(TEXT("*"))) { OutNodeModel->RawProperties.Add(TEXT("FunctionName"), TEXT("Multiply_FloatFloat")); }
			// Add more fallbacks for comparisons, bools etc. if needed
			else { UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine operator FunctionName for %s based on title '%s'"), *OutNodeModel->Guid, *DisplayNameStr); }
		}
		else
		{
			UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Extractor: Failed to determine operator FunctionName (no FName or Title) for %s"), *OutNodeModel->Guid);
		}
	}

	static void ExtractProps_DynamicCast(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)
	{
		if (UK2Node_DynamicCast* CastNode = Cast<UK2Node_DynamicCast>(GraphNode))
		{
			if (CastNode->TargetType) { OutNodeModel->RawProperties.Add(TEXT("TargetType"), CastNode->TargetType->GetPathName()); }
		}
	}

 } // End namespace NodeFactoryExtractors


// --- Initialization and Dispatch Functions ---

void FBlueprintNodeFactory::InitializeExtractors()
{
	if (bExtractorsInitialized) return;

	UE_LOG(LogBlueprintNodeFactory, Log, TEXT("Initializing Property Extractors Map..."));

	// --- Register Handlers (Alphabetical by Node Type for easier maintenance) ---

	// A
	PropertyExtractors.Add(UK2Node_AddComponent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_AddComponent);
	PropertyExtractors.Add(UK2Node_AddDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DelegateBinding);
	PropertyExtractors.Add(UK2Node_ActorBoundEvent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_ActorBoundEvent);
	PropertyExtractors.Add(UK2Node_AssignDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DelegateBinding);
	// B
	PropertyExtractors.Add(UK2Node_BreakStruct::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_BreakStruct);
	// C
	PropertyExtractors.Add(UK2Node_CallArrayFunction::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_CallArrayFunction);
	PropertyExtractors.Add(UK2Node_CallDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DelegateBinding);
	PropertyExtractors.Add(UK2Node_CallFunction::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_CallFunction);
	PropertyExtractors.Add(UK2Node_CallParentFunction::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_CallParentFunction);
	PropertyExtractors.Add(UK2Node_ClearDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DelegateBinding);
	PropertyExtractors.Add(UK2Node_CommutativeAssociativeBinaryOperator::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Operator);
	PropertyExtractors.Add(UK2Node_ComponentBoundEvent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_ComponentBoundEvent);
	PropertyExtractors.Add(UK2Node_Composite::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Composite);
	PropertyExtractors.Add(UK2Node_CreateDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_CreateDelegate);
	PropertyExtractors.Add(UK2Node_CreateWidget::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_CreateWidget);
	PropertyExtractors.Add(UK2Node_CustomEvent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Event); // Handled by base Event
	// D
	PropertyExtractors.Add(UK2Node_DynamicCast::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DynamicCast);
	// E
	PropertyExtractors.Add(UK2Node_EnhancedInputAction::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_EnhancedInputAction);
	PropertyExtractors.Add(UK2Node_Event::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Event); // Base event handler
	// G
	PropertyExtractors.Add(UK2Node_GetClassDefaults::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_GetClassDefaults);
	PropertyExtractors.Add(UK2Node_GenericCreateObject::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_GenericCreateObject);
	PropertyExtractors.Add(UK2Node_GetSubsystem::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_GetSubsystem); // Handles variants (GetEngineSubsystem, GetSubsystemFromPC inherit)
	// I
	PropertyExtractors.Add(UK2Node_InputAction::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_InputAction);
	PropertyExtractors.Add(UK2Node_InputAxisEvent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_InputAxisEvent);
	PropertyExtractors.Add(UK2Node_InputAxisKeyEvent::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_InputAxisKeyEvent);
	PropertyExtractors.Add(UK2Node_InputKey::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_InputKey); // Handles InputKey and InputDebugKey
	// UK2Node_InputTouchEvent has no specific properties, so no handler needed currently
	// M
	PropertyExtractors.Add(UK2Node_MacroInstance::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_MacroInstance);
	PropertyExtractors.Add(UK2Node_MakeStruct::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_MakeStruct);
	PropertyExtractors.Add(UK2Node_Message::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Message); // Interface messages
	// P
	PropertyExtractors.Add(UK2Node_PromotableOperator::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Operator);
	// R
	PropertyExtractors.Add(UK2Node_RemoveDelegate::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_DelegateBinding);
	// S
	PropertyExtractors.Add(UK2Node_SpawnActorFromClass::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_SpawnActorFromClass);
	PropertyExtractors.Add(UK2Node_SwitchEnum::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_SwitchEnum);
	// Add SwitchName, SwitchString, SwitchInteger if specific handlers are needed later
	// T
	PropertyExtractors.Add(UK2Node_Timeline::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_Timeline);
	// V
	PropertyExtractors.Add(UK2Node_VariableGet::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_VariableGet);
	PropertyExtractors.Add(UK2Node_VariableSet::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_VariableSet);

	// Add handlers for nodes previously missed in registration (IfThenElse, Sequence, etc.)
	// If they don't have specific properties to extract, no handler is strictly needed,
	// but registering them prevents the "No specific property handler found" log.
	// We can add empty handlers or handlers that just log for now if desired.
	// Example (Empty Handler - can be added to NodeFactoryExtractors namespace):
	// static void ExtractProps_NoOp(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel) { }
	// PropertyExtractors.Add(UK2Node_IfThenElse::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_NoOp);
	// PropertyExtractors.Add(UK2Node_ExecutionSequence::StaticClass()->GetFName(), &NodeFactoryExtractors::ExtractProps_NoOp);
	// ... etc. for other nodes without specific properties like Knot, Select, MakeArray, FormatText ...

	bExtractorsInitialized = true;
	UE_LOG(LogBlueprintNodeFactory, Log, TEXT("Property Extractors Map Initialized with %d handlers."), PropertyExtractors.Num());
}


void FBlueprintNodeFactory::EnsureExtractorsInitialized()
{
	// Simple check, could add thread safety if needed in a multi-threaded context
	if (!bExtractorsInitialized)
	{
		InitializeExtractors();
	}
}

// --- Refactored CreateNode using Handler Map ---
TSharedPtr<FBlueprintNode> FBlueprintNodeFactory::CreateNode(UEdGraphNode* GraphNode)
{
	EnsureExtractorsInitialized(); // Make sure the map is ready

	// --- Standard Checks ---
	if (!GraphNode || !GraphNode->GetClass()) {
		if(GraphNode){ UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("FBlueprintNodeFactory::CreateNode: GraphNode has NULL Class! GUID: %s"), *GraphNode->NodeGuid.ToString()); }
		else { UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("FBlueprintNodeFactory::CreateNode: Input GraphNode is NULL.")); }
		return nullptr;
	}

	FString ClassPath = GraphNode->GetClass()->GetPathName();
	FName ClassFName = GraphNode->GetClass()->GetFName(); // Use FName for map lookup
	FString NodeGuid = GraphNode->NodeGuid.ToString();
	FString NodeType = GetNodeTypeName(ClassPath);

	// --- Node Creation ---
	TSharedPtr<FBlueprintNode> Node = MakeShared<FBlueprintNode>(NodeGuid, NodeType);
	Node->OriginalEdGraphNode = GraphNode;
	
	// --- Standard Property Assignment ---
	Node->Name = GraphNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
	Node->UEClass = ClassPath;
	Node->Position = FVector2D(GraphNode->NodePosX, GraphNode->NodePosY);
	Node->NodeComment = GraphNode->NodeComment;

	// --- Pin Extraction ---
	for (UEdGraphPin* GraphPin : GraphNode->Pins)
	{
		if (!GraphPin) continue;

		FString PinId = GraphPin->PinId.ToString();
		if (PinId.IsEmpty()) {
			UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Factory: Node '%s' (Class: %s, GUID: %s) has a pin with an empty PinId. Skipping this pin's data extraction."),
				*GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
				*GraphNode->GetClass()->GetName(),
				*NodeGuid);
			continue;
		}

		TSharedPtr<FBlueprintPin> Pin = MakeShared<FBlueprintPin>(PinId, NodeGuid);

		Pin->Name = GraphPin->PinName.ToString();
		FText FriendlyNameText = GraphPin->PinFriendlyName;
		Pin->FriendlyName = FriendlyNameText.IsEmpty() ? FString() : FriendlyNameText.ToString();
		Pin->Direction = (GraphPin->Direction == EGPD_Input) ? TEXT("EGPD_Input") : TEXT("EGPD_Output");

		// --- DETAILED LOGGING AND TYPE EXTRACTION FOR PIN ---
		const FEdGraphPinType& PinType = GraphPin->PinType;
		FString PinCategoryStr = PinType.PinCategory.ToString();
		FString PinSubCategoryStr = PinType.PinSubCategory.ToString();
		FString PinSubCategoryObjectStr = PinType.PinSubCategoryObject.IsValid() ? PinType.PinSubCategoryObject->GetPathName() : TEXT("None");
		
		FString PinValueTypeStr = TEXT("N/A");
		if (PinType.PinValueType.TerminalCategory != NAME_None) { // Check if TerminalCategory itself is valid first
			if (PinType.PinValueType.TerminalCategory == NAME_Struct || PinType.PinValueType.TerminalCategory == NAME_Enum ) {
				if (PinType.PinValueType.TerminalSubCategoryObject.IsValid()) {
					PinValueTypeStr = PinType.PinValueType.TerminalSubCategoryObject->GetPathName();
				} else {
					PinValueTypeStr = FString::Printf(TEXT("%s (TerminalSubCategoryObject invalid)"), *PinType.PinValueType.TerminalCategory.ToString());
				}
			} else { // For simple terminal categories like int, bool, byte etc.
				PinValueTypeStr = PinType.PinValueType.TerminalCategory.ToString();
			}

		}

		const UEnum* ContainerEnum = StaticEnum<EPinContainerType>();
		FString ContainerTypeStr = ContainerEnum ? ContainerEnum->GetNameStringByValue(static_cast<int64>(PinType.ContainerType)) : TEXT("UnknownContainer");
		/*
		UE_LOG(LogBlueprintNodeFactory, Warning, 
			TEXT("Factory PIN DETAIL: Node '%s' (Class: %s, GUID: %s), Pin '%s' (ID: %s), Direction: %s --- Category: '%s', SubCategory: '%s', SubCatObj: '%s', ValueType(Terminal): '%s', ContainerType: '%s', bIsArray: %d, bIsSet: %d, bIsMap: %d"),
			*GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
            *GraphNode->GetClass()->GetName(),
			*NodeGuid,
			*GraphPin->PinName.ToString(),
			*PinId,
			(GraphPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output")),
			*PinCategoryStr,
			*PinSubCategoryStr,
			*PinSubCategoryObjectStr,
            *PinValueTypeStr, 
			*ContainerTypeStr,
			PinType.IsArray(),
			PinType.IsSet(),
			PinType.IsMap()
		);*/
		// --- END DETAILED LOGGING AND TYPE EXTRACTION ---

		Pin->Category = PinCategoryStr; 
		Pin->SubCategory = PinSubCategoryStr;
		if (PinType.PinSubCategoryObject.IsValid()) { 
			Pin->SubCategoryObject = PinSubCategoryObjectStr; 
		} else { 
			Pin->SubCategoryObject = TEXT(""); // Explicitly clear if not valid
		}

		Pin->bIsReference = PinType.bIsReference;
		Pin->bIsConst = PinType.bIsConst;
		Pin->ContainerType = ContainerTypeStr; 
		
		// Populate map value type information if this is a map pin
		if (Pin->ContainerType == TEXT("Map"))
		{
			// From FEdGraphPinType::PinValueType (which is FEdGraphTerminalType)
			// This holds the type for Array/Set elements, OR Map Values.
			// PinCategory/PinSubCategoryObject on the main pin hold Map Key info.
			Pin->MapValueTerminalCategory = PinType.PinValueType.TerminalCategory.ToString();
			if (PinType.PinValueType.TerminalSubCategoryObject.IsValid())
			{
				Pin->MapValueTerminalSubCategoryObjectPath = PinType.PinValueType.TerminalSubCategoryObject->GetPathName();
			}
			else
			{
				Pin->MapValueTerminalSubCategoryObjectPath = TEXT(""); // Ensure it's empty if not valid
			}
			// Log this new info for verification during factory processing
			UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  Factory MAP VALUE TYPE for Pin '%s' on Node '%s': MapValueTermCategory='%s', MapValueTermSubCatObjPath='%s' (Derived from PinType.PinValueType)"),
				*Pin->Name, *Node->Name, *Pin->MapValueTerminalCategory, *Pin->MapValueTerminalSubCategoryObjectPath);
		}

		if (GraphNode->IsA(UK2Node_Composite::StaticClass())) // Check if the parent GraphNode is a Composite
		{
			UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("CreateNode (Composite Pin LOG): Node '%s' (GUID:%s, Class:%s), PinName='%s', PinFriendlyName='%s', PinID='%s', Direction='%s', Category='%s', SubCategory='%s', SubCatObjPath='%s', ContainerType='%s'"),
				*GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
				*GraphNode->NodeGuid.ToString().Left(8),
				*GraphNode->GetClass()->GetName(),
				*GraphPin->PinName.ToString(),
				*GraphPin->PinFriendlyName.ToString(),
				*GraphPin->PinId.ToString().Left(8),
				(GraphPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output")),
				*GraphPin->PinType.PinCategory.ToString(),
				*GraphPin->PinType.PinSubCategory.ToString(),
				PinType.PinSubCategoryObject.IsValid() ? *PinType.PinSubCategoryObject->GetPathName() : TEXT("None"),
				*ContainerTypeStr
			);
		}

		
		
		if (GraphPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Text)
		{
			// FText literals are stored in DefaultTextValue, not the FString DefaultValue.
			// FText::ToString() reliably gives us the displayable source string.
			Pin->DefaultValue = GraphPin->DefaultTextValue.ToString();
		}
		else
		{
			// For all other types, the existing logic of using the FString property is correct.
			Pin->DefaultValue = GraphPin->DefaultValue;

			if (Pin->Name == TEXT("MyStringTestStructVar")) {
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧 COMPREHENSIVE DEBUG: Found target pin '%s'"), *Pin->Name);
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   Category='%s', DefaultValue='%s'"), *PinCategoryStr, *Pin->DefaultValue);
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   GraphPin->DefaultValue='%s'"), *GraphPin->DefaultValue);
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   SubCategoryObject='%s'"), *PinSubCategoryObjectStr);
    
    // 🔧 NEW: Check ALL GraphPin properties that might contain struct defaults
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   GraphPin->DefaultTextValue='%s'"), *GraphPin->DefaultTextValue.ToString());
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   GraphPin->AutogeneratedDefaultValue='%s'"), *GraphPin->AutogeneratedDefaultValue);
    
    // Check if there's a DefaultObject
    if (GraphPin->DefaultObject) {
        UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   GraphPin->DefaultObject='%s'"), *GraphPin->DefaultObject->GetPathName());
        UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   DefaultObject->GetName()='%s'"), *GraphPin->DefaultObject->GetName());
    } else {
        UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   GraphPin->DefaultObject=NULL"));
    }
    
    // 🔧 Use the existing PinType variable (don't redeclare it)
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   PinType.PinCategory='%s'"), *PinType.PinCategory.ToString());
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   PinType.PinSubCategory='%s'"), *PinType.PinSubCategory.ToString());
    
    if (PinType.PinSubCategoryObject.IsValid()) {
        UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   PinType.PinSubCategoryObject='%s'"), *PinType.PinSubCategoryObject->GetPathName());
        
        // Try to get the struct itself
        if (UScriptStruct* StructType = Cast<UScriptStruct>(PinType.PinSubCategoryObject.Get())) {
            UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   Found UScriptStruct: '%s'"), *StructType->GetName());
            
            // 🔧 Try alternative ways to get struct default values
            UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   StructType->GetStructureSize()=%d"), StructType->GetStructureSize());
            
            // Check if the schema has any useful methods
            if (const UEdGraphSchema* Schema = GraphPin->GetSchema()) {
                UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   Schema class: '%s'"), *Schema->GetClass()->GetName());
                // Note: Skipping schema method calls for now due to API differences
            }
        }
    }
    
    // 🔧 Check if the owning node has any struct-related properties
    if (UEdGraphNode* OwningNode = GraphPin->GetOwningNode()) {
        UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   OwningNode='%s', Class='%s'"), 
            *OwningNode->GetName(), *OwningNode->GetClass()->GetName());
            
        // For VariableSet nodes, check if there are any additional properties
        if (UK2Node_VariableSet* VarSetNode = Cast<UK2Node_VariableSet>(OwningNode)) {
            UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   VariableSet node - checking for more properties..."));
            
            // Check the variable reference
            const FMemberReference& VarRef = VarSetNode->VariableReference;
            UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧   VarRef.MemberName='%s'"), *VarRef.GetMemberName().ToString());
        }
    }
    
    UE_LOG(LogBlueprintNodeFactory, Error, TEXT("🔧 END COMPREHENSIVE DEBUG for pin '%s'"), *Pin->Name);
}
		}
		// --- END NEW ---

		
		if (GraphPin->DefaultObject) { 
			Pin->DefaultObject = GraphPin->DefaultObject->GetPathName(); 
		}

if (PinCategoryStr == TEXT("struct") && Pin->DefaultValue.IsEmpty()) {
    // Get the owning node
    UEdGraphNode* OwningNode = GraphPin->GetOwningNode();
    if (OwningNode) {
        // Check if this is a VariableSet or VariableGet node
        UK2Node_VariableSet* VarSetNode = Cast<UK2Node_VariableSet>(OwningNode);
        UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(OwningNode);
        
        FName VariableName = NAME_None;
        if (VarSetNode) {
            VariableName = VarSetNode->VariableReference.GetMemberName();
        } else if (VarGetNode) {
            VariableName = VarGetNode->VariableReference.GetMemberName();
        }
        
        if (VariableName != NAME_None) {
            // Look for expanded struct member pins on the same node
            FString VariableNameStr = VariableName.ToString();
            TMap<FString, FString> CollectedDefaults;
            
            // Iterate through all pins on this node to find expanded struct members
            for (UEdGraphPin* NodePin : OwningNode->Pins) {
                if (NodePin && NodePin->Direction == EGPD_Input) {
                    FString NodePinName = NodePin->PinName.ToString();
                    
                    // Check if this pin is an expanded member of our struct variable
                    if (NodePinName.StartsWith(VariableNameStr + TEXT("_")) && !NodePin->DefaultValue.IsEmpty()) {
                        // Extract the member name from the pin name
                        // Format: "VariableName_MemberName_Index_GUID"
                        FString MemberName = NodePinName;
                        MemberName.RemoveFromStart(VariableNameStr + TEXT("_"));
                        
                        // Remove the index and GUID suffix (everything after the second underscore)
                        int32 FirstUnderscorePos = -1;
                        int32 SecondUnderscorePos = -1;
                        for (int32 i = 0; i < MemberName.Len(); ++i) {
                            if (MemberName[i] == TEXT('_')) {
                                if (FirstUnderscorePos == -1) {
                                    FirstUnderscorePos = i;
                                } else {
                                    SecondUnderscorePos = i;
                                    break;
                                }
                            }
                        }
                        
                        if (SecondUnderscorePos > 0) {
                            MemberName = MemberName.Left(SecondUnderscorePos);
                        }
                        
                        // Store the default value for this member
                        CollectedDefaults.Add(MemberName, NodePin->DefaultValue);
                    }
                }
            }
            
            // If we found expanded pin defaults, reconstruct the struct default
            if (CollectedDefaults.Num() > 0) {
                // Reconstruct the struct default value string
                TArray<FString> MemberPairs;
                for (const auto& Pair : CollectedDefaults) {
                    FString Value = Pair.Value.TrimStartAndEnd();
                    // Add quotes if it's a string value without quotes
                    if (!Value.StartsWith(TEXT("\"")) && !Value.EndsWith(TEXT("\""))) {
                        // Check if it's a numeric value
                        bool bIsNumeric = Value.IsNumeric();
                        if (!bIsNumeric) {
                            Value = FString::Printf(TEXT("\"%s\""), *Value);
                        }
                    }
                    MemberPairs.Add(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Value));
                }
                
                FString ReconstructedDefault = FString::Printf(TEXT("(%s)"), *FString::Join(MemberPairs, TEXT(",")));
                
                // Store the reconstructed default value in the pin
                Pin->RawProperties.Add(TEXT("BlueprintVariableDefault"), ReconstructedDefault);
                
                // Also populate DefaultStruct map
                for (const auto& Pair : CollectedDefaults) {
                    FString CleanValue = Pair.Value.TrimStartAndEnd().TrimQuotes();
                    Pin->DefaultStruct.Add(Pair.Key, CleanValue);
                }
                
                // Optional: Add a debug log for successful extraction (generic)
                UE_LOG(LogBlueprintNodeFactory, Log, TEXT("Extracted %d struct member defaults for variable '%s'"), 
                    Pin->DefaultStruct.Num(), *VariableNameStr);
            }
        }
    }
}
		
		Pin->RawProperties.Add(TEXT("bHidden"), GraphPin->bHidden ? TEXT("true") : TEXT("false"));
		Pin->RawProperties.Add(TEXT("bAdvancedView"), GraphPin->bAdvancedView ? TEXT("true") : TEXT("false"));
		Pin->RawProperties.Add(TEXT("AutogeneratedDefaultValue"), GraphPin->AutogeneratedDefaultValue);

		// --- Store Link Info directly here ---
		int32 LinkIndex = 0;
		for (UEdGraphPin* LinkedPin : GraphPin->LinkedTo) {
		   if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;
			
		   FString LinkedPinId = LinkedPin->PinId.ToString();
		   FString LinkedNodeGuid = LinkedPin->GetOwningNode()->NodeGuid.ToString();
		   FName LinkNodeKey = FName(*FString::Printf(TEXT("LinkedTo_%d_NodeGuid"), LinkIndex));
		   FName LinkPinKey = FName(*FString::Printf(TEXT("LinkedTo_%d_PinID"), LinkIndex));
		   Pin->RawProperties.Add(LinkNodeKey.ToString(), LinkedNodeGuid);
		   Pin->RawProperties.Add(LinkPinKey.ToString(), LinkedPinId);
		   LinkIndex++;
		}
		// --- End Link Info Storage ---

		Node->Pins.Add(PinId, Pin);
	}

	// --- Pin Link Info Storage ---
	 for (UEdGraphPin* GraphPin : GraphNode->Pins)
	 {
		if (!GraphPin) continue;
		FString PinId = GraphPin->PinId.ToString();
		TSharedPtr<FBlueprintPin> Pin = Node->Pins.FindRef(PinId);
		if (Pin.IsValid()) {
			int32 LinkIndex = 0;
			for (UEdGraphPin* LinkedPin : GraphPin->LinkedTo) {
			   if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;
				 if (GraphNode && GraphPin && 
                (GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetDataTableRowNames") || 
                 GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetArrayItem") ||
                 GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetDataTableRow") /* Also log if GetDataTableRow itself has output links */
                 ))
            {
                // Only log if the Source Pin (GraphPin) is an OUTPUT pin and NOT an EXEC pin,
                // as we are interested in how DATA links are being established.
                 if (GraphNode && GraphPin && 
                (GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetDataTableRowNames") || 
                 GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetArrayItem") ||
                 GraphNode->GetClass()->GetFName() == TEXT("K2Node_GetDataTableRow") 
                 ))
            {
                // Only log if the Source Pin (GraphPin) is an OUTPUT pin and NOT an EXEC pin,
                // as we are interested in how DATA links are being established.
                if (GraphPin->Direction == EGPD_Output && GraphPin->PinType.PinCategory != TEXT("exec"))
                {
                    UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("Factory UEdGraphPin::LinkedTo Inspection: Node '%s' (GUID:%s, Class:%s)"),
                        *GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
                        *GraphNode->NodeGuid.ToString().Left(8),
                        *GraphNode->GetClass()->GetName());
                    UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  Processing Source Output Pin on this Node: '%s' (ID:%s, Category:%s)"),
                        *GraphPin->PinName.ToString(),
                        *GraphPin->PinId.ToString().Left(8),
                        *GraphPin->PinType.PinCategory.ToString());
                    UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("    This Source Pin is LinkedTo Target: NodeName='%s' (GUID:%s, Class:%s), TargetPinName='%s' (ID:%s, Direction:%s, Category:%s)"),
                        *LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString(), // Uses LinkedPin
                        *LinkedPin->GetOwningNode()->NodeGuid.ToString().Left(8),                      // Uses LinkedPin
                        *LinkedPin->GetOwningNode()->GetClass()->GetName(),                             // Uses LinkedPin
                        *LinkedPin->PinName.ToString(),                                                 // Uses LinkedPin
                        *LinkedPin->PinId.ToString().Left(8),                                           // Uses LinkedPin
                        (LinkedPin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output")),          // Uses LinkedPin
                        *LinkedPin->PinType.PinCategory.ToString());                                    // Uses LinkedPin
                }
            }
            }

				
			   FString LinkedPinId = LinkedPin->PinId.ToString();
			   FString LinkedNodeGuid = LinkedPin->GetOwningNode()->NodeGuid.ToString();
			   FName LinkNodeKey = FName(*FString::Printf(TEXT("LinkedTo_%d_NodeGuid"), LinkIndex));
			   FName LinkPinKey = FName(*FString::Printf(TEXT("LinkedTo_%d_PinID"), LinkIndex));
			   Pin->RawProperties.Add(LinkNodeKey.ToString(), LinkedNodeGuid);
			   Pin->RawProperties.Add(LinkPinKey.ToString(), LinkedPinId);
			   LinkIndex++;
			}
		}
	}

	// --- Dispatch to Specific Property Extractor ---
	
	UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("FactoryDispatchDebug: Attempting to dispatch for Node Type (FName): '%s', Node Title: '%s', Node GUID: %s"), 
		*ClassFName.ToString(), 
		*GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
		*GraphNode->NodeGuid.ToString());

	const FNodePropertyExtractorFunc* Handler = PropertyExtractors.Find(ClassFName);
	if (Handler && *Handler != nullptr) // Also check if the TFunction itself is bound
	{
		UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDispatchDebug: Found BOUND handler for FName: '%s'. Calling handler now."), *ClassFName.ToString());
		(*Handler)(GraphNode, Node); 
	}
	else if (Handler && *Handler == nullptr)
	{
		UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDispatchDebug: Found handler for FName: '%s', BUT THE TFunction IS UNBOUND/NULL. Handler will NOT be called."), *ClassFName.ToString());
	}
	else
	{
		UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDispatchDebug: NO handler found in PropertyExtractors map for FName: '%s'."), *ClassFName.ToString());
		// Optional: Log if a non-generic K2Node doesn't have a handler (already existing logic)
		if (GraphNode->IsA(UK2Node::StaticClass())) 
		{
			UE_LOG(LogBlueprintNodeFactory, Warning, TEXT("  FactoryDispatchDebug: (Further Info) Node Type: %s (%s) is a K2Node but has no specific handler beyond generic property extraction."), *NodeType, *ClassPath);
		}
	}
	// --- End Dispatch ---

	// --- Final Logging ---
	if (Node.IsValid()) {
	   UE_LOG(LogBlueprintNodeFactory, Log, TEXT("Factory EXIT Node %s (%s)"), *NodeGuid, *NodeType);
		UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  PRESERVED Props: Comp='%s', Del='%s', OwnerPathWorkaround='%s'"), // Log the correct member
			   *Node->PreservedCompPropName, *Node->PreservedDelPropName, *Node->BoundEventOwnerClassPath);
	   UE_LOG(LogBlueprintNodeFactory, Log, TEXT("  FINAL RawProperties Count: %d"), Node->RawProperties.Num());
	}

	return Node;
}
// --- End of refactored CreateNode ---


// --- CreateNodeFromClassPath and GetNodeTypeName (Remain Unchanged) ---
TSharedPtr<FBlueprintNode> FBlueprintNodeFactory::CreateNodeFromClassPath(const FString& Guid, const FString& ClassPath)
{
	FString NodeType = GetNodeTypeName(ClassPath);
	UE_LOG(LogBlueprintNodeFactory, Verbose, TEXT("FBlueprintNodeFactory::CreateNodeFromClassPath: Guid: %s, ClassPath: %s -> NodeType: %s"), *Guid, *ClassPath, *NodeType);
	return MakeShared<FBlueprintNode>(Guid, NodeType);
}

FString FBlueprintNodeFactory::GetNodeTypeName(const FString& ClassPath)
{
	FString FullObjectName = FPaths::GetCleanFilename(ClassPath);
	FString TypePart = FullObjectName;
	int32 LastDotIndex = -1;
	if (FullObjectName.FindLastChar(TEXT('.'), LastDotIndex)) { TypePart = FullObjectName.Mid(LastDotIndex + 1); }
	if (TypePart.StartsWith(TEXT("K2Node_"))) { return TypePart.RightChop(7); }
	else if (TypePart.StartsWith(TEXT("EdGraphNode_"))) { return TypePart.RightChop(12); }
	else if (TypePart.StartsWith(TEXT("AnimGraphNode_"))) { return TypePart.RightChop(14); }
	return TypePart;
}
// --- End of file ---