/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/HTML/HTMLDocumentBuilder.h

#pragma once

#include "Trace/Generation/GenerationShared.h"
#include "Trace/SemanticFormatter.h"

class BP2AI_API FHTMLDocumentBuilder : public IDocumentBuilder
{
public:
    FHTMLDocumentBuilder();
    virtual ~FHTMLDocumentBuilder() = default;

    // IDocumentBuilder interface
    virtual FString BuildDocument(const FTracingResults& TracingData, const FGenerationSettings& Settings) override;
    virtual FString GetFileExtension() const override { return TEXT("html"); }
    virtual FString GetDisplayName() const override { return TEXT("HTML"); }

    // ✅ MAIN IMPLEMENTATION: Complete HTML with embedded copy data
    FString BuildDocumentWithCopyData(
        const FTracingResults& HTMLResults,
        const FTracingResults& MarkdownResults,
        const FGenerationSettings& Settings);

    // External JS loading for testing
    FString LoadExternalJS() const;

    FBlueprintTypeInfo ParseTypeFromIOSpec(const FString& IOSpec) const;
    
private:
    const FGenerationSettings* CurrentSettings = nullptr;
    TUniquePtr<FHTMLSemanticFormatter> HTMLSemanticFormatter;

    // ✅ CORE IMPLEMENTATION: Process all content with copy data embedding
    void ProcessAllContentWithCopyData(
        const FTracingResults& HTMLResults,
        const FTracingResults& MarkdownResults,
        TArray<FString>& OutLines);

    // ✅ EXECUTION TRACES: Add trace with copy unit support
    void AddExecutionTraceWithCopyData(
        const FTraceEntry& HTMLTrace,
        const FTraceEntry* MarkdownTrace,
        int32 TraceIndex,
        TArray<FString>& OutLines,
        const FGenerationSettings& Settings);
    
    // ✅ GRAPH DEFINITIONS: Add definition with embedded copy data
    void AddGraphDefinitionWithCopyData(
        const FGraphDefinitionEntry& HTMLGraphDef,
        const FGraphDefinitionEntry* MarkdownGraphDef,
        int32 DefinitionIndex,
        TArray<FString>& OutLines,
        const FGenerationSettings& Settings);

    // ✅ UTILITY: Create copy-enabled containers
    void BeginTraceContainer(TArray<FString>& OutLines, int32 TraceIndex);
    void EndTraceContainer(TArray<FString>& OutLines);
    void BeginDefinitionContainer(TArray<FString>& OutLines, int32 DefinitionIndex);
    void EndDefinitionContainer(TArray<FString>& OutLines);

    // ✅ UTILITY: Data embedding helpers
    FString EscapeForHTMLAttribute(const FString& Text) const;
    FString JoinWithNewlines(const TArray<FString>& Lines) const;
    FString CreateDataAttribute(const FString& AttributeName, const FString& Content) const;

    // HTML formatting helpers (preserved for compatibility)
    void AddSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level, bool bTitleIsPreformattedHtml = false) const;
    void AddSectionSeparator(TArray<FString>& OutLines, bool bIsMajor = false) const;
    void AddTraceStartHeader(TArray<FString>& OutLines, const FString& TraceName, const FString& NodeType = TEXT("")) const;
    void AddParagraph(TArray<FString>& OutLines, const FString& Text, bool bTextIsPreformattedHtml = false) const;
    void BeginUnorderedList(TArray<FString>& OutLines) const;
    void EndUnorderedList(TArray<FString>& OutLines) const;
    void AddListItem(TArray<FString>& OutLines, const FString& ItemContent) const;
    
    // Complete HTML document generation
    FString WrapInCompleteHTMLDocument(const FString& BodyContent) const;
    FString GetInteractiveFeaturesJavaScript() const;
    void GenerateAndPrependTOC(TArray<FString>& AllOutputLines, const FTracingResults& TracingData) const;
    
    // Interactive code blocks
    void BeginInteractiveCodeBlock(TArray<FString>& OutLines, const FString& Title = TEXT(""), const FString& Language = TEXT("blueprint")) const;
    void EndInteractiveCodeBlock(TArray<FString>& OutLines) const;
    void AddCollapsibleSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level, const FString& SectionId) const;
    void BeginCollapsibleContent(TArray<FString>& OutLines) const;
    void EndCollapsibleContent(TArray<FString>& OutLines) const;

    // Category visibility and enhanced navigation
    bool IsCategoryVisible(const FString& CategoryString) const;
    FString ProcessExecutionLineForLinks(const FString& FlowLine);
    
    // Semantic data processing Deprecated
    void ProcessSemanticTraceEntry(const FTraceEntry& TraceEntry, TArray<FString>& OutLines);
    void ProcessSemanticExecutionSteps(const TArray<FSemanticExecutionStep>& Steps, TArray<FString>& OutLines);

    // ✅ NEW: Modern layout generation methods  
    void GenerateModernHTMLLayout(
        const FTracingResults& HTMLResults,
        const FTracingResults& MarkdownResults,
        TArray<FString>& OutLines);
        
    void GenerateSidebarLayout(
        TArray<FString>& OutLines, 
        const FTracingResults& TracingData) const;
        
    void GenerateMainContentArea(
        const FTracingResults& HTMLResults,
        const FTracingResults& MarkdownResults,
        TArray<FString>& OutLines);

	// ✅ ADDED: Helper to render definition cards for a given category.
	void AddDefinitionCardsForCategory(
		const FString& CategoryName,
		const TMap<FString, TArray<FGraphDefinitionEntry>>& DefinitionsByCategory,
		const TMap<FString, const FGraphDefinitionEntry*>& MarkdownDefMap,
		int32& DefinitionIndex, // Pass by reference to maintain a unique index
		TArray<FString>& OutLines);
    

    // ✅ CORRECTED: Execution trace and definition card methods
    void AddExecutionTraceCard(
        const FTraceEntry& HTMLTrace,
        const FTraceEntry* MarkdownTrace,
        int32 TraceIndex,
        TArray<FString>& OutLines);
        
    void AddGraphDefinitionCard(
        const FGraphDefinitionEntry& HTMLGraphDef,
        const FGraphDefinitionEntry* MarkdownGraphDef,
        int32 DefIndex,
        TArray<FString>& OutLines);
        
       
    void AddExecutionFlowSection(
        const FGraphDefinitionEntry& GraphDef,
        TArray<FString>& OutLines);
        
    // ✅ UTILITY METHODS

    FString DetermineElementType(const FString& Category) const;
    FString CreateSectionId(const FString& Title) const;
    FString GetModernInteractiveJavaScript() const;


    void AddModernIOSection(
        const FGraphDefinitionEntry& GraphDef,
        const FString& SectionTitle,
        bool bIsInputs,
        TArray<FString>& OutLines);
        
    void AddExecutionFlowSectionFixed(
        const FGraphDefinitionEntry& GraphDef,
        TArray<FString>& OutLines);
    
    // ✅ FIX: Enhanced type parsing methods
    FBlueprintTypeInfo ParseTypeFromString(const FString& TypeString) const;
    FString NormalizeTypeName(const FString& TypeName) const;
    FString GenerateTypeCSSClasses(const FBlueprintTypeInfo& TypeInfo) const;

  

    FString LoadModernUIJS() const;


    bool bEnablePerformanceMode = true;
    FString GetCategoryTOCClass(const FString& CategoryName) const;


    
};