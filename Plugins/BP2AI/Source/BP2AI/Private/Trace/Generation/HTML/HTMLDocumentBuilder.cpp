/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/HTML/HTMLDocumentBuilder.cpp


#include "HTMLDocumentBuilder.h"
#include "Trace/Utils/MarkdownSpanSystem.h"
#include "Trace/BlueprintMarkdownCSS.h"
#include "Trace/FMarkdownPathTracer.h"
#include "Logging/BP2AILog.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Trace/SemanticFormatter.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"  // For FBlueprintTypeInfo
#include "Internationalization/Regex.h"


FHTMLDocumentBuilder::FHTMLDocumentBuilder()
{
    UE_LOG(LogBP2AI, Log, TEXT("FHTMLDocumentBuilder: Initialized for HTML output with copy units support."));
    HTMLSemanticFormatter = MakeUnique<FHTMLSemanticFormatter>();
}

// ✅ MAIN INTERFACE: Delegate to copy-enabled version for HTML contexts
FString FHTMLDocumentBuilder::BuildDocument(const FTracingResults& TracingData, const FGenerationSettings& Settings)
{
    // Set HTML context for consistent formatting
    FMarkdownGenerationContext HTMLContext(FMarkdownGenerationContext::EOutputFormat::StyledHTML);
    FMarkdownContextManager ContextManager(HTMLContext);

    // For backward compatibility: if we don't have copy data, generate it
    // This ensures the main interface always provides copy functionality
    UE_LOG(LogBP2AI, Log, TEXT("HTMLDocumentBuilder: Delegating to copy-enabled version"));
    
    // Use the same data for both HTML and Markdown (suboptimal but functional)
    return BuildDocumentWithCopyData(TracingData, TracingData, Settings);
}

// ✅ COMPLETE IMPLEMENTATION: HTML with embedded copy data
// ✅ MODIFY EXISTING METHOD: Update BuildDocumentWithCopyData to use modern layout
FString FHTMLDocumentBuilder::BuildDocumentWithCopyData(
    const FTracingResults& HTMLResults,
    const FTracingResults& MarkdownResults,
    const FGenerationSettings& Settings)
{
    FMarkdownGenerationContext HTMLContext(FMarkdownGenerationContext::EOutputFormat::StyledHTML);
    FMarkdownContextManager ContextManager(HTMLContext);

    CurrentSettings = &Settings;
    TArray<FString> AllOutputLines;
    
    UE_LOG(LogBP2AI, Log, TEXT("HTMLDocumentBuilder: Building modern layout with copy units support"));
    
    // ✅ NEW: Use modern layout generation
    GenerateModernHTMLLayout(HTMLResults, MarkdownResults, AllOutputLines);
    
    UE_LOG(LogBP2AI, Log, TEXT("HTMLDocumentBuilder: Generated document with %d HTML lines"), AllOutputLines.Num());
    
    return WrapInCompleteHTMLDocument(FString::Join(AllOutputLines, TEXT("\n")));
}

// ✅ CORE PROCESSING: Handle all content with copy data embedding
void FHTMLDocumentBuilder::ProcessAllContentWithCopyData(
    const FTracingResults& HTMLResults,
    const FTracingResults& MarkdownResults,
    TArray<FString>& OutLines)
{
    // Create lookup map for markdown traces
    TMap<FString, const FTraceEntry*> MarkdownTraceMap;
    for (const FTraceEntry& MTrace : MarkdownResults.ExecutionTraces)
    {
        MarkdownTraceMap.Add(MTrace.TraceName, &MTrace);
    }
    
    // ✅ PHASE 1: Process execution traces with copy data
    for (int32 TraceIndex = 0; TraceIndex < HTMLResults.ExecutionTraces.Num(); ++TraceIndex)
    {
        const FTraceEntry& HTMLTrace = HTMLResults.ExecutionTraces[TraceIndex];
        const FTraceEntry* MarkdownTrace = MarkdownTraceMap.FindRef(HTMLTrace.TraceName);
        
        AddExecutionTraceWithCopyData(HTMLTrace, MarkdownTrace, TraceIndex, OutLines, *CurrentSettings);
        AddSectionSeparator(OutLines);
    }
    
    // Create lookup map for markdown definitions
    TMap<FString, const FGraphDefinitionEntry*> MarkdownDefMap;
    for (const FGraphDefinitionEntry& MDef : MarkdownResults.GraphDefinitions)
    {
        MarkdownDefMap.Add(MDef.GraphName, &MDef);
    }
    
    // ✅ PHASE 2: Process section headers and graph definitions with copy data
    TMap<FString, TArray<FGraphDefinitionEntry>> DefinitionsByCategory;
    for (const FGraphDefinitionEntry& GraphDef : HTMLResults.GraphDefinitions)
    {
        if (IsCategoryVisible(GraphDef.Category))
        {
            DefinitionsByCategory.FindOrAdd(GraphDef.Category).Add(GraphDef);
        }
    }
    
    bool bInCollapsibleSection = false;
    int32 DefinitionIndex = 0;
    
    for (const FSectionMetadata& Section : HTMLResults.SectionHeaders)
    {
        if (Section.Level == 3 && !IsCategoryVisible(Section.SectionName))
        {
            continue;
        }
        
        if (Section.bIsMajorSection)
        {
            if (bInCollapsibleSection)
            {
                EndCollapsibleContent(OutLines);
                bInCollapsibleSection = false;
            }
            AddSectionSeparator(OutLines, true);
            FString SectionId = Section.SectionName.Replace(TEXT(" "), TEXT("-")).ToLower();
            AddCollapsibleSectionHeader(OutLines, Section.SectionName, Section.Level, SectionId);
            BeginCollapsibleContent(OutLines);
            bInCollapsibleSection = true;
        }
        else
        {
            AddSectionHeader(OutLines, Section.SectionName, Section.Level);
        }
        
        if (DefinitionsByCategory.Contains(Section.SectionName))
        {
            TArray<FGraphDefinitionEntry>& CategoryDefinitions = DefinitionsByCategory[Section.SectionName];
            CategoryDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                return A.GraphName < B.GraphName;
            });
            
            for (const FGraphDefinitionEntry& HTMLGraphDef : CategoryDefinitions)
            {
                const FGraphDefinitionEntry* MarkdownGraphDef = MarkdownDefMap.FindRef(HTMLGraphDef.GraphName);
                AddGraphDefinitionWithCopyData(HTMLGraphDef, MarkdownGraphDef, DefinitionIndex++, OutLines, *CurrentSettings);
            }
            DefinitionsByCategory.Remove(Section.SectionName);
        }
    }
    
    if (bInCollapsibleSection)
    {
        EndCollapsibleContent(OutLines);
    }
    
    // Handle orphaned definitions
    for (const auto& Pair : DefinitionsByCategory)
    {
        if (!Pair.Value.IsEmpty())
        {
            AddSectionHeader(OutLines, Pair.Key, 3);
            TArray<FGraphDefinitionEntry> SortedDefinitions = Pair.Value;
            SortedDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                return A.GraphName < B.GraphName;
            });
            for (const FGraphDefinitionEntry& HTMLGraphDef : SortedDefinitions)
            {
                const FGraphDefinitionEntry* MarkdownGraphDef = MarkdownDefMap.FindRef(HTMLGraphDef.GraphName);
                AddGraphDefinitionWithCopyData(HTMLGraphDef, MarkdownGraphDef, DefinitionIndex++, OutLines, *CurrentSettings);
            }
        }
    }
}

// ✅ EXECUTION TRACES: Add trace with copy units support
void FHTMLDocumentBuilder::AddExecutionTraceWithCopyData(
    const FTraceEntry& HTMLTrace,
    const FTraceEntry* MarkdownTrace,
    int32 TraceIndex,
    TArray<FString>& OutLines,
    const FGenerationSettings& Settings)
{
    // Begin trace container with copy unit support
    BeginTraceContainer(OutLines, TraceIndex);
    
    // Add trace header
    AddTraceStartHeader(OutLines, HTMLTrace.TraceName, HTMLTrace.NodeType);
    
    // Prepare clean markdown data for Copy Unit 2 (formatted trace)
    FString CleanMarkdownData = TEXT("");
    if (MarkdownTrace && !MarkdownTrace->ExecutionLines.IsEmpty())
    {
        CleanMarkdownData = EscapeForHTMLAttribute(JoinWithNewlines(MarkdownTrace->ExecutionLines));
    }
    
    // Begin interactive code block with embedded clean data
    FString DataAttr = CleanMarkdownData.IsEmpty() ? TEXT("") : CreateDataAttribute(TEXT("clean-markdown"), CleanMarkdownData);
    OutLines.Add(FString::Printf(TEXT("<div class=\"execution-section\"%s>"), *DataAttr));
    OutLines.Add(TEXT("<p><span class=\"bp-keyword\">Execution Flow:</span></p>"));
    BeginInteractiveCodeBlock(OutLines, HTMLTrace.TraceName, TEXT("blueprint"));
    
    // ✅ CRITICAL FIX: Use newlines instead of <br /> for proper code formatting
    if (Settings.bUseSemanticData && !HTMLTrace.ExecutionSteps.IsEmpty())
    {
        ProcessSemanticExecutionSteps(HTMLTrace.ExecutionSteps, OutLines);
    }
    else if (!HTMLTrace.ExecutionLines.IsEmpty())
    {
        // Links are generated upstream now. Simply join the pre-formatted lines.
        FString JoinedContent = FString::Join(HTMLTrace.ExecutionLines, TEXT("\n"));
        OutLines.Add(JoinedContent);
    }
    else
    {
        OutLines.Add(TEXT("<!-- No execution content for this trace entry -->"));
    }
    
    EndInteractiveCodeBlock(OutLines);
    OutLines.Add(TEXT("</div>")); // Close execution-section
    
    // End trace container
    EndTraceContainer(OutLines);
}


