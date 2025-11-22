/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/Models/BlueprintNode.h
#pragma once

#include "CoreMinimal.h"
#include "Models/BlueprintPin.h"
#include "UObject/WeakObjectPtr.h" // Added include for TWeakObjectPtr
#include "EdGraph/EdGraphNode.h"   // Added include for UEdGraphNode

/**
 * Base class for Blueprint node models
 */
class BP2AI_API FBlueprintNode : public TSharedFromThis<FBlueprintNode>
{
public:
	FBlueprintNode(const FString& InGuid, const FString& InNodeType);
	virtual ~FBlueprintNode() {}

	TWeakObjectPtr<UEdGraphNode> OriginalEdGraphNode; // Pointer back to the source node

	/** Core properties */
	FString Guid;
	FString NodeType;
	FString UEClass;
	FString Name;
	FVector2D Position;
	TMap<FString, TSharedPtr<FBlueprintPin>> Pins;
	TMap<FString, FString> RawProperties;
	FString NodeComment;

	// Store critical bound event properties separately to prevent loss during ResolveLinks
	FString PreservedCompPropName;
	FString PreservedDelPropName;
	FString TestStringMember;
	FString BoundEventOwnerClassPath;

	/** Virtual methods for specialized node behavior */
	virtual TSharedPtr<FBlueprintPin> GetExecutionOutputPin(const FString& PinName = TEXT("then")) const;
	virtual TSharedPtr<FBlueprintPin> GetExecutionInputPin() const;
	virtual bool IsPure() const;

	// --- ADDED GETTER ---
	UEdGraphNode* GetEdGraphNode() const { return OriginalEdGraphNode.Get(); }
	// --- END ADDED GETTER ---

	/** Common node helper methods */
	TSharedPtr<FBlueprintPin> GetPin(const FString& InPinName) const;
	TSharedPtr<FBlueprintPin> GetPin(const FString& InPinName, const FString& Direction) const;

	/** Get all pins matching criteria */
	TArray<TSharedPtr<FBlueprintPin>> GetOutputPins(const FString& Category = TEXT(""), bool bIncludeHidden = false) const;
	TArray<TSharedPtr<FBlueprintPin>> GetInputPins(const FString& Category = TEXT(""), bool bIncludeHidden = false, bool bExcludeExec = true) const;

	/** Break reference cycles before destruction */
	void BreakReferenceCycles();
	void PreserveCriticalProperties();
};