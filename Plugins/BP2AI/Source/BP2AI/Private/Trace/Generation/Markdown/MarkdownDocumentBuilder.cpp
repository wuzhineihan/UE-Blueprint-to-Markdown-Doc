/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/Markdown/MarkdownDocumentBuilder.cpp


#include "MarkdownDocumentBuilder.h"
#include "Trace/Utils/MarkdownSpanSystem.h"
#include "Trace/FMarkdownPathTracer.h"
#include "Logging/BP2AILog.h"
#include "Trace/Generation/GenerationShared.h"
#include "Settings/BP2AIExportConfig.h"

FMarkdownDocumentBuilder::FMarkdownDocumentBuilder()
{
    // UE_LOG(LogBP2AI, Log, TEXT("FMarkdownDocumentBuilder: Initialized for Markdown-only output.")); // Removed
    CleanCodeBlockFormatter = MakeUnique<FCleanMarkdownSemanticFormatter>();
    StyledContentFormatter = MakeUnique<FStyledMarkdownSemanticFormatter>();
}
FString FMarkdownDocumentBuilder::BuildDocument(const FTracingResults& TracingData, const FGenerationSettings& Settings)
{
    // ✅ SET MARKDOWN CONTEXT for consistent formatting
    FMarkdownGenerationContext MarkdownContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    FMarkdownContextManager ContextManager(MarkdownContext);

    
    CurrentSettings = &Settings; // Store settings reference
    
    TArray<FString> AllOutputLines;
    
    if (BP2AIExportConfig::bDetailedBlueprintLog)
    {
        UE_LOG(LogBP2AI, Log, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Processing %d execution traces. UseSemanticData: %s"), TracingData.ExecutionTraces.Num(), Settings.bUseSemanticData ? TEXT("TRUE") : TEXT("FALSE"));
    }
    
    // Process execution traces  
    for (const FTraceEntry& TraceEntry : TracingData.ExecutionTraces)
    {
        // ZONE 3: Rich formatting for trace headers  
        AddTraceStartHeader(AllOutputLines, TraceEntry.TraceName, TraceEntry.NodeType);
    
        // ZONE 1: CLEAN code block content
        BeginCodeBlock(AllOutputLines, TEXT("blueprint"));
        TArray<FString> CleanLines = GenerateCleanExecutionLines(TraceEntry);
        AllOutputLines.Append(CleanLines);
        EndCodeBlock(AllOutputLines);
    
        AddSectionSeparator(AllOutputLines);
    }
    
    // Process section headers and graph definitions (ProcessStructuredContent will be updated next)
    ProcessStructuredContent(TracingData, AllOutputLines);
    
    // TOC generation for Markdown is typically not done, or handled by external tools.
    // GenerateAndPrependTOC(AllOutputLines, TracingData); // Stays empty as per current logic.
    
    if (BP2AIExportConfig::bDetailedBlueprintLog)
    {
        UE_LOG(LogBP2AI, Log, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Generated document with %d lines"), AllOutputLines.Num());
    }
    
    return FString::Join(AllOutputLines, TEXT("\n"));
}

void FMarkdownDocumentBuilder::ProcessStructuredContent(const FTracingResults& TracingData, TArray<FString>& OutLines)
{
    if (BP2AIExportConfig::bDetailedBlueprintLog)
    {
        UE_LOG(LogBP2AI, Log, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Processing %d section headers and %d graph definitions. UseSemanticData from CurrentSettings: %s"), 
            TracingData.SectionHeaders.Num(), TracingData.GraphDefinitions.Num(), CurrentSettings ? (CurrentSettings->bUseSemanticData ? TEXT("TRUE") : TEXT("FALSE")) : TEXT("SETTINGS_NULL"));
    }

    TMap<FString, TArray<FGraphDefinitionEntry>> DefinitionsByCategory;
    for (const FGraphDefinitionEntry& GraphDef : TracingData.GraphDefinitions)
    {
        if (IsCategoryVisible(GraphDef.Category))
        {
            DefinitionsByCategory.FindOrAdd(GraphDef.Category).Add(GraphDef);
        }
        else
        {
            UE_LOG(LogBP2AI, Verbose, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Filtering out definition '%s' from invisible category '%s'"), 
                *GraphDef.GraphName, *GraphDef.Category);
        }
    }

    // Routine summary -> Log (not Warning)
    if (BP2AIExportConfig::bDetailedBlueprintLog)
    {
        UE_LOG(LogBP2AI, Log, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Visible definition category count: %d"), DefinitionsByCategory.Num());
    }
    for (const auto& CategoryPair : DefinitionsByCategory)
    {
        UE_LOG(LogBP2AI, Verbose, TEXT("  Category '%s': %d definitions"), 
            *CategoryPair.Key, CategoryPair.Value.Num());
    }

    UE_LOG(LogBP2AI, Verbose, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Processing sections in ExecutePrintingPhase order:"));
    
    for (const FSectionMetadata& Section : TracingData.SectionHeaders)
    {
        UE_LOG(LogBP2AI, Verbose, TEXT("  Section: '%s' (Level %d, Major: %s)"), 
            *Section.SectionName, Section.Level, Section.bIsMajorSection ? TEXT("YES") : TEXT("NO"));

        if (Section.Level == 3 && !IsCategoryVisible(Section.SectionName))
        {
            UE_LOG(LogBP2AI, Verbose, TEXT("    SKIPPED: Category '%s' is not visible"), *Section.SectionName);
            continue;
        }

        AddSectionHeader(OutLines, Section.SectionName, Section.Level);
        
        if (Section.Level == 3)
        {
            if (DefinitionsByCategory.Contains(Section.SectionName))
            {
                const TArray<FGraphDefinitionEntry>& Definitions = DefinitionsByCategory[Section.SectionName];
                UE_LOG(LogBP2AI, Verbose, TEXT("    MATCHED: Adding %d definitions for '%s'"), 
                    Definitions.Num(), *Section.SectionName);
                
                TArray<FGraphDefinitionEntry> SortedDefinitions = Definitions;
                SortedDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                    return A.GraphName < B.GraphName;
                });
                
                for (const FGraphDefinitionEntry& GraphDef : SortedDefinitions)
                {
                    // ✅ Pass settings to AddGraphDefinition
                    AddGraphDefinition(OutLines, GraphDef, *CurrentSettings);
                }
                DefinitionsByCategory.Remove(Section.SectionName);
            }
            else
            {
                UE_LOG(LogBP2AI, Verbose, TEXT("    NO DEFINITIONS: Category '%s' has no definitions to add"), 
                    *Section.SectionName);
            }
        }
    }
    
    if (!DefinitionsByCategory.IsEmpty())
    {
        UE_LOG(LogBP2AI, Error, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): ERROR: %d categories have definitions but no section headers!"), 
            DefinitionsByCategory.Num());
        
        for (const auto& CategoryPair : DefinitionsByCategory)
        {
            UE_LOG(LogBP2AI, Error, TEXT("  Orphaned category: '%s' with %d definitions"), 
                *CategoryPair.Key, CategoryPair.Value.Num());
        }
        
        UE_LOG(LogBP2AI, Error, TEXT("Adding orphaned definitions as fallback..."));
        AddSectionHeader(OutLines, TEXT("Orphaned Definitions"), 2);
        
        for (const auto& CategoryPair : DefinitionsByCategory)
        {
            AddSectionHeader(OutLines, CategoryPair.Key, 3);
            
            TArray<FGraphDefinitionEntry> SortedDefinitions = CategoryPair.Value;
            SortedDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                return A.GraphName < B.GraphName;
            });
            
            for (const FGraphDefinitionEntry& GraphDef : SortedDefinitions)
            {
                AddGraphDefinition(OutLines, GraphDef, *CurrentSettings);
            }
        }
    }
    else
    {
        if (BP2AIExportConfig::bDetailedBlueprintLog)
        {
            UE_LOG(LogBP2AI, Log, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): SUCCESS: All definitions matched to section headers"));
        }
    }
}