// ✅ GRAPH DEFINITIONS: Add definition with embedded copy data for all units
void FHTMLDocumentBuilder::AddGraphDefinitionWithCopyData(
    const FGraphDefinitionEntry& HTMLGraphDef,
    const FGraphDefinitionEntry* MarkdownGraphDef,
    int32 DefinitionIndex,
    TArray<FString>& OutLines,
    const FGenerationSettings& Settings)
{
    // Begin definition container
    BeginDefinitionContainer(OutLines, DefinitionIndex);
    
    // Graph header
    FString ConsistentAnchorId = FMarkdownPathTracer::SanitizeAnchorName(HTMLGraphDef.GraphName);
    FString CleanGraphName = FMarkdownSpanSystem::CleanTextForHTML(HTMLGraphDef.GraphName);
    OutLines.Add(FString::Printf(TEXT("<h4 id=\"%s\"><span class=\"bp-func-name\">%s</span></h4>"), 
        *ConsistentAnchorId, *FMarkdownSpanSystem::EscapeHtml(CleanGraphName)));
    
    // Inputs Section with embedded markdown data for Copy Unit 3
    FString MarkdownInputsData = (MarkdownGraphDef && !MarkdownGraphDef->InputSpecs.IsEmpty()) ? 
        EscapeForHTMLAttribute(JoinWithNewlines(MarkdownGraphDef->InputSpecs)) : TEXT("");
    OutLines.Add(FString::Printf(TEXT("<div class=\"inputs-section\"%s>"), 
        MarkdownInputsData.IsEmpty() ? TEXT("") : *CreateDataAttribute(TEXT("markdown-inputs"), MarkdownInputsData)));
    OutLines.Add(TEXT("<p><span class=\"bp-keyword\">Inputs:</span></p>"));
    if (HTMLGraphDef.InputSpecs.IsEmpty())
    {
        OutLines.Add(TEXT("<p><span class=\"bp-modifier\">(No distinct data inputs)</span></p>"));
    }
    else
    {
        OutLines.Add(TEXT("<ul>"));
        for (const FString& InputSpec : HTMLGraphDef.InputSpecs)
        {
            OutLines.Add(FString::Printf(TEXT("<li>%s</li>"), *InputSpec));
        }
        OutLines.Add(TEXT("</ul>"));
    }
    OutLines.Add(TEXT("</div>")); // Close inputs-section
    
    // Execution Flow Section (if not pure)
    if (!HTMLGraphDef.bIsPure)
    {
        // Prepare clean markdown data for Copy Unit 1 and 3
        FString CleanMarkdownFlow = (MarkdownGraphDef && !MarkdownGraphDef->ExecutionFlow.IsEmpty()) ? 
            EscapeForHTMLAttribute(JoinWithNewlines(MarkdownGraphDef->ExecutionFlow)) : TEXT("");
        
        OutLines.Add(FString::Printf(TEXT("<div class=\"execution-section\"%s>"), 
            CleanMarkdownFlow.IsEmpty() ? TEXT("") : *CreateDataAttribute(TEXT("clean-markdown"), CleanMarkdownFlow)));
        OutLines.Add(TEXT("<p><span class=\"bp-keyword\">Execution Flow:</span></p>"));
        BeginInteractiveCodeBlock(OutLines, HTMLGraphDef.GraphName + TEXT(" Execution"), TEXT("blueprint"));
        
        // Add formatted HTML execution flow
        if (Settings.bUseSemanticData && !HTMLGraphDef.SemanticExecutionFlow.IsEmpty())
        {
            ProcessSemanticExecutionSteps(HTMLGraphDef.SemanticExecutionFlow, OutLines);
        }
        else if (!HTMLGraphDef.ExecutionFlow.IsEmpty())
        {
            // Links are generated upstream now. Simply join the pre-formatted lines.
            FString JoinedFlow = FString::Join(HTMLGraphDef.ExecutionFlow, TEXT("<br />\n"));
            OutLines.Add(JoinedFlow);
        }
        else
        {
            OutLines.Add(TEXT("<!-- No execution flow content for this graph definition -->"));
        }
        EndInteractiveCodeBlock(OutLines);
        OutLines.Add(TEXT("</div>")); // Close execution-section
    }
    else
    {
        OutLines.Add(TEXT("<p><span class=\"bp-modifier\">(Pure Graph - No Execution Flow)</span></p>"));
    }
    
    // Outputs Section with embedded markdown data for Copy Unit 3
    FString MarkdownOutputsData = (MarkdownGraphDef && !MarkdownGraphDef->OutputSpecs.IsEmpty()) ? 
        EscapeForHTMLAttribute(JoinWithNewlines(MarkdownGraphDef->OutputSpecs)) : TEXT("");
    OutLines.Add(FString::Printf(TEXT("<div class=\"outputs-section\"%s>"), 
        MarkdownOutputsData.IsEmpty() ? TEXT("") : *CreateDataAttribute(TEXT("markdown-outputs"), MarkdownOutputsData)));
    OutLines.Add(TEXT("<p><span class=\"bp-keyword\">Outputs:</span></p>"));
    if (HTMLGraphDef.OutputSpecs.IsEmpty())
    {
        OutLines.Add(TEXT("<p><span class=\"bp-modifier\">(No distinct data outputs)</span></p>"));
    }
    else
    {
        OutLines.Add(TEXT("<ul>"));
        for (const FString& OutputSpec : HTMLGraphDef.OutputSpecs)
        {
            OutLines.Add(FString::Printf(TEXT("<li>%s</li>"), *OutputSpec));
        }
        OutLines.Add(TEXT("</ul>"));
    }
    OutLines.Add(TEXT("</div>")); // Close outputs-section
    
    // End definition container
    EndDefinitionContainer(OutLines);
    AddSectionSeparator(OutLines);
}

// ✅ CONTAINER UTILITIES: Copy-enabled containers
void FHTMLDocumentBuilder::BeginTraceContainer(TArray<FString>& OutLines, int32 TraceIndex)
{
    OutLines.Add(FString::Printf(TEXT("<div class=\"trace-container\" id=\"trace-container-%d\">"), TraceIndex));
}

void FHTMLDocumentBuilder::EndTraceContainer(TArray<FString>& OutLines)
{
    OutLines.Add(TEXT("</div>"));
}

void FHTMLDocumentBuilder::BeginDefinitionContainer(TArray<FString>& OutLines, int32 DefinitionIndex)
{
    OutLines.Add(FString::Printf(TEXT("<div class=\"definition-container\" id=\"definition-container-%d\">"), DefinitionIndex));
}

void FHTMLDocumentBuilder::EndDefinitionContainer(TArray<FString>& OutLines)
{
    OutLines.Add(TEXT("</div>"));
}

// ✅ DATA EMBEDDING UTILITIES
FString FHTMLDocumentBuilder::EscapeForHTMLAttribute(const FString& Text) const
{
    return Text
        .Replace(TEXT("&"), TEXT("&amp;"))
        .Replace(TEXT("\""), TEXT("&quot;"))
        .Replace(TEXT("'"), TEXT("&#39;"))
        .Replace(TEXT("<"), TEXT("&lt;"))
        .Replace(TEXT(">"), TEXT("&gt;"))
        .Replace(TEXT("\n"), TEXT("&#10;"))
        .Replace(TEXT("\r"), TEXT(""));
}

FString FHTMLDocumentBuilder::JoinWithNewlines(const TArray<FString>& Lines) const
{
    return FString::Join(Lines, TEXT("\n"));
}

