/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/MarkdownGenerationContext.cpp

// Private/Trace/MarkdownGenerationContext.cpp
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/Utils/MarkdownSpanSystem.h" // Changed from MarkdownFormattingUtils.h to MarkdownSpanSystem.h

// FMarkdownContextManager implementation
FMarkdownContextManager::FMarkdownContextManager(const FMarkdownGenerationContext& Context)
{
	// Call the context management functions now in FMarkdownSpanSystem
	FMarkdownSpanSystem::SetGlobalContext(&Context);
}

FMarkdownContextManager::~FMarkdownContextManager()
{
	// Call the context management functions now in FMarkdownSpanSystem
	FMarkdownSpanSystem::ClearGlobalContext();
}