void FMarkdownDocumentBuilder::AddGraphDefinition(TArray<FString>& OutLines, const FGraphDefinitionEntry& GraphDef, const FGenerationSettings& Settings)
{
    UE_LOG(LogBP2AI, Verbose, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): Adding graph definition for '%s'. UseSemanticData: %s"), *GraphDef.GraphName, Settings.bUseSemanticData ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Graph header - Use StyledContentFormatter if we want **`Name`** for the header.
    // Or, keep direct formatting for headers as they are simple.
    // For now, direct formatting for the header itself.
    OutLines.Add(FString::Printf(TEXT("#### `%s` <a id=\"%s\"></a>"), *GraphDef.GraphName, *GraphDef.AnchorId));
    OutLines.Add(TEXT(""));
    
    // Inputs
    OutLines.Add(TEXT("**Inputs:**"));
    if (GraphDef.InputSpecs.IsEmpty())
    {
        OutLines.Add(TEXT("*(No distinct data inputs)*"));
    }
    else
    {
        for (const FString& InputSpec : GraphDef.InputSpecs)
        {
            // InputSpecs are legacy strings, often already styled (e.g. `Name` (type)).
            // For styled markdown output, we might pass them through StyledContentFormatter if they were semantic.
            // For now, pass through as they are.
            OutLines.Add(TEXT("* ") + InputSpec);
        }
    }
    OutLines.Add(TEXT(""));
    
    // Execution flow
    if (!GraphDef.bIsPure)
    {
        OutLines.Add(TEXT("**Execution Flow:**"));
        OutLines.Add(TEXT(""));
        BeginCodeBlock(OutLines, TEXT("blueprint"));
    
        // Generate clean execution flow
        for (const FString& Line : GraphDef.ExecutionFlow)
        {
            FString CleanLine = CleanExecutionLine(Line);
            OutLines.Add(CleanLine);
        }
    
        EndCodeBlock(OutLines);
    }
    else
    {
        OutLines.Add(TEXT("*(Pure Graph - No Execution Flow)*"));
        OutLines.Add(TEXT(""));
    }
    
    // Outputs
    OutLines.Add(TEXT("**Outputs:**"));
    if (GraphDef.OutputSpecs.IsEmpty())
    {
        OutLines.Add(TEXT("*(No distinct data outputs)*"));
    }
    else
    {
        for (const FString& OutputSpec : GraphDef.OutputSpecs)
        {
            OutLines.Add(TEXT("* ") + OutputSpec);
        }
    }
    OutLines.Add(TEXT(""));
    
    AddSectionSeparator(OutLines);
}