FString FHTMLDocumentBuilder::CreateDataAttribute(const FString& AttributeName, const FString& Content) const
{
    if (Content.IsEmpty())
    {
        return TEXT("");
    }
    return FString::Printf(TEXT(" data-%s=\"%s\""), *AttributeName, *Content);
}

// ✅ PRESERVED METHODS: Keep existing functionality
void FHTMLDocumentBuilder::AddSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level, bool bTitleIsPreformattedHtml) const
{
    FString SafeTitle = bTitleIsPreformattedHtml ? Title : FMarkdownSpanSystem::EscapeHtml(Title);
    int32 ClampedLevel = FMath::Clamp(Level, 1, 6);
    FString HeaderTag = FString::Printf(TEXT("h%d"), ClampedLevel);
    OutLines.Add(FString::Printf(TEXT("<%s>%s</%s>"), *HeaderTag, *SafeTitle, *HeaderTag));
}

void FHTMLDocumentBuilder::AddSectionSeparator(TArray<FString>& OutLines, bool bIsMajor) const
{
    FString CssClass = bIsMajor ? TEXT("major-section-separator") : TEXT("section-separator");
    if (OutLines.IsEmpty() || 
        (!OutLines.Last().Contains(TEXT("major-section-separator")) && 
         (bIsMajor || !OutLines.Last().Contains(TEXT("section-separator")))))
    {
        OutLines.Add(FString::Printf(TEXT("<hr class=\"%s\">"), *CssClass));
    }
}

void FHTMLDocumentBuilder::AddTraceStartHeader(TArray<FString>& OutLines, const FString& TraceName, const FString& NodeType) const
{
    FString TraceId = FMarkdownPathTracer::SanitizeAnchorName(TraceName);
    FString HeaderContent = FString::Printf(TEXT("Trace Start: %s"), *FMarkdownSpanSystem::EscapeHtml(TraceName));
    
    if (!NodeType.IsEmpty())
    {
        HeaderContent += FString::Printf(TEXT(" <span class=\"node-type-hint\">(%s)</span>"), *FMarkdownSpanSystem::EscapeHtml(NodeType));
    }
    
    OutLines.Add(FString::Printf(TEXT("<h3 class=\"trace-start\" id=\"trace-%s\">%s</h3>"), *TraceId, *HeaderContent));
}

void FHTMLDocumentBuilder::BeginInteractiveCodeBlock(TArray<FString>& OutLines, const FString& Title, const FString& Language) const
{
    OutLines.Add(TEXT("<div class=\"enhanced-code-block\">"));
    if (!Title.IsEmpty())
    {
        OutLines.Add(FString::Printf(TEXT("<div class=\"code-header\">%s</div>"), *FMarkdownSpanSystem::EscapeHtml(Title)));
    }
    OutLines.Add(TEXT("<pre class=\"blueprint\"><code class=\"nohighlight blueprint-code\">"));
}

void FHTMLDocumentBuilder::EndInteractiveCodeBlock(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("</code></pre>"));
    OutLines.Add(TEXT("</div>"));
}

void FHTMLDocumentBuilder::AddCollapsibleSectionHeader(TArray<FString>& OutLines, const FString& Title, int32 Level, const FString& SectionId) const
{
    OutLines.Add(FString::Printf(TEXT("<input type=\"checkbox\" id=\"toggle-%s\" class=\"collapse-toggle\">"), *SectionId));
    OutLines.Add(FString::Printf(TEXT("<label for=\"toggle-%s\" class=\"collapsible-header\">"), *SectionId));
    int32 ClampedLevel = FMath::Clamp(Level, 1, 6);
    FString HeaderTag = FString::Printf(TEXT("h%d"), ClampedLevel);
    OutLines.Add(FString::Printf(TEXT("<%s>%s</%s>"), *HeaderTag, *FMarkdownSpanSystem::EscapeHtml(Title), *HeaderTag));
    OutLines.Add(TEXT("</label>"));
}

void FHTMLDocumentBuilder::BeginCollapsibleContent(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("<div class=\"collapsible-content\">"));
}

void FHTMLDocumentBuilder::EndCollapsibleContent(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("</div>"));
}

void FHTMLDocumentBuilder::AddParagraph(TArray<FString>& OutLines, const FString& Text, bool bTextIsPreformattedHtml) const
{
    FString SafeText = bTextIsPreformattedHtml ? Text : FMarkdownSpanSystem::EscapeHtml(Text);
    OutLines.Add(FString::Printf(TEXT("<p>%s</p>"), *SafeText));
}

void FHTMLDocumentBuilder::BeginUnorderedList(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("<ul>"));
}

void FHTMLDocumentBuilder::EndUnorderedList(TArray<FString>& OutLines) const
{
    OutLines.Add(TEXT("</ul>"));
}

void FHTMLDocumentBuilder::AddListItem(TArray<FString>& OutLines, const FString& ItemContent) const
{
    OutLines.Add(FString::Printf(TEXT("<li>%s</li>"), *ItemContent));
}

FString FHTMLDocumentBuilder::WrapInCompleteHTMLDocument(const FString& BodyContent) const
{
    FString HTMLDocument = TEXT(R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Flow Inspector</title>
    <style>
)");

    // ✅ FIX: Only use modern CSS, remove legacy interactive features
    HTMLDocument += FBlueprintMarkdownCSS::GetModernThemeCSS();
    
    HTMLDocument += TEXT(R"(
    </style>
</head>
<body>
)");

    // ✅ FIX: Remove .blueprint-content wrapper that breaks grid layout
    HTMLDocument += BodyContent;
    
    HTMLDocument += TEXT(R"(
    
    <script>
)");

    // ✅ FIX: Load only modern JavaScript, no legacy search overlay
    HTMLDocument += GetModernInteractiveJavaScript();
    
    HTMLDocument += TEXT(R"(
    </script>
</body>
</html>)");

    return HTMLDocument;
}


void FHTMLDocumentBuilder::GenerateAndPrependTOC(TArray<FString>& AllOutputLines, const FTracingResults& TracingData) const
{
    if (TracingData.CollectedTraceHeaders.IsEmpty() && TracingData.GraphDefinitions.IsEmpty())
    {
        return;
    }
    
    TArray<FString> TOCLines;
    TOCLines.Add(TEXT("<div class=\"toc-container\">"));
    TOCLines.Add(TEXT("<h2>Contents</h2>"));
    
    if (!TracingData.CollectedTraceHeaders.IsEmpty()) 
    {
        TOCLines.Add(TEXT("<div class=\"toc-section\">"));
        TOCLines.Add(TEXT("<h3>Execution Traces</h3>"));
        TOCLines.Add(TEXT("<ul>"));
        for (const FString& Header : TracingData.CollectedTraceHeaders) 
        {
            FString CleanHeader = Header.Replace(TEXT("Trace Start: "), TEXT(""));
            FString Anchor = FMarkdownPathTracer::SanitizeAnchorName(CleanHeader);
            TOCLines.Add(FString::Printf(TEXT("<li><a href=\"#trace-%s\" class=\"toc-link\">%s</a></li>"), 
                *Anchor, *FMarkdownSpanSystem::EscapeHtml(CleanHeader)));
        }
        TOCLines.Add(TEXT("</ul>"));
        TOCLines.Add(TEXT("</div>"));
    }
    
    if (!TracingData.GraphDefinitions.IsEmpty())
    {
        TOCLines.Add(TEXT("<div class=\"toc-section\">"));
        TOCLines.Add(TEXT("<h3>Graph Definitions</h3>"));
        
        TMap<FString, TArray<FGraphDefinitionEntry>> DefinitionsByCategory;
        for (const FGraphDefinitionEntry& GraphDef : TracingData.GraphDefinitions)
        {
            if (IsCategoryVisible(GraphDef.Category))
            {
                DefinitionsByCategory.FindOrAdd(GraphDef.Category).Add(GraphDef);
            }
        }
        
        TArray<FString> ProcessedCategories;
        for (const FSectionMetadata& Section : TracingData.SectionHeaders)
        {
            if (Section.bIsMajorSection) continue;
            
            FString CategoryName = Section.SectionName;
            if (!IsCategoryVisible(CategoryName)) continue;
            if (ProcessedCategories.Contains(CategoryName) || !DefinitionsByCategory.Contains(CategoryName))
                continue;
            
            TArray<FGraphDefinitionEntry>& CategoryDefinitions = DefinitionsByCategory[CategoryName];
            if (CategoryDefinitions.IsEmpty()) continue;
            
            TOCLines.Add(FString::Printf(TEXT("<h4 class=\"toc-category\">%s</h4>"), *FMarkdownSpanSystem::EscapeHtml(CategoryName)));
            TOCLines.Add(TEXT("<ul>"));
            
            CategoryDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                return A.GraphName < B.GraphName;
            });
            
            for (const FGraphDefinitionEntry& GraphDef : CategoryDefinitions)
            {
                FString Anchor = FMarkdownPathTracer::SanitizeAnchorName(GraphDef.GraphName);
                FString CleanGraphName = FMarkdownSpanSystem::CleanTextForHTML(GraphDef.GraphName);
                TOCLines.Add(FString::Printf(TEXT("<li><a href=\"#%s\" class=\"toc-link toc-subitem\">%s</a></li>"), 
                    *Anchor, *FMarkdownSpanSystem::EscapeHtml(CleanGraphName)));
            }
            
            TOCLines.Add(TEXT("</ul>"));
            ProcessedCategories.Add(CategoryName);
        }
        
        TOCLines.Add(TEXT("</div>"));
    }
    
    TOCLines.Add(TEXT("</div>"));
    
    for (int32 i = TOCLines.Num() - 1; i >= 0; i--)
    {
        AllOutputLines.Insert(TOCLines[i], 0);
    }
}

