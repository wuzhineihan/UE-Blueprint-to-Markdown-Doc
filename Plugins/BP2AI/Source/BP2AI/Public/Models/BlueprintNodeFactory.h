/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/Models/BlueprintNodeFactory.h


#pragma once

#include "CoreMinimal.h"
#include "Models/BlueprintNode.h" // Include base node model
#include "Templates/Function.h"    // Include for TFunction
#include "Containers/Map.h"        // Include for TMap
#include "UObject/NameTypes.h"     // Include for FName

class UEdGraphNode;

/**
 * Factory for creating Blueprint node model objects using a handler map.
 */
class BP2AI_API FBlueprintNodeFactory
{
public:
	/** Handler function signature: Takes UEdGraphNode and the FBlueprintNode model to populate */
	using FNodePropertyExtractorFunc = TFunction<void(UEdGraphNode* GraphNode, TSharedPtr<FBlueprintNode> OutNodeModel)>;

	/**
	 * Create a node model from a UEdGraphNode.
	 * Extracts basic properties, pins, links, and calls specific handlers for type-specific properties.
	 * @param GraphNode The source UEdGraphNode.
	 * @return A TSharedPtr to the created FBlueprintNode model, or nullptr on failure.
	 */
	static TSharedPtr<FBlueprintNode> CreateNode(UEdGraphNode* GraphNode);

	/**
	 * Create a node model from a class path string (for testing or serialization).
	 * NOTE: This creates a basic node; it does not extract detailed properties.
	 * @param Guid The GUID for the node.
	 * @param ClassPath The UE class path string.
	 * @return A TSharedPtr to the created FBlueprintNode model.
	 */
	static TSharedPtr<FBlueprintNode> CreateNodeFromClassPath(const FString& Guid, const FString& ClassPath);

	/** Ensures the property extractor handlers map is initialized */
	static void EnsureExtractorsInitialized();

private:
	/**
	 * Get a readable node type name from a class path string.
	 * @param ClassPath The full UE class path.
	 * @return A simplified node type string (e.g., "CallFunction").
	 */
	static FString GetNodeTypeName(const FString& ClassPath);

	/** Initializes the handler map (called once internally). */
	static void InitializeExtractors();

	/** Map of UClass FNames to their specific property extraction handlers */
	static TMap<FName, FNodePropertyExtractorFunc> PropertyExtractors;
	/** Flag to ensure one-time initialization of the handler map */
	static bool bExtractorsInitialized;
};