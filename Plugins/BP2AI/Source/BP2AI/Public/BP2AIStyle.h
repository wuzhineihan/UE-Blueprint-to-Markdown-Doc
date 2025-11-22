/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/BP2AIStyle.h


#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * Style set for Blueprint Markdown UI components
 */
class FBP2AIStyle
{
public:
	static void Initialize();
	static void Shutdown();
    
	/** Reload textures used by the slate renderer */
	static void ReloadTextures();

	/** Get the style set */
	static const ISlateStyle& Get();

	/** Get the style set name */
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};