bool FHTMLDocumentBuilder::IsCategoryVisible(const FString& CategoryString) const
{
    if (!CurrentSettings)
    {
        return true;
    }
    return CategoryUtils::IsCategoryStringVisible(CategoryString, *CurrentSettings);
}
/*
FString FHTMLDocumentBuilder::ProcessExecutionLineForLinks(const FString& FlowLine)
{
    // ✅ CLEAR DEBUG: Log every call to understand what's happening
    UE_LOG(LogBP2AI, Error, TEXT("🔍 ProcessExecutionLineForLinks CALLED"));
    UE_LOG(LogBP2AI, Error, TEXT("  PerformanceMode: %s"), bEnablePerformanceMode ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogBP2AI, Error, TEXT("  Input: '%s'"), *FlowLine);

    if (bEnablePerformanceMode)
    {
        UE_LOG(LogBP2AI, Error, TEXT("  ⚡ PERFORMANCE MODE: Skipping ALL regex processing"));
        UE_LOG(LogBP2AI, Error, TEXT("  Output: '%s' (unchanged)"), *FlowLine);
        return FlowLine; // Skip ALL regex processing
    }
    
    UE_LOG(LogBP2AI, Error, TEXT("  🐌 FULL MODE: Processing with regex..."));
    
    FString ProcessedLine = FlowLine;

    if (bEnablePerformanceMode)
    {
        return FlowLine; // Skip ALL regex processing
    }
    
    // Process existing markdown links first: [text](#anchor)
    static const FRegexPattern ExistingLinkPattern(TEXT("\\[([^\\]]+)\\]\\(#([^\\)]+)\\)"));
    FRegexMatcher LinkMatcher(ExistingLinkPattern, ProcessedLine);

    
    TArray<FString> ProcessedSegments;
    int32 LastEnd = 0;
    
    while (LinkMatcher.FindNext())
    {
        int32 MatchStart = LinkMatcher.GetMatchBeginning();
        int32 MatchEnd = LinkMatcher.GetMatchEnding();
        
        if (MatchStart > LastEnd)
        {
            ProcessedSegments.Add(ProcessedLine.Mid(LastEnd, MatchStart - LastEnd));
        }
        
        FString LinkText = LinkMatcher.GetCaptureGroup(1);
        FString AnchorId = LinkMatcher.GetCaptureGroup(2);
        
        FString CleanLinkText = FMarkdownSpanSystem::CleanTextForHTML(LinkText);
        
        // ✅ FIX: Apply proper CSS classes for function links
        FString StyledLink = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link function-link\"><span class=\"bp-func-name\">%s</span></a>"),
            *AnchorId, *FMarkdownSpanSystem::EscapeHtml(CleanLinkText));
        
        ProcessedSegments.Add(StyledLink);
        LastEnd = MatchEnd;
    }
    
    if (LastEnd < ProcessedLine.Len())
    {
        ProcessedSegments.Add(ProcessedLine.Mid(LastEnd));
    }
    
    FString ResultLine = ProcessedSegments.IsEmpty() ? ProcessedLine : FString::Join(ProcessedSegments, TEXT(""));
    
    // Process function calls: "Call Function: FunctionName"
    static const FRegexPattern FunctionCallPattern(TEXT("(Call Function:|Custom Event:|Call Macro:)\\s*([^\\(\\[\\n<]+)"));
    FRegexMatcher FunctionMatcher(FunctionCallPattern, ResultLine);
    
    if (FunctionMatcher.FindNext())
    {
        FString CallType = FunctionMatcher.GetCaptureGroup(1);
        FString FunctionName = FunctionMatcher.GetCaptureGroup(2).TrimStartAndEnd();
        
        // Only process if not already a link
        if (!FunctionName.Contains(TEXT("<a href")) && !FunctionName.Contains(TEXT("</a>")))
        {
            FString FunctionAnchor = FMarkdownPathTracer::SanitizeAnchorName(FunctionName);
            
            // ✅ FIX: Apply proper CSS classes based on call type
            FString CSSClasses = TEXT("graph-link");
            if (CallType.Contains(TEXT("Function")))
            {
                CSSClasses += TEXT(" function-link");
            }
            else if (CallType.Contains(TEXT("Event")))
            {
                CSSClasses += TEXT(" event-link");
            }
            else if (CallType.Contains(TEXT("Macro")))
            {
                CSSClasses += TEXT(" macro-link");
            }
            
            FString StyledFunctionCall = FString::Printf(TEXT("%s <a href=\"#%s\" class=\"%s\"><span class=\"bp-func-name\">%s</span></a>"),
                *CallType, *FunctionAnchor, *CSSClasses, *FMarkdownSpanSystem::EscapeHtml(FunctionName));
            
            FString OriginalMatch = CallType + TEXT(" ") + FunctionName;
            ResultLine.ReplaceInline(*OriginalMatch, *StyledFunctionCall);
            
            UE_LOG(LogBP2AI, Verbose, TEXT("ProcessExecutionLineForLinks: Applied classes '%s' to function '%s'"), 
                *CSSClasses, *FunctionName);
        }
    }
    
    return ResultLine;
}
*/

void FHTMLDocumentBuilder::ProcessSemanticTraceEntry(const FTraceEntry& TraceEntry, TArray<FString>& OutLines)
{
    if (HTMLSemanticFormatter.IsValid())
    {
        ProcessSemanticExecutionSteps(TraceEntry.ExecutionSteps, OutLines);
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("HTMLDocumentBuilder: HTMLSemanticFormatter not initialized"));
        OutLines.Add(TEXT("<!-- Error: HTMLSemanticFormatter not initialized -->"));
    }
}

void FHTMLDocumentBuilder::ProcessSemanticExecutionSteps(const TArray<FSemanticExecutionStep>& Steps, TArray<FString>& OutLines)
{
    if (!HTMLSemanticFormatter.IsValid())
    {
        UE_LOG(LogBP2AI, Error, TEXT("HTMLDocumentBuilder: HTMLSemanticFormatter not initialized"));
        OutLines.Add(TEXT("<!-- Error: HTMLSemanticFormatter not initialized -->"));
        return;
    }

    TArray<FString> FormattedLines;
    for (const FSemanticExecutionStep& Step : Steps)
    {
        FormattedLines.Add(HTMLSemanticFormatter->FormatExecutionStep(Step));
    }
    
    FString JoinedContent = FString::Join(FormattedLines, TEXT("<br />\n"));
    OutLines.Add(JoinedContent);
}

// ✅ NEW: Modern layout generation coordinating sidebar + main content
void FHTMLDocumentBuilder::GenerateModernHTMLLayout(
    const FTracingResults& HTMLResults,
    const FTracingResults& MarkdownResults,
    TArray<FString>& OutLines)
{
    // Generate sidebar with TOC navigation
    GenerateSidebarLayout(OutLines, HTMLResults);
    
    // Generate main content area with section cards
    GenerateMainContentArea(HTMLResults, MarkdownResults, OutLines);
}