void FMarkdownDocumentBuilder::AddSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level) const
{
    // int32 ClampedLevel = FMath::Clamp(Level, 1, 6); // Removed clamping
    // FString MarkdownHeader = FString::ChrN(ClampedLevel, TEXT('#')) + TEXT(" ") + Title;
    FString MarkdownHeader = FString::ChrN(Level, TEXT('#')) + TEXT(" ") + Title; // Uses original Level
    
    // if (!OutLines.IsEmpty() && !OutLines.Last().IsEmpty()) // Removed conditional empty line
    // {
    //     OutLines.Add(TEXT(""));
    // }
    
    OutLines.Add(MarkdownHeader);
    // OutLines.Add(TEXT("")); // Removed adding an extra empty line after the header
}

void FMarkdownDocumentBuilder::AddSectionSeparator(TArray<FString>& OutLines, bool bIsMajor) const
{
    // Simplified condition and separator type
    // if (OutLines.IsEmpty() || 
    //     (!OutLines.Last().Equals(TEXT("---")) && 
    //      !OutLines.Last().Equals(TEXT("***")) && // No longer checks for "***"
    //      !OutLines.Last().IsEmpty()))
    if (OutLines.IsEmpty() || OutLines.Last() != TEXT("---")) // Simplified condition
    {
        // if (!OutLines.IsEmpty()) // Removed this specific check, covered by the outer if
        // {
        //     OutLines.Add(TEXT("")); // Removed adding an empty line before the separator
        // }
        
        // FString Separator = bIsMajor ? TEXT("***") : TEXT("---"); // No longer uses bIsMajor
        OutLines.Add(TEXT("---")); // Always uses "---"
        // OutLines.Add(TEXT("")); // Removed adding an empty line after the separator
    }
}

