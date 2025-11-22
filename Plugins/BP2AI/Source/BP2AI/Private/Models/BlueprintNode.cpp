/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Models/BlueprintNode.cpp


#include "Models/BlueprintNode.h"
#include "Logging/BP2AILog.h"

FBlueprintNode::FBlueprintNode(const FString& InGuid, const FString& InNodeType)
    : Guid(InGuid)
    , NodeType(InNodeType)
    , Position(FVector2D::ZeroVector)
    , PreservedCompPropName(TEXT(""))
    , PreservedDelPropName(TEXT(""))
    , BoundEventOwnerClassPath(TEXT("InitialTestOwnerPath")) // <-- ADDED INITIALIZER
{
    // Constructor body can remain empty
}
void FBlueprintNode::BreakReferenceCycles()
{
    // Break circular references between pins
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value.IsValid())
        {
            PinPair.Value->LinkedPins.Empty();
            PinPair.Value->SourcePinFor.Empty();
        }
    }
}
void FBlueprintNode::PreserveCriticalProperties()
{
	// This function was intended as a workaround before the Factory Hex fix.
	// With the Hex fix and the BoundEventOwnerClassPath member populated directly
	// in the Factory, this function might no longer be strictly necessary,
	// as the Factory handlers now store the needed data reliably.
	// However, keeping it allows for potential future use or if RawProperties
	// are needed for other reasons before ResolveLinks.

	// Let's keep the logic for CompPropName and DelPropName as they might be
	// useful if read from RawProperties elsewhere, but remove the part
	// related to the deleted PreservedDelOwnerClassPath.

	if (NodeType == TEXT("ComponentBoundEvent") || NodeType == TEXT("ActorBoundEvent"))
	{
		UE_LOG(LogModels, Log, TEXT("PreserveCriticalProperties: Checking RawProperties for Node %s (%s) (OwnerPath preservation removed)"), *Guid.Left(8), *NodeType);

		// Find and copy values from the RawProperties map (if they were stored there)
		const FString* FoundCompProp = RawProperties.Find(TEXT("ComponentPropertyName"));
		if (FoundCompProp)
		{
			// We already populate PreservedCompPropName via Hex in Factory,
			// but this could act as a verification or secondary source if needed.
			// For now, just log if found in RawProperties.
			// PreservedCompPropName = *FoundCompProp; // Avoid overwriting Hex-fixed value
			UE_LOG(LogModels, Log, TEXT("  -> Found CompPropName in RawProperties: '%s'"), **FoundCompProp);
		} else {
			UE_LOG(LogModels, Verbose, TEXT("  -> CompPropName not found in RawProperties during PreserveCriticalProperties check."));
		}

		const FString* FoundDelProp = RawProperties.Find(TEXT("DelegatePropertyName"));
		if (FoundDelProp)
		{
			// Similar to CompPropName, avoid overwriting Hex-fixed value.
			// PreservedDelPropName = *FoundDelProp;
			UE_LOG(LogModels, Log, TEXT("  -> Found DelPropName in RawProperties: '%s'"), **FoundDelProp);
		} else {
			UE_LOG(LogModels, Verbose, TEXT("  -> DelPropName not found in RawProperties during PreserveCriticalProperties check."));
		}

		// Removed logic related to PreservedDelOwnerClassPath as the member is removed.
		// The reliable value is now stored in BoundEventOwnerClassPath directly by the Factory.
	}
}


TSharedPtr<FBlueprintPin> FBlueprintNode::GetExecutionOutputPin(const FString& PinName) const
{
    // Try specific name first
    TSharedPtr<FBlueprintPin> Pin = GetPin(PinName, TEXT("EGPD_Output"));
    if (Pin.IsValid() && Pin->IsExecution())
    {
        return Pin;
    }
    
    // Try common names
    TArray<FString> CommonNames = { 
        TEXT("then"), TEXT("Trigger"), TEXT("Completed"), TEXT("LoopBody"), 
        TEXT("Exit"), TEXT("Success"), TEXT("A"), TEXT("Pressed"), 
        TEXT("Released"), TEXT("Update") 
    };
    
    for (const FString& CommonName : CommonNames)
    {
        Pin = GetPin(CommonName, TEXT("EGPD_Output"));
        if (Pin.IsValid() && Pin->IsExecution())
        {
            return Pin;
        }
    }
    
    // Fallback: find first execution output pin
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->IsOutput() && PinPair.Value->IsExecution() && !PinPair.Value->IsHidden())
        {
            return PinPair.Value;
        }
    }
    
    return nullptr;
}