// ✅ NEW: Generate sidebar with modern TOC and search
void FHTMLDocumentBuilder::GenerateSidebarLayout(
    TArray<FString>& OutLines, 
    const FTracingResults& TracingData) const
{
    OutLines.Add(TEXT("<nav id=\"sidebar\">"));
    OutLines.Add(TEXT("    <div id=\"sidebar-header\">"));
    OutLines.Add(TEXT("        <h1 id=\"sidebar-title\">"));
    OutLines.Add(TEXT("            <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M12 2L2 7l10 5 10-5-10-5z\"></path><path d=\"M2 17l10 5 10-5\"></path><path d=\"M2 12l10 5 10-5\"></path></svg>"));
    OutLines.Add(TEXT("            <span>BP2AI</span>"));
    OutLines.Add(TEXT("        </h1>"));
    OutLines.Add(TEXT("        <div id=\"sidebar-search-container\">"));
    OutLines.Add(TEXT("            <input type=\"text\" id=\"sidebar-search-input\" placeholder=\"Search (Ctrl+F)...\">"));
    OutLines.Add(TEXT("            <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line></svg>"));
    OutLines.Add(TEXT("        </div>"));
    
    // --- NEW: HTML for Advanced Search Options and Navigation ---
    OutLines.Add(TEXT("        <div class=\"search-controls\">"));
    OutLines.Add(TEXT("            <div class=\"search-options\">"));
    OutLines.Add(TEXT("                <label title=\"Case Sensitive\"><input type=\"checkbox\" id=\"search-case-sensitive\">Aa</label>"));
    OutLines.Add(TEXT("                <label title=\"Whole Words\"><input type=\"checkbox\" id=\"search-whole-words\">\"\"</label>"));
    OutLines.Add(TEXT("            </div>"));
    OutLines.Add(TEXT("            <div id=\"search-nav-controls\" style=\"display: none;\">"));
    OutLines.Add(TEXT("                <span id=\"search-counter\">0/0</span>"));
    OutLines.Add(TEXT("                <button id=\"search-prev-btn\" title=\"Previous (Shift+Enter)\">▲</button>"));
    OutLines.Add(TEXT("                <button id=\"search-next-btn\" title=\"Next (Enter)\">▼</button>"));
    OutLines.Add(TEXT("            </div>"));
    OutLines.Add(TEXT("        </div>"));
    // --- END NEW ---
    
    OutLines.Add(TEXT("    </div>"));
    
    // TOC Navigation
    OutLines.Add(TEXT("    <div id=\"toc-nav\">"));
    
    if (!TracingData.CollectedTraceHeaders.IsEmpty()) 
    {
        OutLines.Add(TEXT("        <h3 class=\"toc-heading\">Execution Traces</h3>"));
        OutLines.Add(TEXT("        <ul class=\"toc-list\">"));
        for (const FString& Header : TracingData.CollectedTraceHeaders) 
        {
            FString CleanHeader = Header.Replace(TEXT("Trace Start: "), TEXT(""));
            FString Anchor = FMarkdownPathTracer::SanitizeAnchorName(CleanHeader);
            OutLines.Add(FString::Printf(TEXT("            <li><a href=\"#trace_%s\" class=\"toc-link\">%s</a></li>"), 
                *Anchor, *FMarkdownSpanSystem::EscapeHtml(CleanHeader)));
        }
        OutLines.Add(TEXT("        </ul>"));
    }
    
    if (!TracingData.GraphDefinitions.IsEmpty())
    {
        OutLines.Add(TEXT("        <div class=\"toc-separator\">Graph Definitions</div>"));
        
        TMap<FString, TArray<FGraphDefinitionEntry>> DefinitionsByCategory;
        for (const FGraphDefinitionEntry& GraphDef : TracingData.GraphDefinitions)
        {
            if (IsCategoryVisible(GraphDef.Category))
            {
                DefinitionsByCategory.FindOrAdd(GraphDef.Category).Add(GraphDef);
            }
        }
        
        // --- FIX: Use the authoritative CategoryUtils to prevent string mismatches ---
        const TArray<EDocumentationGraphCategory> CategoryOrder = CategoryUtils::GetRequiredCategoryOrder();
        
        for (const EDocumentationGraphCategory CategoryEnum : CategoryOrder)
        {
            // Convert enum to the canonical display string used as the key in the map
            const FString CategoryName = CategoryUtils::CategoryToDisplayString(CategoryEnum);

            if (DefinitionsByCategory.Contains(CategoryName))
            {
                const TArray<FGraphDefinitionEntry>& CategoryDefs = DefinitionsByCategory[CategoryName];
                FString CategoryCSSClass = GetCategoryTOCClass(CategoryName);
                OutLines.Add(FString::Printf(TEXT("        <h3 class=\"toc-heading %s\">%s</h3>"), *CategoryCSSClass, *CategoryName));

                OutLines.Add(TEXT("        <ul class=\"toc-list\">"));
                
                // Sort definitions alphabetically within the category for consistent display
                TArray<FGraphDefinitionEntry> SortedDefs = CategoryDefs;
                SortedDefs.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
                    return A.GraphName < B.GraphName;
                });

                for (const FGraphDefinitionEntry& GraphDef : SortedDefs)
                {
                    FString Anchor = FMarkdownPathTracer::SanitizeAnchorName(GraphDef.GraphName);
                    if (GraphDef.Category.Contains(TEXT("Collapsed"), ESearchCase::IgnoreCase))
                    {
                        UE_LOG(LogBP2AI, Error, TEXT("🔗 TOC LINK DEBUG:"));
                        UE_LOG(LogBP2AI, Error, TEXT("  GraphName: '%s'"), *GraphDef.GraphName);
                        UE_LOG(LogBP2AI, Error, TEXT("  TOC Anchor: '%s'"), *Anchor);
                        UE_LOG(LogBP2AI, Error, TEXT("  TOC Link: href=\"#%s\""), *Anchor);
                    }
                    OutLines.Add(FString::Printf(TEXT("            <li><a href=\"#%s\" class=\"toc-link\">%s</a></li>"), 
                        *Anchor, *FMarkdownSpanSystem::EscapeHtml(GraphDef.GraphName)));
                }
                
                OutLines.Add(TEXT("        </ul>"));
            }
        }
        // --- END FIX ---
    }
    
    OutLines.Add(TEXT("    </div>"));
    OutLines.Add(TEXT("</nav>"));
}

// ✅ NEW: Generate main content area with section cards
void FHTMLDocumentBuilder::GenerateMainContentArea(
    const FTracingResults& HTMLResults,
    const FTracingResults& MarkdownResults,
    TArray<FString>& OutLines)
{
    OutLines.Add(TEXT("<main id=\"main-content\">"));
    OutLines.Add(TEXT("    <div class=\"content-wrapper\">"));
    
    // Create lookup maps for markdown data (for copy functionality)
    TMap<FString, const FTraceEntry*> MarkdownTraceMap;
    for (const FTraceEntry& MTrace : MarkdownResults.ExecutionTraces)
    {
        MarkdownTraceMap.Add(MTrace.TraceName, &MTrace);
    }
    
    TMap<FString, const FGraphDefinitionEntry*> MarkdownDefMap;
    for (const FGraphDefinitionEntry& MDef : MarkdownResults.GraphDefinitions)
    {
        MarkdownDefMap.Add(MDef.GraphName, &MDef);
    }
    
    // --- Execution Traces section (unchanged) ---
    if (!HTMLResults.ExecutionTraces.IsEmpty())
    {
        OutLines.Add(TEXT("        <h3 class=\"content-heading\">Execution Traces</h3>"));
        
        for (int32 TraceIndex = 0; TraceIndex < HTMLResults.ExecutionTraces.Num(); ++TraceIndex)
        {
            const FTraceEntry& HTMLTrace = HTMLResults.ExecutionTraces[TraceIndex];
            const FTraceEntry* MarkdownTrace = MarkdownTraceMap.FindRef(HTMLTrace.TraceName);
            
            AddExecutionTraceCard(HTMLTrace, MarkdownTrace, TraceIndex, OutLines);
        }
    }
    
    // --- Graph Definitions section (REFACTORED to restore legacy logic) ---
    if (!HTMLResults.GraphDefinitions.IsEmpty())
    {
        OutLines.Add(TEXT("        <h3 class=\"content-heading\">Graph Definitions</h3>"));
        
        // 1. Group all visible definitions by their category string.
        TMap<FString, TArray<FGraphDefinitionEntry>> DefinitionsByCategory;
        for (const FGraphDefinitionEntry& GraphDef : HTMLResults.GraphDefinitions)
        {
            if (IsCategoryVisible(GraphDef.Category))
            {
                DefinitionsByCategory.FindOrAdd(GraphDef.Category).Add(GraphDef);
            }
        }

        // 2. Iterate through the official category order provided by the FTracingResults.
        // This ensures the main content order perfectly matches the TOC order.
        int32 DefinitionMasterIndex = 0; // Maintain a unique index for all cards.

        for (const FSectionMetadata& Section : HTMLResults.SectionHeaders)
        {
            // We are only interested in the definition categories, not major sections like "Execution Traces".
            if (Section.Level != 3) continue;

            const FString& CategoryName = Section.SectionName;
            
            // 3. Render the cards for this category.
            AddDefinitionCardsForCategory(CategoryName, DefinitionsByCategory, MarkdownDefMap, DefinitionMasterIndex, OutLines);
        }
    }
    
    OutLines.Add(TEXT("    </div>"));
    OutLines.Add(TEXT("</main>"));
}