void FMarkdownDocumentBuilder::AddTraceStartHeader(TArray<FString>& OutLines, const FString& TraceName, const FString& NodeType) const
{
    FString HeaderText = FString::Printf(TEXT("**Trace Start: %s**"), *TraceName);
    
    if (!NodeType.IsEmpty())
    { 
        HeaderText += FString::Printf(TEXT(" (%s)"), *NodeType);
    }
    
    if (!OutLines.IsEmpty() && !OutLines.Last().IsEmpty())
    {
        OutLines.Add(TEXT(""));
    }
    OutLines.Add(HeaderText);
    OutLines.Add(TEXT(""));
}

void FMarkdownDocumentBuilder::BeginCodeBlock(TArray<FString>& OutLines, const FString& Language) const
{
    OutLines.Add(FString::Printf(TEXT("```%s"), *Language));
}

void FMarkdownDocumentBuilder::EndCodeBlock(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("```"));
}

void FMarkdownDocumentBuilder::AddParagraph(TArray<FString>& OutLines, const FString& Text) const
{
    if (!OutLines.IsEmpty() && !OutLines.Last().IsEmpty())
    {
        OutLines.Add(TEXT(""));
    }
    OutLines.Add(Text);
    OutLines.Add(TEXT(""));
}

void FMarkdownDocumentBuilder::BeginUnorderedList(TArray<FString>& OutLines) const
{
    // Markdown lists don't need explicit begin/end markers
    // if (!OutLines.IsEmpty() && !OutLines.Last().IsEmpty()) // Removed conditional empty line
    // {
    //     OutLines.Add(TEXT(""));
    // }
}

void FMarkdownDocumentBuilder::EndUnorderedList(TArray<FString>& OutLines) const
{
    if (!OutLines.IsEmpty() && !OutLines.Last().IsEmpty())
    {
        OutLines.Add(TEXT(""));
    }
}

void FMarkdownDocumentBuilder::AddListItem(TArray<FString>& OutLines, const FString& ItemContent) const
{
    OutLines.Add(TEXT("* ") + ItemContent);
}

void FMarkdownDocumentBuilder::GenerateAndPrependTOC(TArray<FString>& AllOutputLines, const FTracingResults& TracingData) const
{
    // CRITICAL FIX: This method should do NOTHING for Markdown
    // TOC is only for HTML output according to legacy behavior
    // Empty implementation - no TOC for Markdown
    UE_LOG(LogBP2AI, Verbose, TEXT("MarkdownDocumentBuilder: Skipping TOC generation for Markdown output")); // Log retained from target
}

// ✅ NEW: Category visibility helper implementation
bool FMarkdownDocumentBuilder::IsCategoryVisible(const FString& CategoryString) const
{
    if (!CurrentSettings)
    {
        // No settings available - default to visible (preserves existing behavior)
        return true;
    }
    
    return CategoryUtils::IsCategoryStringVisible(CategoryString, *CurrentSettings);
}

void FMarkdownDocumentBuilder::ProcessSemanticTraceEntry(const FTraceEntry& TraceEntry, TArray<FString>& OutLines)
{
    // This method is called when Settings.bUseSemanticData is true AND TraceEntry.ExecutionSteps is not empty.
    // It formats semantic steps for Markdown output, using the CleanCodeBlockFormatter for code block content.
    if (CleanCodeBlockFormatter.IsValid())
    {
        ProcessSemanticExecutionSteps(TraceEntry.ExecutionSteps, OutLines, true /*bForCodeBlock*/);
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): CleanCodeBlockFormatter is not valid in ProcessSemanticTraceEntry!"));
        OutLines.Add(TEXT("<!-- Error: CleanCodeBlockFormatter not initialized -->"));
    }
}

