/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/MarkdownGenerationContext.h

#pragma once

#include "CoreMinimal.h"

// Forward declaration to avoid circular includes
struct FMarkdownSpan;

/**
 * Context structure for markdown generation that determines output format
 * Used to control whether generation produces raw markdown or styled HTML
 */
struct BP2AI_API FMarkdownGenerationContext
{
public:
    enum class EOutputFormat : uint8
    {
        RawMarkdown,
        StyledHTML,
        CleanText // NEW - for copy functionality
    };

private:
    EOutputFormat OutputFormat = EOutputFormat::RawMarkdown;

public:
    // Constructors
    FMarkdownGenerationContext() = default;
    explicit FMarkdownGenerationContext(EOutputFormat InFormat) : OutputFormat(InFormat) {}

    // Accessors
    EOutputFormat GetOutputFormat() const { return OutputFormat; }
    void SetOutputFormat(EOutputFormat InFormat) { OutputFormat = InFormat; }
    
    // Convenience methods
    bool IsHTML() const { return OutputFormat == EOutputFormat::StyledHTML; }
    bool IsMarkdown() const { return OutputFormat == EOutputFormat::RawMarkdown; }
    bool IsClean() const { return OutputFormat == EOutputFormat::CleanText; } // NEW
    
    // Comparison operators
    bool operator==(const FMarkdownGenerationContext& Other) const
    {
        return OutputFormat == Other.OutputFormat;
    }
    
    bool operator!=(const FMarkdownGenerationContext& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * RAII-style context manager for FMarkdownSpan
 * Automatically sets context on construction and clears on destruction
 * Ensures thread-safe context management
 */
class BP2AI_API FMarkdownContextManager
{
public:
	explicit FMarkdownContextManager(const FMarkdownGenerationContext& Context);
    
	~FMarkdownContextManager();
    
	// Non-copyable, non-movable for safety
	FMarkdownContextManager(const FMarkdownContextManager&) = delete;
	FMarkdownContextManager& operator=(const FMarkdownContextManager&) = delete;
	FMarkdownContextManager(FMarkdownContextManager&&) = delete;
	FMarkdownContextManager& operator=(FMarkdownContextManager&&) = delete;
};