void FHTMLDocumentBuilder::AddExecutionTraceCard(
    const FTraceEntry& HTMLTrace,
    const FTraceEntry* MarkdownTrace,
    int32 TraceIndex,
    TArray<FString>& OutLines)
{
    FString TraceId = FString::Printf(TEXT("trace_%s"), *FMarkdownPathTracer::SanitizeAnchorName(HTMLTrace.TraceName));
    
    OutLines.Add(FString::Printf(TEXT("        <details class=\"content-section\" id=\"%s\" open>"), *TraceId));
    OutLines.Add(TEXT("            <summary class=\"section-header\">"));
    OutLines.Add(TEXT("                <div class=\"section-header-main\">"));
    OutLines.Add(TEXT("                    <span class=\"section-tag trace\">TRACE</span>"));
    OutLines.Add(FString::Printf(TEXT("                    <h2 class=\"section-title\">%s</h2>"), 
        *FMarkdownSpanSystem::EscapeHtml(HTMLTrace.TraceName)));
    OutLines.Add(TEXT("                </div>"));
    OutLines.Add(TEXT("                <div class=\"section-controls\">"));
    OutLines.Add(TEXT("                    <button class=\"copy-section-btn\" data-copy-target=\"trace\" title=\"Copy trace as Markdown\">"));
    OutLines.Add(TEXT("                        <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"9\" y=\"9\" width=\"13\" height=\"13\" rx=\"2\" ry=\"2\"></rect><path d=\"M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1\"></path></svg>"));
    OutLines.Add(TEXT("                        <span>Copy Trace</span>"));
    OutLines.Add(TEXT("                    </button>"));
    OutLines.Add(TEXT("                    <svg class=\"chevron\" xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><polyline points=\"9 18 15 12 9 6\"></polyline></svg>"));
    OutLines.Add(TEXT("                </div>"));
    OutLines.Add(TEXT("            </summary>"));
    OutLines.Add(TEXT("            <div class=\"section-body\">"));
    OutLines.Add(TEXT("                <h4>Execution Flow</h4>"));
    OutLines.Add(TEXT("                <div class=\"code-container\">"));
    
    // ✅ RESTORED: Legacy approach with <br /> tags for proper HTML display
    OutLines.Add(TEXT("                    <pre><code class=\"language-blueprint\">"));
    if (!HTMLTrace.ExecutionLines.IsEmpty())
    {
        // Links are now generated upstream. Simply join the pre-formatted lines.
        FString JoinedContent = FString::Join(HTMLTrace.ExecutionLines, TEXT("<br />\n"));
        OutLines.Add(JoinedContent);
    }
    else
    {
        OutLines.Add(TEXT("No execution flow available"));
    }
    OutLines.Add(TEXT("                    </code></pre>"));
    
    OutLines.Add(TEXT("                    <button class=\"copy-btn\">Copy Raw</button>"));
    OutLines.Add(TEXT("                </div>"));
    OutLines.Add(TEXT("            </div>"));
    OutLines.Add(TEXT("        </details>"));
}

// ✅ CORRECTED: Graph definition card with legacy <br /> approach  
void FHTMLDocumentBuilder::AddGraphDefinitionCard(
    const FGraphDefinitionEntry& HTMLGraphDef,
    const FGraphDefinitionEntry* MarkdownGraphDef,
    int32 DefIndex,
    TArray<FString>& OutLines)
{
    FString DefId = FMarkdownPathTracer::SanitizeAnchorName(HTMLGraphDef.GraphName);
    FString ElementType = DetermineElementType(HTMLGraphDef.Category);


    if (HTMLGraphDef.Category.Contains(TEXT("Collapsed"), ESearchCase::IgnoreCase)) 
    {
        UE_LOG(LogBP2AI, Error, TEXT("🔍 COLLAPSED GRAPH DEBUG:"));
        UE_LOG(LogBP2AI, Error, TEXT("  Original GraphName: '%s'"), *HTMLGraphDef.GraphName);
        UE_LOG(LogBP2AI, Error, TEXT("  Generated DOM ID: '%s'"), *DefId);
        UE_LOG(LogBP2AI, Error, TEXT("  Category: '%s'"), *HTMLGraphDef.Category);
        UE_LOG(LogBP2AI, Error, TEXT("  ElementType: '%s'"), *ElementType);
        
        // ✅ NEW: Also check what the TOC is generating
        FString TOCAnchor = FMarkdownPathTracer::SanitizeAnchorName(HTMLGraphDef.GraphName);
        UE_LOG(LogBP2AI, Error, TEXT("  TOC Would Generate: '%s'"), *TOCAnchor);
        
        // ✅ NEW: Check if AnchorId field exists and compare
        if (!HTMLGraphDef.AnchorId.IsEmpty())
        {
            UE_LOG(LogBP2AI, Error, TEXT("  Stored AnchorId: '%s'"), *HTMLGraphDef.AnchorId);
            if (!HTMLGraphDef.AnchorId.Equals(DefId, ESearchCase::IgnoreCase))
            {
                UE_LOG(LogBP2AI, Error, TEXT("  🚨 MISMATCH! Generated != Stored"));
            }
        }
    }
    
    OutLines.Add(FString::Printf(TEXT("        <details class=\"content-section\" id=\"%s\" open>"), *DefId));
    OutLines.Add(TEXT("            <summary class=\"section-header\">"));
    OutLines.Add(TEXT("                <div class=\"section-header-main\">"));
    OutLines.Add(FString::Printf(TEXT("                    <span class=\"section-tag %s\">%s</span>"), 
        *ElementType.ToLower(), *ElementType.ToUpper()));
    OutLines.Add(FString::Printf(TEXT("                    <h2 class=\"section-title\">%s</h2>"), 
        *FMarkdownSpanSystem::EscapeHtml(HTMLGraphDef.GraphName)));
    OutLines.Add(TEXT("                </div>"));
    OutLines.Add(TEXT("                <div class=\"section-controls\">"));
    OutLines.Add(TEXT("                    <button class=\"copy-section-btn\" data-copy-target=\"definition\" title=\"Copy definition as Markdown\">"));
    OutLines.Add(TEXT("                        <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect x=\"9\" y=\"9\" width=\"13\" height=\"13\" rx=\"2\" ry=\"2\"></rect><path d=\"M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2 2v1\"></path></svg>"));
    OutLines.Add(TEXT("                        <span>Copy Definition</span>"));
    OutLines.Add(TEXT("                    </button>"));
    OutLines.Add(TEXT("                    <svg class=\"chevron\" xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><polyline points=\"9 18 15 12 9 6\"></polyline></svg>"));
    OutLines.Add(TEXT("                </div>"));
    OutLines.Add(TEXT("            </summary>"));
    OutLines.Add(TEXT("            <div class=\"section-body\">"));
    
    // Add I/O sections
    AddModernIOSection(HTMLGraphDef, TEXT("Inputs"), true, OutLines);
    
    // ✅ RESTORED: Execution flow section with legacy <br /> approach
    if (!HTMLGraphDef.bIsPure && !HTMLGraphDef.ExecutionFlow.IsEmpty())
    {
        OutLines.Add(TEXT("                <h4>Execution Flow</h4>"));
        OutLines.Add(TEXT("                <div class=\"code-container\">"));
        OutLines.Add(TEXT("                    <pre><code class=\"language-blueprint\">"));
    
        // Links are now generated upstream. Simply join the pre-formatted lines.
        FString JoinedFlow = FString::Join(HTMLGraphDef.ExecutionFlow, TEXT("<br />\n"));
        OutLines.Add(JoinedFlow);
    
        OutLines.Add(TEXT("                    </code></pre>"));
        OutLines.Add(TEXT("                    <button class=\"copy-btn\">Copy Raw</button>"));
        OutLines.Add(TEXT("                </div>"));
    }
    else if (!HTMLGraphDef.bIsPure)
    {
        OutLines.Add(TEXT("                <h4>Execution Flow</h4>"));
        OutLines.Add(TEXT("                <div class=\"code-container\">"));
        OutLines.Add(TEXT("                    <pre><code class=\"language-blueprint\">No valid entry node found for execution flow</code></pre>"));
        OutLines.Add(TEXT("                    <button class=\"copy-btn\">Copy Raw</button>"));
        OutLines.Add(TEXT("                </div>"));
    }
    
    AddModernIOSection(HTMLGraphDef, TEXT("Outputs"), false, OutLines);
    
    OutLines.Add(TEXT("            </div>"));
    OutLines.Add(TEXT("        </details>"));
}

