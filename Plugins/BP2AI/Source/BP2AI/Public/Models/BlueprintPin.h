/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/Models/BlueprintPin.h


#pragma once

#include "CoreMinimal.h"


/**
 * Represents a pin on a Blueprint node
 */
class BP2AI_API FBlueprintPin : public TSharedFromThis<FBlueprintPin>
{
public:
	FBlueprintPin(const FString& InId, const FString& InNodeGuid);
    
	/** Core properties */
	FString Id;
	FString PinId;  
	FString NodeGuid;
	FString Name;
	FName PinName; 
	FString FriendlyName;
	FString Direction; // "EGPD_Output" or "EGPD_Input"
	FString Category;  // "exec", "bool", "int", etc.
	FString SubCategory;
	FString SubCategoryObject;
	bool bIsReference = false;
	bool bIsConst = false;
	FString ContainerType; // None, Array, Set, Map
	FString DefaultValue;
	FString DefaultObject;
	TMap<FString, FString> DefaultStruct;
	TMap<FString, FString> RawProperties;
	// Solving TerminalCategory and TerminalSubCategoryObject 
	FString MapValueTerminalCategory;
	FString MapValueTerminalSubCategoryObjectPath;
	
	/** Linked pins - resolved during post-processing */
	TArray<TSharedPtr<FBlueprintPin>> LinkedPins;
	TArray<TSharedPtr<FBlueprintPin>> SourcePinFor;
    
	/** Helper methods */
	bool IsOutput() const;
	bool IsInput() const;
	bool IsExecution() const;
	bool IsHidden() const;
	bool IsAdvancedView() const;
    
	/** Get a formatted type signature string */
	FString GetTypeSignature() const;
};