void FMarkdownDocumentBuilder::ProcessSemanticExecutionSteps(const TArray<FSemanticExecutionStep>& Steps, TArray<FString>& OutLines, bool bForCodeBlock)
{
    // Helper to format a list of semantic steps into lines for Markdown.
    // Uses CleanCodeBlockFormatter if bForCodeBlock is true, otherwise StyledContentFormatter.
    ISemanticFormatter* FormatterToUse = nullptr;
    if (bForCodeBlock)
    {
        if (CleanCodeBlockFormatter.IsValid())
        {
            FormatterToUse = CleanCodeBlockFormatter.Get();
        }
        else
        {
             UE_LOG(LogBP2AI, Error, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): CleanCodeBlockFormatter is not valid!"));
        }
    }
    else
    {
        if (StyledContentFormatter.IsValid())
        {
            FormatterToUse = StyledContentFormatter.Get();
        }
        else
        {
            UE_LOG(LogBP2AI, Error, TEXT("MarkdownDocumentBuilder (!!!MARKDOWN!!!): StyledContentFormatter is not valid!"));
        }
    }

    if (!FormatterToUse)
    {
        OutLines.Add(TEXT("<!-- Error: Appropriate Markdown formatter not initialized for execution steps -->"));
        // As a basic fallback, just output node names if no formatter is available
        for (const FSemanticExecutionStep& Step : Steps)
        {
            OutLines.Add(Step.IndentPrefix + TEXT("* ") + Step.NodeName + TEXT(" (Formatter Missing)"));
        }
        return;
    }

    for (const FSemanticExecutionStep& Step : Steps)
    {
        OutLines.Add(FormatterToUse->FormatExecutionStep(Step));
    }
}

TArray<FString> FMarkdownDocumentBuilder::GenerateCleanExecutionLines(const FTraceEntry& TraceEntry)
{
    TArray<FString> CleanLines;
    
    // Set CleanText context for this generation
    FMarkdownGenerationContext CleanContext(FMarkdownGenerationContext::EOutputFormat::CleanText);
    FMarkdownContextManager CleanContextManager(CleanContext);
    
    UE_LOG(LogBP2AI, Verbose, TEXT("GenerateCleanExecutionLines: Converting %d lines to clean format"), TraceEntry.ExecutionLines.Num());
    
    for (const FString& Line : TraceEntry.ExecutionLines)
    {
        FString CleanLine = CleanExecutionLine(Line);
        CleanLines.Add(CleanLine);
    }
    
    return CleanLines;
}

FString FMarkdownDocumentBuilder::CleanExecutionLine(const FString& Line)
{
    // Preserve structural elements (indentation, pipes, bullets)
    FString StructuralPrefix;
    int32 ContentStart = 0;
    
    // Extract leading whitespace and structural characters
    for (int32 i = 0; i < Line.Len(); i++)
    {
        TCHAR Char = Line[i];
        if (Char == TEXT(' ') || Char == TEXT('\t') || Char == TEXT('|') || Char == TEXT('-') || Char == TEXT('L'))
        {
            StructuralPrefix.AppendChar(Char);
        }
        else if (Char == TEXT('*') && i + 1 < Line.Len() && Line[i + 1] == TEXT(' '))
        {
            // Preserve execution bullets  
            StructuralPrefix += TEXT("* ");
            ContentStart = i + 2;
            break;
        }
        else
        {
            ContentStart = i;
            break;
        }
    }
    
    if (ContentStart >= Line.Len())
    {
        return Line; // Line is all structural
    }
    
    FString Content = Line.Mid(ContentStart);
    FString CleanContent = ApplyCleaningRules(Content);
    
    return StructuralPrefix + CleanContent;
}

FString FMarkdownDocumentBuilder::ApplyCleaningRules(const FString& Content)
{
    FString Clean = Content;
    
    // Remove bold markers
    Clean = Clean.Replace(TEXT("**"), TEXT(""));
    
    // Remove backticks from identifiers
    Clean = Clean.Replace(TEXT("`"), TEXT(""));
    
    // Normalize quotes: 'string' -> "string"
    Clean = Clean.Replace(TEXT("'"), TEXT("\""));
    
    return Clean;
}