// ✅ NEW: Utility methods for modern layout
FString FHTMLDocumentBuilder::DetermineElementType(const FString& Category) const
{
    if (Category.Contains(TEXT("Function")))
    {
        return Category.Contains(TEXT("Pure")) ? TEXT("pure-function") : TEXT("function");
    }
    else if (Category.Contains(TEXT("Custom Event")))
    {
        return TEXT("custom-event");
    }
    else if (Category.Contains(TEXT("Collapsed")))
    {
        return TEXT("collapsed-graph");
    }
    else if (Category.Contains(TEXT("Macro")))
    {
        return Category.Contains(TEXT("Pure")) ? TEXT("pure-macro") : TEXT("executable-macro");
    }
    else if (Category.Contains(TEXT("Interface")))
    {
        return TEXT("interface");
    }
    
    return TEXT("function"); // Default fallback
}

void FHTMLDocumentBuilder::AddModernIOSection(
    const FGraphDefinitionEntry& GraphDef,
    const FString& SectionTitle,
    bool bIsInputs,
    TArray<FString>& OutLines)
{
    OutLines.Add(FString::Printf(TEXT("                <h4>%s</h4>"), *SectionTitle));
    
    const TArray<FString>& Specs = bIsInputs ? GraphDef.InputSpecs : GraphDef.OutputSpecs;
    
    if (Specs.IsEmpty())
    {
        FString EmptyMessage = bIsInputs ? 
            TEXT("*(No distinct data inputs)*") : 
            TEXT("*(No distinct data outputs)*");
        OutLines.Add(FString::Printf(TEXT("                <p><span class=\"bp-modifier\">%s</span></p>"), *EmptyMessage));
        return;
    }
    
    OutLines.Add(TEXT("                <div class=\"io-list\">"));
    
    for (const FString& Spec : Specs)
    {
        FString NameAndTypePart = Spec;
        FString ValuePart = TEXT("");

        // Reliably split the spec string into the "name/type" part and the "value" part.
        // The Split function handles cases where " = " is not found (e.g., for inputs).
        Spec.Split(TEXT(" = "), &NameAndTypePart, &ValuePart);

        // Clean up the name/type part by removing the leading list marker "* "
        NameAndTypePart.TrimStartInline();
        if (NameAndTypePart.StartsWith(TEXT("* ")))
        {
            NameAndTypePart.RightChopInline(2);
        }

        // Now, parse the type info from the first part of the string
        FBlueprintTypeInfo TypeInfo = ParseTypeFromIOSpec(NameAndTypePart);
        
        // Extract the pin name, which is whatever is left in the NameAndTypePart
        // after the type has been accounted for. This is simpler than complex regex.
        FString NameOnly;
        int32 ParenPos = NameAndTypePart.Find(TEXT("("));
        if (ParenPos != INDEX_NONE)
        {
            NameOnly = NameAndTypePart.Left(ParenPos).TrimStartAndEnd();
        }
        else // Fallback if no parentheses
        {
             // If format is `type` `name`, find first two backticked words
            TArray<FString> Words;
            NameAndTypePart.ParseIntoArray(Words, TEXT("`"));
            if(Words.Num() >= 2) NameOnly = FString::Printf(TEXT("`%s`"), *Words[1]);
            else NameOnly = NameAndTypePart;
        }


        OutLines.Add(TEXT("                    <div class=\"io-item\">"));
        
        // Generate the type badge using the parsed info
        FString TypeCSSClasses = FString::Printf(TEXT("io-type %s"), *TypeInfo.CSSClasses);
        OutLines.Add(FString::Printf(TEXT("                        <span class=\"%s\">%s</span>"), 
            *TypeCSSClasses, *FMarkdownSpanSystem::EscapeHtml(TypeInfo.DisplayName)));
        
        // Combine the name and value parts for display
        FString DisplayNameAndValue = NameOnly;
        if (!ValuePart.IsEmpty())
        {
            DisplayNameAndValue += TEXT(" = ") + ValuePart;
        }
        
        OutLines.Add(FString::Printf(TEXT("                        <span class=\"io-name\">%s</span>"), *DisplayNameAndValue));
        OutLines.Add(TEXT("                    </div>"));
    }
    
    OutLines.Add(TEXT("                </div>"));
}

// ✅ NEW: Add execution flow section for graph definitions
void FHTMLDocumentBuilder::AddExecutionFlowSection(
    const FGraphDefinitionEntry& GraphDef,
    TArray<FString>& OutLines)
{
    OutLines.Add(TEXT("                <h4>Execution Flow</h4>"));
    OutLines.Add(TEXT("                <div class=\"code-container\">"));
    
    if (!GraphDef.ExecutionFlow.IsEmpty())
    {
        OutLines.Add(TEXT("                    <pre><code class=\"language-blueprint\">"));
    
        // Links are generated upstream now. Simply join the pre-formatted lines.
        // NOTE: The legacy code joined with "\n", which is fine for <pre>. Using <br />\n is also fine. Let's stick to the original here.
        FString JoinedFlow = FString::Join(GraphDef.ExecutionFlow, TEXT("\n"));
        OutLines.Add(JoinedFlow);
    
        OutLines.Add(TEXT("                    </code></pre>"));
    }
    else
    {
        OutLines.Add(TEXT("                    <pre><code class=\"language-blueprint\">No valid entry node found for execution flow</code></pre>"));
    }
    
    OutLines.Add(TEXT("                    <button class=\"copy-btn\">Copy Raw</button>"));
    OutLines.Add(TEXT("                </div>"));
}

// ✅ NEW: Parse type information from string format
FBlueprintTypeInfo FHTMLDocumentBuilder::ParseTypeFromString(const FString& TypeString) const
{
    FBlueprintTypeInfo TypeInfo;
    FString CleanType = TypeString.TrimStartAndEnd().ToLower();
    
    // Handle container types
    if (CleanType.Contains(TEXT("array<")) || CleanType.Contains(TEXT("[")))
    {
        TypeInfo.ContainerType = TEXT("Array");
        
        // Extract inner type
        static const FRegexPattern ArrayPattern(TEXT("array<([^>]+)>|\\[\\s*([^\\]]+)\\s*\\]"));
        FRegexMatcher ArrayMatcher(ArrayPattern, CleanType);
        if (ArrayMatcher.FindNext())
        {
            FString InnerType = ArrayMatcher.GetCaptureGroup(1);
            if (InnerType.IsEmpty())
            {
                InnerType = ArrayMatcher.GetCaptureGroup(2);
            }
            TypeInfo.BaseType = InnerType.TrimStartAndEnd();
        }
        else
        {
            TypeInfo.BaseType = TEXT("unknown");
        }
    }
    else if (CleanType.Contains(TEXT("set<")) || CleanType.Contains(TEXT("{")))
    {
        TypeInfo.ContainerType = TEXT("Set");
        
        // Extract inner type
        static const FRegexPattern SetPattern(TEXT("set<([^>]+)>|\\{\\s*([^\\}]+)\\s*\\}"));
        FRegexMatcher SetMatcher(SetPattern, CleanType);
        if (SetMatcher.FindNext())
        {
            FString InnerType = SetMatcher.GetCaptureGroup(1);
            if (InnerType.IsEmpty())
            {
                InnerType = SetMatcher.GetCaptureGroup(2);
            }
            TypeInfo.BaseType = InnerType.TrimStartAndEnd();
        }
        else
        {
            TypeInfo.BaseType = TEXT("unknown");
        }
    }
    else if (CleanType.Contains(TEXT("map<")))
    {
        TypeInfo.ContainerType = TEXT("Map");
        
        // Extract key and value types
        static const FRegexPattern MapPattern(TEXT("map<([^,]+),\\s*([^>]+)>"));
        FRegexMatcher MapMatcher(MapPattern, CleanType);
        if (MapMatcher.FindNext())
        {
            TypeInfo.BaseType = MapMatcher.GetCaptureGroup(1).TrimStartAndEnd();
            TypeInfo.ValueType = MapMatcher.GetCaptureGroup(2).TrimStartAndEnd();
        }
        else
        {
            TypeInfo.BaseType = TEXT("unknown");
            TypeInfo.ValueType = TEXT("unknown");
        }
    }
    else
    {
        // Simple type
        TypeInfo.BaseType = CleanType;
    }
    
    // Normalize common type names
    if (TypeInfo.BaseType == TEXT("integer"))
    {
        TypeInfo.BaseType = TEXT("int");
    }
    else if (TypeInfo.BaseType == TEXT("boolean"))
    {
        TypeInfo.BaseType = TEXT("bool");
    }
    else if (TypeInfo.BaseType == TEXT("text"))
    {
        TypeInfo.BaseType = TEXT("string");
    }
    
    // Generate display information
    TypeInfo.GenerateDisplayInfo();
    
    return TypeInfo;
}

