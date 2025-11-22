/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/BlueprintAnalysisUtils.h

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class UBlueprint;
class UFunction;


class BP2AI_API FBlueprintAnalysisUtils 
{
public:
    /**
     * Analyzes selected Blueprint assets and logs information about their type (Interface/Regular)
     * and their functions (Interface definitions/implementations vs regular functions).
     * Output is sent to the UE Output Log.
     * @param SelectedAssets Array of UObject pointers, expected to be UBlueprint assets.
     */
    static void LogBlueprintFunctionAnalysis(const TArray<UObject*>& SelectedAssets);

private:
    /** Helper to check if a UFunction in a UBlueprint is an implementation of an interface function. */
    static bool IsInterfaceImplementation(const UBlueprint* Blueprint, const UFunction* Function, FString& OutImplementedInterfaceName);
};