TSharedPtr<FBlueprintPin> FBlueprintNode::GetExecutionInputPin() const
{
    // Try common names
    TArray<FString> CommonNames = { 
        TEXT("execute"), TEXT("exec"), TEXT("in"), TEXT("run"), 
        TEXT("TryGet"), TEXT("Enter"), TEXT("Play"), TEXT("PlayFromStart"), 
        TEXT("Cast"), TEXT("Bind"), TEXT("Add"), TEXT("Assign"), TEXT("Set")
    };
    
    for (const FString& CommonName : CommonNames)
    {
        TSharedPtr<FBlueprintPin> Pin = GetPin(CommonName, TEXT("EGPD_Input"));
        if (Pin.IsValid() && Pin->IsExecution())
        {
            return Pin;
        }
    }
    
    // Fallback: find first execution input pin
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->IsInput() && PinPair.Value->IsExecution())
        {
            return PinPair.Value;
        }
    }
    
    return nullptr;
}

bool FBlueprintNode::IsPure() const
{
    // Check for execution pins
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->IsExecution())
        {
            return false;
        }
    }
    
    return true;
}

TSharedPtr<FBlueprintPin> FBlueprintNode::GetPin(const FString& InPinName) const
{
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->Name == InPinName)
        {
            return PinPair.Value;
        }
    }
    
    // Try case insensitive match
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->Name.ToLower() == InPinName.ToLower())
        {
            return PinPair.Value;
        }
    }
    
    return nullptr;
}

TSharedPtr<FBlueprintPin> FBlueprintNode::GetPin(const FString& InPinName, const FString& Direction) const
{
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->Name == InPinName && PinPair.Value->Direction == Direction)
        {
            return PinPair.Value;
        }
    }
    
    // Try case insensitive match
    for (auto& PinPair : Pins)
    {
        if (PinPair.Value->Name.ToLower() == InPinName.ToLower() && PinPair.Value->Direction == Direction)
        {
            return PinPair.Value;
        }
    }
    
    return nullptr;
}

TArray<TSharedPtr<FBlueprintPin>> FBlueprintNode::GetOutputPins(const FString& Category, bool bIncludeHidden) const
{
    TArray<TSharedPtr<FBlueprintPin>> Result;
    
    for (auto& PinPair : Pins)
    {
        auto& Pin = PinPair.Value;
        if (Pin->IsOutput() && (bIncludeHidden || (!Pin->IsHidden() && !Pin->IsAdvancedView())))
        {
            if (Category.IsEmpty() || Pin->Category == Category)
            {
                Result.Add(Pin);
            }
        }
    }
    
    // Sort by name
    Result.Sort([](const TSharedPtr<FBlueprintPin>& A, const TSharedPtr<FBlueprintPin>& B) {
        return A->Name < B->Name;
    });
    
    return Result;
}

TArray<TSharedPtr<FBlueprintPin>> FBlueprintNode::GetInputPins(const FString& Category, bool bIncludeHidden, bool bExcludeExec) const
{
    TArray<TSharedPtr<FBlueprintPin>> Result;
    
    for (auto& PinPair : Pins)
    {
        auto& Pin = PinPair.Value;
        if (Pin->IsInput() && (bIncludeHidden || (!Pin->IsHidden() && !Pin->IsAdvancedView())))
        {
            if (bExcludeExec && Pin->IsExecution())
            {
                continue;
            }
            
            if (Category.IsEmpty() || Pin->Category == Category)
            {
                Result.Add(Pin);
            }
        }
    }
    
    // Sort primarily by non-advanced, then by name
    Result.Sort([](const TSharedPtr<FBlueprintPin>& A, const TSharedPtr<FBlueprintPin>& B) {
        if (A->IsAdvancedView() != B->IsAdvancedView())
        {
            return !A->IsAdvancedView(); // Non-advanced first
        }
        return A->Name < B->Name;
    });
    
    return Result;
}