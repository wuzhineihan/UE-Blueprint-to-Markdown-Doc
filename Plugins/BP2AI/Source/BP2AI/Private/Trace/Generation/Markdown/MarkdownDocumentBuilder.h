/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/Markdown/MarkdownDocumentBuilder.h

#pragma once

#include "Trace/Generation/GenerationShared.h"
#include "Trace/SemanticFormatter.h"


class BP2AI_API FMarkdownDocumentBuilder : public IDocumentBuilder
{
public:
	FMarkdownDocumentBuilder();
	virtual ~FMarkdownDocumentBuilder() = default;

	// IDocumentBuilder interface
	virtual FString BuildDocument(const FTracingResults& TracingData, const FGenerationSettings& Settings) override;
	virtual FString GetFileExtension() const override { return TEXT("md"); }
	virtual FString GetDisplayName() const override { return TEXT("Markdown"); }


		
	// NEW: Generate clean execution lines for code blocks
	TArray<FString> GenerateCleanExecutionLines(const FTraceEntry& TraceEntry);

	// NEW: Clean a single execution line while preserving structure
	FString CleanExecutionLine(const FString& Line);

	// NEW: Apply cleaning rules to content portion only  
	FString ApplyCleaningRules(const FString& Content);

	
private:
	// Settings reference for category filtering
	const FGenerationSettings* CurrentSettings = nullptr;

	
	
	// NOT USED: Semantic formatters
	TUniquePtr<FCleanMarkdownSemanticFormatter> CleanCodeBlockFormatter; // For code blocks (no surrounding **, ``)
	TUniquePtr<FStyledMarkdownSemanticFormatter> StyledContentFormatter; // For headers, list items, etc. (uses **, ``)

	
	// Markdown-specific formatting helpers
	void AddSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level) const;
	void AddSectionSeparator(TArray<FString>& OutLines, bool bIsMajor = false) const;
	void AddTraceStartHeader(TArray<FString>& OutLines, const FString& TraceName, const FString& NodeType = TEXT("")) const;
	void BeginCodeBlock(TArray<FString>& OutLines, const FString& Language = TEXT("blueprint")) const;
	void EndCodeBlock(TArray<FString>& OutLines) const;
	void AddParagraph(TArray<FString>& OutLines, const FString& Text) const;
	void BeginUnorderedList(TArray<FString>& OutLines) const;
	void EndUnorderedList(TArray<FString>& OutLines) const;
	void AddListItem(TArray<FString>& OutLines, const FString& ItemContent) const;

	// Structured content processing
	void ProcessStructuredContent(const FTracingResults& TracingData, TArray<FString>& OutLines);
	void AddGraphDefinition(TArray<FString>& OutLines, const FGraphDefinitionEntry& GraphDef, const FGenerationSettings& Settings);

	// TOC generation (empty for Markdown - TOC only for HTML)
	void GenerateAndPrependTOC(TArray<FString>& AllOutputLines, const FTracingResults& TracingData) const;

	// NEW: Category visibility helper
	bool IsCategoryVisible(const FString& CategoryString) const;

	
	// ✅ ADDED: New methods for processing semantic data
	void ProcessSemanticTraceEntry(const FTraceEntry& TraceEntry, TArray<FString>& OutLines);
	void ProcessSemanticExecutionSteps(const TArray<FSemanticExecutionStep>& Steps, TArray<FString>& OutLines, bool bForCodeBlock);
};