// ✅ NEW: Create section ID from title (utility method)
FString FHTMLDocumentBuilder::CreateSectionId(const FString& Title) const
{
    return FMarkdownPathTracer::SanitizeAnchorName(Title);
}

// ✅ NEW: Modern JavaScript without legacy search overlay conflicts
// Replace the GetModernInteractiveJavaScript() method in HTMLDocumentBuilder.cpp with this:

// ✅ NEW: Modern JavaScript split into manageable chunks to avoid compiler string limits
FString FHTMLDocumentBuilder::GetModernInteractiveJavaScript() const
{
    return LoadModernUIJS();
}

// ✅ NEW: Load modern UI JavaScript from external file
FString FHTMLDocumentBuilder::LoadModernUIJS() const
{
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BP2AI"));
    if (!Plugin.IsValid())
    {
        UE_LOG(LogBP2AI, Error, TEXT("LoadModernUIJS: BP2AI plugin not found."));
        return TEXT("console.error('BP2AI Plugin not found.');");
    }

    FString JSPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources"), TEXT("modern-ui.js"));
    if (!FPaths::FileExists(JSPath))
    {
        UE_LOG(LogBP2AI, Error, TEXT("LoadModernUIJS: File does not exist at path: %s"), *JSPath);
        return TEXT("console.error('modern-ui.js file not found.');");
    }

    FString JSContent;
    if (FFileHelper::LoadFileToString(JSContent, *JSPath))
    {
        UE_LOG(LogBP2AI, Log, TEXT("LoadModernUIJS: Successfully loaded modern UI JS file (%d chars)"), JSContent.Len());
        return JSContent;
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("LoadModernUIJS: Failed to read file: %s"), *JSPath);
        return TEXT("console.error('Failed to read modern-ui.js file.');");
    }
}



FBlueprintTypeInfo FHTMLDocumentBuilder::ParseTypeFromIOSpec(const FString& IOSpec) const
{
    FBlueprintTypeInfo TypeInfo;
    FString CleanSpec = IOSpec;
    if (CleanSpec.StartsWith(TEXT("* ")))
    {
        CleanSpec = CleanSpec.RightChop(2);
    }
    CleanSpec = CleanSpec.TrimStartAndEnd();
    
    FString TypeString;
    FString NamePart = TEXT("unknown");
    
    // Extract type from parentheses: "`Name` (Type)" -> "Type"
    int32 OpenParen = CleanSpec.Find(TEXT("("));
    int32 CloseParen = CleanSpec.Find(TEXT(")"));
    
    if (OpenParen != INDEX_NONE && CloseParen != INDEX_NONE && CloseParen > OpenParen)
    {
        // Extract type from parentheses
        TypeString = CleanSpec.Mid(OpenParen + 1, CloseParen - OpenParen - 1);
        TypeString = TypeString.TrimStartAndEnd();
        
        // Extract name part
        FString BeforeParen = CleanSpec.Left(OpenParen).TrimStartAndEnd();
        // Parse name from backticks if present
        int32 FirstBacktick = BeforeParen.Find(TEXT("`"));
        int32 SecondBacktick = -1;
        if (FirstBacktick != INDEX_NONE)
        {
            SecondBacktick = BeforeParen.Find(TEXT("`"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FirstBacktick + 1);
        }
        
        if (FirstBacktick != INDEX_NONE && SecondBacktick != INDEX_NONE)
        {
            NamePart = BeforeParen.Mid(FirstBacktick + 1, SecondBacktick - FirstBacktick - 1).TrimStartAndEnd();
        }
    }
    else
    {
        // Fallback parsing for other formats
        TypeString = CleanSpec;
    }
    
    // ✅ NEW: Proper container syntax parsing
    if (TypeString.StartsWith(TEXT("Array<")) && TypeString.EndsWith(TEXT(">")))
    {
        TypeInfo.ContainerType = TEXT("array");
        TypeInfo.BaseType = TypeString.Mid(6, TypeString.Len() - 7).TrimStartAndEnd(); // Extract between "Array<" and ">"
    }
    else if (TypeString.StartsWith(TEXT("Set<")) && TypeString.EndsWith(TEXT(">")))
    {
        TypeInfo.ContainerType = TEXT("set");
        TypeInfo.BaseType = TypeString.Mid(4, TypeString.Len() - 5).TrimStartAndEnd(); // Extract between "Set<" and ">"
    }
    else if (TypeString.StartsWith(TEXT("Map<")) && TypeString.EndsWith(TEXT(">")))
    {
        TypeInfo.ContainerType = TEXT("map");
        FString InnerContent = TypeString.Mid(4, TypeString.Len() - 5).TrimStartAndEnd();
        
        // Parse "key, value" from Map<key, value>
        int32 CommaPos = InnerContent.Find(TEXT(","));
        if (CommaPos != INDEX_NONE)
        {
            TypeInfo.BaseType = InnerContent.Left(CommaPos).TrimStartAndEnd();
            TypeInfo.ValueType = InnerContent.RightChop(CommaPos + 1).TrimStartAndEnd();
        }
        else
        {
            TypeInfo.BaseType = InnerContent;
        }
    }
    else
    {
        // Simple type - no container
        TypeInfo.BaseType = NormalizeTypeName(TypeString);
        TypeInfo.ContainerType = TEXT("");
    }
    
    // Normalize the base type
    TypeInfo.BaseType = NormalizeTypeName(TypeInfo.BaseType);
    
    // Generate display information
    TypeInfo.GenerateDisplayInfo();
    
    return TypeInfo;
}

FString FHTMLDocumentBuilder::NormalizeTypeName(const FString& TypeName) const
{
    FString CleanType = TypeName.TrimStartAndEnd().ToLower();
    
    // Handle namespace prefixes (engine:datatable -> datatable)
    if (CleanType.Contains(TEXT(":")))
    {
        int32 ColonPos = CleanType.Find(TEXT(":"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        if (ColonPos != INDEX_NONE)
        {
            CleanType = CleanType.RightChop(ColonPos + 1);
        }
    }
    
    // Normalize common type variations
    if (CleanType == TEXT("integer"))
    {
        return TEXT("int");
    }
    else if (CleanType == TEXT("boolean"))
    {
        return TEXT("bool");
    }
    else if (CleanType == TEXT("text"))
    {
        return TEXT("string");
    }
    else if (CleanType == TEXT("real"))
    {
        return TEXT("float");
    }
    
    return CleanType;
}

void FHTMLDocumentBuilder::AddDefinitionCardsForCategory(
	const FString& CategoryName,
	const TMap<FString, TArray<FGraphDefinitionEntry>>& DefinitionsByCategory,
	const TMap<FString, const FGraphDefinitionEntry*>& MarkdownDefMap,
	int32& DefinitionIndex,
	TArray<FString>& OutLines)
{
	const TArray<FGraphDefinitionEntry>* Definitions = DefinitionsByCategory.Find(CategoryName);
	if (!Definitions || Definitions->IsEmpty())
	{
		return;
	}

	// Sort the definitions for this category alphabetically.
	TArray<FGraphDefinitionEntry> SortedDefinitions = *Definitions;
	SortedDefinitions.Sort([](const FGraphDefinitionEntry& A, const FGraphDefinitionEntry& B) {
		return A.GraphName < B.GraphName;
	});

	// Render a card for each definition in the sorted list.
	for (const FGraphDefinitionEntry& HTMLGraphDef : SortedDefinitions)
	{
		const FGraphDefinitionEntry* MarkdownGraphDef = MarkdownDefMap.FindRef(HTMLGraphDef.GraphName);
		AddGraphDefinitionCard(HTMLGraphDef, MarkdownGraphDef, DefinitionIndex++, OutLines);
	}
}


FString FHTMLDocumentBuilder::GetCategoryTOCClass(const FString& CategoryName) const
{
    if (CategoryName == TEXT("Functions"))
    {
        return TEXT("toc-functions");
    }
    else if (CategoryName == TEXT("Callable Custom Events"))
    {
        return TEXT("toc-custom-events");
    }
    else if (CategoryName == TEXT("Collapsed Graphs"))
    {
        return TEXT("toc-collapsed-graphs");
    }
    else if (CategoryName == TEXT("Executable Macros"))
    {
        return TEXT("toc-executable-macros");
    }
    else if (CategoryName == TEXT("Pure Macros"))
    {
        return TEXT("toc-pure-macros");
    }
    else if (CategoryName == TEXT("Interfaces"))
    {
        return TEXT("toc-interfaces");
    }
    else if (CategoryName == TEXT("Pure Functions"))
    {
        return TEXT("toc-pure-functions");
    }
    
    return TEXT(""); // Default - no additional class
}

