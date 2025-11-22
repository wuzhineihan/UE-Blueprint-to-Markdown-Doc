/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Widgets/SBlueprintExecFlowHTMLWindow.cpp

// Private/Widgets/SBlueprintExecFlowHTMLWindow.cpp
#include "Widgets/SBlueprintExecFlowHTMLWindow.h"
#include "Trace/BlueprintMarkdownCSS.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"

#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "SWebBrowser.h"         
#include "Modules/ModuleManager.h"

#include "Logging/LogMacros.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/BP2AILog.h"
#include "Trace/ExecutionFlow/ExecutionFlowGenerator.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "BlueprintEditor.h"
#include "Engine/Blueprint.h"


void SBlueprintExecFlowHTMLWindow::Construct(const FArguments& InArgs)
{
    // ✅ Initialize category visibility (all visible by default)
    CategoryVisibility.Add(EDocumentationGraphCategory::Functions, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, true);

    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("=== HTML Window Construct (UE 5.5.4) ==="));

    bool bWebBrowserAvailable = FModuleManager::Get().IsModuleLoaded(TEXT("WebBrowser"));
    if (!bWebBrowserAvailable)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("WebBrowser module not loaded. Attempting to load..."));
        if (FModuleManager::Get().ModuleExists(TEXT("WebBrowser")))
        {
            try
            {
                FModuleManager::Get().LoadModule(TEXT("WebBrowser"));
                bWebBrowserAvailable = FModuleManager::Get().IsModuleLoaded(TEXT("WebBrowser"));
                UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("WebBrowser module loaded successfully: %s"), bWebBrowserAvailable ? TEXT("Yes") : TEXT("No"));
            }
            catch (...)
            {
                UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("Exception caught while trying to load WebBrowser module."));
                bWebBrowserAvailable = false;
            }
        }
    }
    
    ChildSlot
[
    SNew(SVerticalBox)
    
    // ✅ COMPACT: Single-line toolbar with collapsible categories
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(4.0f)
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(6.0f)
        [
            SNew(SVerticalBox)
            
            // Main toolbar line with refresh + collapsible categories toggle
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                
                // Refresh button (smaller)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(NSLOCTEXT("BlueprintMarkdown", "Refresh", "Refresh"))
                    .ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
                    .ForegroundColor(FSlateColor::UseForeground())
                    .ContentPadding(FMargin(8, 2))
                    .OnClicked(this, &SBlueprintExecFlowHTMLWindow::OnRefreshClicked)
                ]
                
                // Category toggle button
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
                [
                    SAssignNew(CategoryToggleButton, SButton)
                    .Text(this, &SBlueprintExecFlowHTMLWindow::GetCategoryToggleText)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
                    .ForegroundColor(FSlateColor::UseForeground())
                    .ContentPadding(FMargin(8, 2))
                    .OnClicked(this, &SBlueprintExecFlowHTMLWindow::OnToggleCategoriesClicked)
                    [
                        SNew(SHorizontalBox)
                        
                        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
                        [
                            SNew(STextBlock)
                            .Text(NSLOCTEXT("BlueprintMarkdown", "Categories", "Categories"))
                            .Font(FAppStyle::GetFontStyle("SmallFont"))
                        ]
                        
                        + SHorizontalBox::Slot().AutoWidth()
                        [
                            SAssignNew(CategoryExpandIcon, STextBlock)
                            .Text(FText::FromString(TEXT("▼")))
                            .Font(FAppStyle::GetFontStyle("SmallFont"))
                            .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                        ]
                    ]
                ]
                
                // Category summary (shows active count)
                + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                [
                    SAssignNew(CategorySummaryText, STextBlock)
                    .Text(this, &SBlueprintExecFlowHTMLWindow::GetCategorySummaryText)
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
                
                + SHorizontalBox::Slot().FillWidth(1.0f)
                [
                    SNew(SSpacer)
                ]
            ]
            
            // ✅ COLLAPSIBLE: Category controls (initially collapsed)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 4, 0, 0)
            [
                SAssignNew(CategoryControlsContainer, SBorder)
                .BorderImage(FAppStyle::GetBrush("NoBorder"))
                .Visibility(this, &SBlueprintExecFlowHTMLWindow::GetCategoryControlsVisibility)
                .Padding(FMargin(0, 4, 0, 0))
                [
                    SNew(SVerticalBox)
                    
                    // Compact 4-column layout for categories
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        
                        // Column 1 - Functions
                        + SHorizontalBox::Slot().FillWidth(0.25f).Padding(0, 0, 4, 0)
                        [
                            SNew(SVerticalBox)
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(FunctionsCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::Functions)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::Functions)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Functions")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(PureFunctionsCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::PureFunctions)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::PureFunctions)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Pure Functions")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                        ]
                        
                        // Column 2 - Events & Collapsed
                        + SHorizontalBox::Slot().FillWidth(0.25f).Padding(0, 0, 4, 0)
                        [
                            SNew(SVerticalBox)
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(CustomEventsCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::CustomEvents)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::CustomEvents)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Events")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(CollapsedGraphsCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::CollapsedGraphs)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::CollapsedGraphs)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Collapsed")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                        ]
                        
                        // Column 3 - Macros
                        + SHorizontalBox::Slot().FillWidth(0.25f).Padding(0, 0, 4, 0)
                        [
                            SNew(SVerticalBox)
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(ExecutableMacrosCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::ExecutableMacros)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::ExecutableMacros)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Exec Macros")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(PureMacrosCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::PureMacros)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::PureMacros)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Pure Macros")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                        ]
                        
                        // Column 4 - Interfaces + Quick toggles
                        + SHorizontalBox::Slot().FillWidth(0.25f)
                        [
                            SNew(SVerticalBox)
                            
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 1)
                            [
                                SAssignNew(InterfacesCheckBox, SCheckBox)
                                .IsChecked(this, &SBlueprintExecFlowHTMLWindow::IsCategoryChecked, EDocumentationGraphCategory::Interfaces)
                                .OnCheckStateChanged(this, &SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::Interfaces)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(TEXT("Interfaces")))
                                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                                ]
                            ]
                            
                            // Quick toggles row
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
                            [
                                SNew(SHorizontalBox)
                                
                                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString(TEXT("All")))
                                    .ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
                                    .ContentPadding(FMargin(4, 1))
                                    .OnClicked(this, &SBlueprintExecFlowHTMLWindow::OnSelectAllCategoriesClicked)
                                ]
                                
                                + SHorizontalBox::Slot().AutoWidth()
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString(TEXT("None")))
                                    .ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
                                    .ContentPadding(FMargin(4, 1))
                                    .OnClicked(this, &SBlueprintExecFlowHTMLWindow::OnSelectNoneCategoriesClicked)
                                ]
                            ]
                        ]
                    ]
                ]
            ]
        ]
    ]

    // Web Browser Content (keep existing)
    + SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f)
    [
        SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder")).Padding(4.0f)
        [
            bWebBrowserAvailable ?
            StaticCastSharedRef<SWidget>(SAssignNew(WebBrowser, SWebBrowser).InitialURL(TEXT("about:blank")).ShowControls(false).ShowAddressBar(false).SupportsTransparency(false)) :
            StaticCastSharedRef<SWidget>(SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(FallbackTextBlock, STextBlock).Text(NSLOCTEXT("BlueprintMarkdown", "WebBrowserNotAvailable", "WebBrowser not available.\nContent will be shown as text.")).AutoWrapText(true).ColorAndOpacity(FSlateColor(FLinearColor::Red))])
        ]
    ]
];
}


// ✅ PERFORMANCE DEBUGGING METHODS

double SBlueprintExecFlowHTMLWindow::GetCurrentTimeSeconds() const
{
    return FPlatformTime::Seconds();
}

int32 SBlueprintExecFlowHTMLWindow::CountSubstringOccurrences(const FString& FullString, const FString& Substring) const
{
    if (Substring.IsEmpty()) return 0;
    
    int32 Count = 0;
    int32 SearchStartPos = 0;
    
    while (SearchStartPos < FullString.Len())
    {
        int32 FoundPos = FullString.Find(Substring, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchStartPos);
        if (FoundPos == INDEX_NONE)
        {
            break;
        }
        Count++;
        SearchStartPos = FoundPos + Substring.Len();
    }
    
    return Count;
}

void SBlueprintExecFlowHTMLWindow::AnalyzeHTMLContent(const FString& HTMLContent)
{
    // ✅ UE5.5 Compatible substring counting
    int32 StyleCount = CountSubstringOccurrences(HTMLContent, TEXT("<style"));
    int32 ScriptCount = CountSubstringOccurrences(HTMLContent, TEXT("<script"));
    int32 DivCount = CountSubstringOccurrences(HTMLContent, TEXT("<div"));
    int32 SpanCount = CountSubstringOccurrences(HTMLContent, TEXT("<span"));
    int32 ClassCount = CountSubstringOccurrences(HTMLContent, TEXT("class="));
    
    // Count expensive CSS features using UE5.5 compatible method
    int32 GradientCount = CountSubstringOccurrences(HTMLContent, TEXT("gradient("));
    int32 TransitionCount = CountSubstringOccurrences(HTMLContent, TEXT("transition:"));
    int32 TransformCount = CountSubstringOccurrences(HTMLContent, TEXT("transform:"));
    int32 BoxShadowCount = CountSubstringOccurrences(HTMLContent, TEXT("box-shadow:"));
    int32 BackdropFilterCount = CountSubstringOccurrences(HTMLContent, TEXT("backdrop-filter:"));
    
    TArray<FString> Lines;
    HTMLContent.ParseIntoArrayLines(Lines);
    
    PerfMetrics.JavaScriptLineCount = 0;
    PerfMetrics.CSSRuleCount = 0;
    
    bool bInStyle = false;
    bool bInScript = false;
    
    for (const FString& Line : Lines)
    {
        if (Line.Contains(TEXT("<style"))) bInStyle = true;
        if (Line.Contains(TEXT("</style>"))) bInStyle = false;
        if (Line.Contains(TEXT("<script"))) bInScript = true;
        if (Line.Contains(TEXT("</script>"))) bInScript = false;
        
        if (bInStyle && Line.Contains(TEXT("{"))) PerfMetrics.CSSRuleCount++;
        if (bInScript && !Line.TrimStartAndEnd().IsEmpty()) PerfMetrics.JavaScriptLineCount++;
    }
    
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("🎨 CSS ANALYSIS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  CSS Rules: ~%d"), PerfMetrics.CSSRuleCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Gradients: %d (EXPENSIVE)"), GradientCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Transitions: %d"), TransitionCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Transforms: %d"), TransformCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Box Shadows: %d"), BoxShadowCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Backdrop Filters: %d (VERY EXPENSIVE)"), BackdropFilterCount);
    
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("🏗️ DOM ANALYSIS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  <div> elements: %d"), DivCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  <span> elements: %d"), SpanCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  class attributes: %d"), ClassCount);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  JavaScript lines: %d"), PerfMetrics.JavaScriptLineCount);
    
    // Estimate complexity score
    int32 ComplexityScore = (GradientCount * 10) + (BackdropFilterCount * 20) + (BoxShadowCount * 2) + 
                           (TransitionCount * 1) + (DivCount * 1) + (SpanCount * 1);
    
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("⚡ COMPLEXITY SCORE: %d %s"), ComplexityScore,
        ComplexityScore > 1000 ? TEXT("(VERY HIGH - EXPECT LAG)") : 
        ComplexityScore > 500 ? TEXT("(HIGH - POSSIBLE LAG)") : 
        ComplexityScore > 200 ? TEXT("(MODERATE)") : TEXT("(LOW)"));
}

void SBlueprintExecFlowHTMLWindow::AnalyzeCurrentCSSPerformance()
{
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("🔍 ANALYZING YOUR SPECIFIC CSS PERFORMANCE ISSUES:"));
    
    // Get your current CSS
    FString CurrentCSS = FBlueprintMarkdownCSS::GetModernThemeCSS();
    
    // Count specific performance killers using UE5.5 compatible method
    int32 BackdropFilterCount = CountSubstringOccurrences(CurrentCSS, TEXT("backdrop-filter:"));
    int32 BoxShadowCount = CountSubstringOccurrences(CurrentCSS, TEXT("box-shadow:"));
    int32 GradientCount = CountSubstringOccurrences(CurrentCSS, TEXT("gradient("));
    int32 TransitionAllCount = CountSubstringOccurrences(CurrentCSS, TEXT("transition:")) + 
                              CountSubstringOccurrences(CurrentCSS, TEXT("transition-"));
    int32 TransformCount = CountSubstringOccurrences(CurrentCSS, TEXT("transform:"));
    int32 GridLayoutCount = CountSubstringOccurrences(CurrentCSS, TEXT("display: grid")) + 
                           CountSubstringOccurrences(CurrentCSS, TEXT("grid-template"));
    
    TArray<FString> Lines;
    CurrentCSS.ParseIntoArrayLines(Lines);
    
    TArray<FString> PerformanceWarnings;
    
    for (int32 i = 0; i < Lines.Num(); i++)
    {
        const FString& Line = Lines[i].ToLower();
        
        // Backdrop filters (VERY EXPENSIVE)
        if (Line.Contains(TEXT("backdrop-filter:")))
        {
            PerformanceWarnings.Add(FString::Printf(TEXT("🚨 Line %d: backdrop-filter (VERY EXPENSIVE)"), i + 1));
        }
        
        // Complex gradients (EXPENSIVE)
        if (Line.Contains(TEXT("linear-gradient")) || Line.Contains(TEXT("radial-gradient")))
        {
            if (Line.Contains(TEXT("145deg")) || Line.Contains(TEXT("rgba")))
            {
                PerformanceWarnings.Add(FString::Printf(TEXT("⚠️ Line %d: Complex gradient with transparency"), i + 1));
            }
        }
        
        // Box shadows (EXPENSIVE)
        if (Line.Contains(TEXT("box-shadow:")))
        {
            if (Line.Contains(TEXT("0 0 20px")) || Line.Contains(TEXT("blur")))
            {
                PerformanceWarnings.Add(FString::Printf(TEXT("⚠️ Line %d: Large blur box-shadow"), i + 1));
            }
        }
        
        // Transition: all (EXPENSIVE)
        if (Line.Contains(TEXT("transition:")) && Line.Contains(TEXT("all")))
        {
            PerformanceWarnings.Add(FString::Printf(TEXT("⚠️ Line %d: 'transition: all' - should be specific properties"), i + 1));
        }
    }
    
    // Calculate performance impact score
    int32 PerformanceImpact = (BackdropFilterCount * 50) + (BoxShadowCount * 10) + 
                             (GradientCount * 8) + (TransitionAllCount * 5) + 
                             (TransformCount * 3) + (GridLayoutCount * 2);
    
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("📊 CSS PERFORMANCE ANALYSIS RESULTS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Backdrop Filters: %d (×50 = %d impact)"), BackdropFilterCount, BackdropFilterCount * 50);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Box Shadows: %d (×10 = %d impact)"), BoxShadowCount, BoxShadowCount * 10);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Gradients: %d (×8 = %d impact)"), GradientCount, GradientCount * 8);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Transitions: %d (×5 = %d impact)"), TransitionAllCount, TransitionAllCount * 5);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Transforms: %d (×3 = %d impact)"), TransformCount, TransformCount * 3);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  Grid Layouts: %d (×2 = %d impact)"), GridLayoutCount, GridLayoutCount * 2);
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT(""));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("🎯 TOTAL PERFORMANCE IMPACT SCORE: %d"), PerformanceImpact);
    
    if (PerformanceImpact > 200)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("💥 CRITICAL: Very high performance impact! Expected severe lag."));
    }
    else if (PerformanceImpact > 100)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("⚠️ WARNING: High performance impact. Likely causing lag."));
    }
    else if (PerformanceImpact > 50)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("⚡ MODERATE: Some performance impact. May cause minor lag."));
    }
    else
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("✅ GOOD: Low performance impact expected."));
    }
    
    // Log specific warnings
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT(""));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("🔧 SPECIFIC PERFORMANCE ISSUES FOUND:"));
    for (const FString& Warning : PerformanceWarnings)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("  %s"), *Warning);
    }
    
    // Provide specific fix recommendations
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT(""));
    UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("💡 RECOMMENDED FIXES (in order of impact):"));
    
    if (BackdropFilterCount > 0)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("1. 🚨 REMOVE backdrop-filter effects (saves ~%d points)"), BackdropFilterCount * 50);
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("   Replace with solid background colors"));
    }
    
    if (BoxShadowCount > 5)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("2. ⚠️ REDUCE box-shadow usage (saves ~%d points)"), (BoxShadowCount - 5) * 10);
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("   Use borders instead of shadows where possible"));
    }
    
    if (GradientCount > 3)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("3. ⚠️ SIMPLIFY gradients (saves ~%d points)"), (GradientCount - 3) * 8);
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("   Use solid colors or simple 2-color gradients"));
    }
    
    if (TransitionAllCount > 0)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("4. ⚡ REPLACE 'transition: all' with specific properties (saves ~%d points)"), TransitionAllCount * 5);
        UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("   Use 'transition: opacity 0.2s' instead of 'transition: all 0.2s'"));
    }
}

void SBlueprintExecFlowHTMLWindow::EnableCEFDebugging()
{
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🔧 CEF DEBUGGING INSTRUCTIONS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("1. Restart UE with: -cefdebug=9222"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("2. Open browser to: http://localhost:9222"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("3. Find your Blueprint HTML page"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("4. Use Performance tab to profile"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("5. Check Console for JavaScript errors"));
}
/*
void SBlueprintExecFlowHTMLWindow::LoadCurrentContentWithOptimizedCSS()
{
    if (!WebBrowser.IsValid()) return;
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🚀 Loading current content with TARGETED optimized CSS..."));
    
    // Take your current HTML body content but replace the CSS with optimized version
    FString OptimizedHTML = CurrentHTMLBody;
    
    // ✅ ENHANCED: Get the performance-optimized CSS and log its characteristics
    FString OptimizedCSS = FBlueprintMarkdownCSS::GetPerformanceOptimizedCSS();
    
    // ✅ NEW: Quick analysis of the optimized CSS to verify fixes
    int32 OptimizedTransitionAllCount = CountSubstringOccurrences(OptimizedCSS, TEXT("transition:")) + 
                                       CountSubstringOccurrences(OptimizedCSS, TEXT("transition-"));
    int32 OptimizedBoxShadowCount = CountSubstringOccurrences(OptimizedCSS, TEXT("box-shadow:"));
    int32 OptimizedGradientCount = CountSubstringOccurrences(OptimizedCSS, TEXT("gradient("));
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 OPTIMIZED CSS ANALYSIS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Size: %d bytes (vs %d original)"), OptimizedCSS.Len(), CurrentHTMLBody.Len());
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Transitions: %d (should be ~10, was 22)"), OptimizedTransitionAllCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Box Shadows: %d (should be ~2, was 4)"), OptimizedBoxShadowCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Gradients: %d (should be ~0, was 1)"), OptimizedGradientCount);
    
    // If it's a complete HTML document, replace the <style> section
    if (OptimizedHTML.Contains(TEXT("<!DOCTYPE html>")))
    {
        int32 StyleStart = OptimizedHTML.Find(TEXT("<style>"));
        int32 StyleEnd = OptimizedHTML.Find(TEXT("</style>"));
        
        if (StyleStart != INDEX_NONE && StyleEnd != INDEX_NONE)
        {
            FString BeforeStyle = OptimizedHTML.Left(StyleStart + 7); // Include "<style>"
            FString AfterStyle = OptimizedHTML.Mid(StyleEnd);
            
            OptimizedHTML = BeforeStyle + OptimizedCSS + AfterStyle;
            UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("✅ Replaced <style> section with TARGETED optimized CSS"));
        }
    }
    else
    {
        // It's just body content, wrap it with optimized CSS
        FString WrappedHTML = TEXT("<!DOCTYPE html><html><head><title>Optimized Version</title><style>");
        WrappedHTML += OptimizedCSS;
        WrappedHTML += TEXT("</style></head><body>");
        WrappedHTML += OptimizedHTML;
        WrappedHTML += TEXT("</body></html>");
        OptimizedHTML = WrappedHTML;
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("✅ Wrapped content with TARGETED optimized CSS"));
    }
    
    double StartTime = GetCurrentTimeSeconds();
    WebBrowser->LoadString(OptimizedHTML, TEXT("file:///temp/current_content_TARGETED_optimized.html"));
    double LoadTime = GetCurrentTimeSeconds() - StartTime;
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("⚡ TARGETED optimized CSS loaded in %.3f ms"), LoadTime * 1000.0);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📏 TARGETED optimized HTML size: %d bytes"), OptimizedHTML.Len());
    
    // ✅ ENHANCED: Calculate predicted performance improvement
    int32 OriginalImpact = 197; // From your analysis
    int32 TransitionSavings = (22 - OptimizedTransitionAllCount) * 5;
    int32 BoxShadowSavings = (4 - OptimizedBoxShadowCount) * 10;
    int32 GradientSavings = (1 - OptimizedGradientCount) * 8;
    int32 PredictedNewImpact = OriginalImpact - TransitionSavings - BoxShadowSavings - GradientSavings;
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🎯 PERFORMANCE PREDICTION:"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Original Impact: %d"), OriginalImpact);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Transition Savings: %d (-%d transitions)"), TransitionSavings, 22 - OptimizedTransitionAllCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Box Shadow Savings: %d (-%d shadows)"), BoxShadowSavings, 4 - OptimizedBoxShadowCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Gradient Savings: %d (-%d gradients)"), GradientSavings, 1 - OptimizedGradientCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  Predicted New Impact: %d"), PredictedNewImpact);
    
    if (PredictedNewImpact < 100)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🎉 SUCCESS: Should achieve MODERATE performance level!"));
    }
    else if (PredictedNewImpact < 150)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📈 GOOD: Significant improvement expected"));
    }
    else
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("⚠️ PARTIAL: Some improvement but may need more fixes"));
    }
    
    // Compare with original
    if (PerfMetrics.LastHTMLLoadTime > 0)
    {
        double Improvement = ((PerfMetrics.LastHTMLLoadTime - LoadTime) / PerfMetrics.LastHTMLLoadTime) * 100.0;
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📈 ACTUAL Performance improvement: %.1f%% faster"), Improvement);
        
        if (Improvement > 20.0)
        {
            UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🏆 EXCELLENT: >20%% improvement achieved!"));
        }
        else if (Improvement > 10.0)
        {
            UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("✅ GOOD: >10%% improvement achieved"));
        }
        else if (Improvement > 0.0)
        {
            UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 MINOR: Some improvement detected"));
        }
        else
        {
            UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("❌ NO IMPROVEMENT: Need to investigate CSS application"));
        }
    }
    
    // ✅ FINAL: Analyze the optimized version to verify changes took effect
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🔍 VERIFICATION: Analyzing optimized content..."));
    // ✅ FIX: Analyze the optimized HTML, not the original CurrentHTMLBody
    AnalyzeHTMLContent(OptimizedHTML);  // Use OptimizedHTML instead of CurrentHTMLBody
    
    // ✅ ENHANCED: Calculate actual impact reduction
    int32 ActualTransitions = CountSubstringOccurrences(OptimizedHTML, TEXT("transition:")) + 
                             CountSubstringOccurrences(OptimizedHTML, TEXT("transition-"));
    int32 ActualBoxShadows = CountSubstringOccurrences(OptimizedHTML, TEXT("box-shadow:"));
    int32 ActualGradients = CountSubstringOccurrences(OptimizedHTML, TEXT("gradient("));
    
    int32 ActualImpact = (ActualTransitions * 5) + (ActualBoxShadows * 10) + (ActualGradients * 8);
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🎯 VERIFICATION RESULTS:"));
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  ✅ Actual Transitions: %d (target was %d)"), ActualTransitions, OptimizedTransitionAllCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  ✅ Actual Box Shadows: %d (target was %d)"), ActualBoxShadows, OptimizedBoxShadowCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  ✅ Actual Gradients: %d (target was %d)"), ActualGradients, OptimizedGradientCount);
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("  ✅ Calculated Impact: %d (predicted was %d)"), ActualImpact, PredictedNewImpact);
    
    if (FMath::Abs(ActualImpact - PredictedNewImpact) < 20)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🎯 PERFECT: Verification matches prediction!"));
    }
    else
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("⚠️ DEVIATION: %d point difference from prediction"), 
            FMath::Abs(ActualImpact - PredictedNewImpact));
    }
}
*/
// ✅ EXISTING METHODS WITH PERFORMANCE MONITORING

void SBlueprintExecFlowHTMLWindow::SetHTMLContent(const FString& InGeneratedHTMLBody)
{
    double StartTime = GetCurrentTimeSeconds();
    
    CurrentHTMLBody = InGeneratedHTMLBody;
    AnalyzeHTMLContent(InGeneratedHTMLBody);
    
    PerfMetrics.LastContentSetTime = GetCurrentTimeSeconds() - StartTime;
    PerfMetrics.ContentSizeBytes = InGeneratedHTMLBody.Len() * sizeof(TCHAR);
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 SetHTMLContent: %d chars (%.2f KB) in %.3f ms"), 
        InGeneratedHTMLBody.Len(), 
        PerfMetrics.ContentSizeBytes / 1024.0f,
        PerfMetrics.LastContentSetTime * 1000.0);
    
    RefreshHTMLDisplay();
}

void SBlueprintExecFlowHTMLWindow::RefreshHTMLDisplay()
{
    double StartTime = GetCurrentTimeSeconds();
    
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🔄 RefreshHTMLDisplay: Starting performance-monitored refresh"));
    
    FString HtmlBodyToShow = this->CurrentHTMLBody; 
    if (HtmlBodyToShow.IsEmpty())
    {
        HtmlBodyToShow = TEXT("<p style=\"color: #888; padding:10px;\">No execution flow content to display.</p>");
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("RefreshHTMLDisplay: CurrentHTMLBody is empty. Using placeholder."));
    }

    FString FullHTMLDocument = GenerateFullHTMLPageStructure(HtmlBodyToShow);
    FString DummyURL = TEXT("file:///temp/bpmarkdown_perf_test.html"); 
    
    // 🔍 CRITICAL: Analyze the final HTML document
    AnalyzeHTMLContent(FullHTMLDocument);
    
    if (WebBrowser.IsValid())
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📱 WebBrowser LoadString: %d chars"), FullHTMLDocument.Len());
        
        double LoadStartTime = GetCurrentTimeSeconds();
        WebBrowser->LoadString(FullHTMLDocument, DummyURL);
        PerfMetrics.LastHTMLLoadTime = GetCurrentTimeSeconds() - LoadStartTime;
        
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("⏱️ LoadString completed in %.3f ms"), 
            PerfMetrics.LastHTMLLoadTime * 1000.0);
    }
    else if (FallbackTextBlock.IsValid())
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📝 Using fallback text display"));
        FString FallbackDisplay = this->CurrentHTMLBody.IsEmpty() ? 
            NSLOCTEXT("BlueprintMarkdown", "NoHTMLContentExecFlow", "No execution flow HTML content available.").ToString() :
            TEXT("HTML content generated (WebBrowser view failed or unavailable).");
        FallbackTextBlock->SetText(FText::FromString(FallbackDisplay));
    }
    
    double TotalTime = GetCurrentTimeSeconds() - StartTime;
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🏁 RefreshHTMLDisplay: Total time %.3f ms"), TotalTime * 1000.0);
    
    // Log comprehensive performance metrics
    PerfMetrics.LogMetrics();
}

// ✅ ALL WRAPPER METHODS

FReply SBlueprintExecFlowHTMLWindow::RunPerformanceTest_Wrapper()
{
    RunPerformanceTest();
    return FReply::Handled();
}

FReply SBlueprintExecFlowHTMLWindow::AnalyzeCSSPerformance_Wrapper()
{
    AnalyzeCurrentCSSPerformance();
    return FReply::Handled();
}

// ✅ ALL TEST METHODS


void SBlueprintExecFlowHTMLWindow::RunPerformanceTest()
{
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("🏃 RUNNING COMPREHENSIVE PERFORMANCE TEST"));
    
    // Test 1: Minimal HTML
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 Test 1: Minimal HTML Baseline"));
 
    
    // Test 2: Current content analysis
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 Test 2: Current Content Analysis"));
    if (!CurrentHTMLBody.IsEmpty())
    {
        AnalyzeHTMLContent(CurrentHTMLBody);
    }
    
    // Test 3: CSS Performance Analysis
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 Test 3: CSS Performance Analysis"));
    AnalyzeCurrentCSSPerformance();
    
    // Test 4: Browser settings
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 Test 4: Browser Settings"));
    
    // Test 5: CEF debugging info
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("📊 Test 5: CEF Debugging"));
    EnableCEFDebugging();
    
    PerfMetrics.LogMetrics();
}


void SBlueprintExecFlowHTMLWindow::UpdateSettings(bool bInTraceAll, bool bInDefineSeparately, bool bInExpandInline, bool bInShowDefaults)
{
    bTraceAllSelectedExec = bInTraceAll;
    bDefineUserGraphsSeparately = bInDefineSeparately;
    bExpandCompositesInline = bInExpandInline;
    bShowTrivialDefaultParams = bInShowDefaults;
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window settings updated. TraceAll: %s, DefineSep: %s, ExpandInline: %s, ShowDefaults: %s"),
        bTraceAllSelectedExec ? TEXT("true") : TEXT("false"),
        bDefineUserGraphsSeparately ? TEXT("true") : TEXT("false"),
        bExpandCompositesInline ? TEXT("true") : TEXT("false"),
        bShowTrivialDefaultParams ? TEXT("true") : TEXT("false")
    );
}

void SBlueprintExecFlowHTMLWindow::UpdateSettingsWithCategories(
    bool bInTraceAll, bool bInDefineSeparately, bool bInExpandInline, bool bInShowDefaults,
    const TMap<EDocumentationGraphCategory, bool>& InCategoryVisibility)
{
    UpdateSettings(bInTraceAll, bInDefineSeparately, bInExpandInline, bInShowDefaults);
    SynchronizeCategorySettings(InCategoryVisibility);
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Settings and categories synchronized"));
}

ECheckBoxState SBlueprintExecFlowHTMLWindow::IsCategoryChecked(EDocumentationGraphCategory Category) const
{
    const bool* VisibilityPtr = CategoryVisibility.Find(Category);
    bool bIsVisible = VisibilityPtr ? *VisibilityPtr : true;
    return bIsVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SBlueprintExecFlowHTMLWindow::OnCategoryCheckboxChanged(ECheckBoxState NewState, EDocumentationGraphCategory Category)
{
    bool bNewVisibility = (NewState == ECheckBoxState::Checked);
    CategoryVisibility.Add(Category, bNewVisibility);
    
    FString CategoryName = CategoryUtils::CategoryToDisplayString(Category);
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Category '%s' visibility changed to: %s"), 
        *CategoryName, bNewVisibility ? TEXT("visible") : TEXT("hidden"));
    
    if (OnCategoryChangedCallback)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Notifying main window of category change"));
        OnCategoryChangedCallback(CategoryVisibility);
    }
    else
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("HTML Window: No callback set - category change won't trigger regeneration"));
        RefreshHTMLDisplay();
    }
}

void SBlueprintExecFlowHTMLWindow::SynchronizeCategorySettings(const TMap<EDocumentationGraphCategory, bool>& InCategoryVisibility)
{
    CategoryVisibility = InCategoryVisibility;
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Category settings synchronized from main window"));
}

FGenerationSettings SBlueprintExecFlowHTMLWindow::CreateCurrentSettings() const
{
    FGenerationSettings Settings;
    Settings.bTraceAllSelected = bTraceAllSelectedExec;
    Settings.bDefineUserGraphsSeparately = bDefineUserGraphsSeparately;
    Settings.bExpandCompositesInline = bExpandCompositesInline;
    Settings.bShowTrivialDefaultParams = bShowTrivialDefaultParams;
    Settings.bShouldTraceSymbolicallyForData = false;
    Settings.CategoryVisibility = CategoryVisibility;
    return Settings;
}

FReply SBlueprintExecFlowHTMLWindow::OnRefreshClicked()
{
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("=== HTML Window Refresh clicked - triggering independent trace ==="));
    TriggerInitialTrace(); // Use the new tracing method instead of just refreshing display
    return FReply::Handled();
}

FString SBlueprintExecFlowHTMLWindow::GenerateFullHTMLPageStructure(const FString& InHTMLBodyContent)
{
    if (InHTMLBodyContent.Contains(TEXT("<!DOCTYPE html>")) && 
        InHTMLBodyContent.Contains(TEXT("blueprint")))
    {
        UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("Incoming content is already complete interactive HTML document"));
        return InHTMLBodyContent;
    }
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("Wrapping body content in basic HTML structure"));
    
    FString CSS;
    {
        FMarkdownGenerationContext HTMLContext(FMarkdownGenerationContext::EOutputFormat::StyledHTML);
        FMarkdownContextManager ContextManager(HTMLContext);
        CSS = FBlueprintMarkdownCSS::GetCSS();
    }
    
    return FString::Printf(TEXT(R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Blueprint Execution Flow</title>
    <style>
%s
    </style>
</head>
<body>
    <div class="blueprint-content">
%s
    </div>
</body>
</html>)"), *CSS, *InHTMLBodyContent);
}

void SBlueprintExecFlowHTMLWindow::GenerateAndDisplayInteractiveHTML()
{
    UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("=== GenerateAndDisplayInteractiveHTML ==="));
    
    if (CurrentHTMLBody.IsEmpty())
    {
        FString EmptyHTML = TEXT("<p style=\"color: #ff8080; padding:10px;\">No nodes selected or content generated.</p>");
        if (WebBrowser.IsValid())
        {
            FString FullHTMLDocument = GenerateFullHTMLPageStructure(EmptyHTML);
            WebBrowser->LoadString(FullHTMLDocument, TEXT("file:///temp/bpmarkdown_empty.html"));
        }
        return;
    }

    if (WebBrowser.IsValid())
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("Loading complete interactive HTML document (Length: %d chars)"), CurrentHTMLBody.Len());
        
        if (CurrentHTMLBody.StartsWith(TEXT("<!DOCTYPE")) || CurrentHTMLBody.StartsWith(TEXT("<html")))
        {
            WebBrowser->LoadString(CurrentHTMLBody, TEXT("file:///temp/bpmarkdown_interactive.html"));
        }
        else
        {
            FString FullHTMLDocument = GenerateFullHTMLPageStructure(CurrentHTMLBody);
            WebBrowser->LoadString(FullHTMLDocument, TEXT("file:///temp/bpmarkdown_interactive.html"));
        }
    }
}



// ✅ COLLAPSIBLE: Toggle category controls visibility
FReply SBlueprintExecFlowHTMLWindow::OnToggleCategoriesClicked()
{
    bCategoryControlsExpanded = !bCategoryControlsExpanded;
    
    // Update expand icon
    if (CategoryExpandIcon.IsValid())
    {
        FString IconText = bCategoryControlsExpanded ? TEXT("▲") : TEXT("▼");
        CategoryExpandIcon->SetText(FText::FromString(IconText));
    }
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Category controls %s"), 
        bCategoryControlsExpanded ? TEXT("expanded") : TEXT("collapsed"));
    
    return FReply::Handled();
}

// ✅ BULK OPERATIONS: Select all categories
FReply SBlueprintExecFlowHTMLWindow::OnSelectAllCategoriesClicked()
{
    // Update all category visibility
    CategoryVisibility.Add(EDocumentationGraphCategory::Functions, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, true);
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: All categories selected"));
    
    // Trigger callback if set
    if (OnCategoryChangedCallback)
    {
        OnCategoryChangedCallback(CategoryVisibility);
    }
    else
    {
        RefreshHTMLDisplay();
    }
    
    return FReply::Handled();
}

// ✅ BULK OPERATIONS: Deselect all categories
FReply SBlueprintExecFlowHTMLWindow::OnSelectNoneCategoriesClicked()
{
    // Update all category visibility
    CategoryVisibility.Add(EDocumentationGraphCategory::Functions, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, false);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, false);
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: All categories deselected"));
    
    // Trigger callback if set
    if (OnCategoryChangedCallback)
    {
        OnCategoryChangedCallback(CategoryVisibility);
    }
    else
    {
        RefreshHTMLDisplay();
    }
    
    return FReply::Handled();
}

// ✅ DYNAMIC UI: Generate toggle button text
FText SBlueprintExecFlowHTMLWindow::GetCategoryToggleText() const
{
    // Not actually used since we're showing "Categories" + icon, but required for the delegate
    return NSLOCTEXT("BlueprintMarkdown", "Categories", "Categories");
}

// ✅ DYNAMIC UI: Generate category summary text
FText SBlueprintExecFlowHTMLWindow::GetCategorySummaryText() const
{
    int32 VisibleCount = 0;
    int32 TotalCount = 7; // Total number of categories
    
    // Count visible categories
    for (const auto& Pair : CategoryVisibility)
    {
        if (Pair.Value)
        {
            VisibleCount++;
        }
    }
    
    // Handle case where CategoryVisibility might be empty (all default to visible)
    if (CategoryVisibility.IsEmpty())
    {
        VisibleCount = TotalCount;
    }
    
    if (VisibleCount == TotalCount)
    {
        return NSLOCTEXT("BlueprintMarkdown", "AllCategoriesVisible", "(All visible)");
    }
    else if (VisibleCount == 0)
    {
        return NSLOCTEXT("BlueprintMarkdown", "NoCategoriesVisible", "(None visible)");
    }
    else
    {
        return FText::Format(
            NSLOCTEXT("BlueprintMarkdown", "SomeCategoriesVisible", "({0}/{1} visible)"),
            FText::AsNumber(VisibleCount),
            FText::AsNumber(TotalCount)
        );
    }
}

// ✅ DYNAMIC UI: Control category controls visibility
EVisibility SBlueprintExecFlowHTMLWindow::GetCategoryControlsVisibility() const
{
    return bCategoryControlsExpanded ? EVisibility::Visible : EVisibility::Collapsed;
}

// ✅ NEW: Methods for coordinated tracing with main window
void SBlueprintExecFlowHTMLWindow::RefreshFromMainWindow()
{
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: RefreshFromMainWindow called - triggering HTML trace"));
    TriggerInitialTrace();
}

void SBlueprintExecFlowHTMLWindow::TriggerInitialTrace()
{
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: TriggerInitialTrace - generating HTML content independently"));
    
    TArray<UEdGraphNode*> SelectedNodes = GetSelectedBlueprintNodes();
    if (SelectedNodes.Num() == 0)
    {
        UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("HTML Window: No nodes selected for tracing"));
        CurrentHTMLBody = TEXT("<p style=\"color: #ff8080; padding:10px;\">No nodes selected in Blueprint Editor.</p>");
        RefreshHTMLDisplay();
        return;
    }
    
    // Generate HTML using the execution flow generator
    FGenerationSettings Settings = CreateCurrentSettings();
    static FExecutionFlowGenerator FlowGenerator;
    
    // Generate HTML with proper context
    FMarkdownGenerationContext HTMLContext(FMarkdownGenerationContext::EOutputFormat::StyledHTML);
    FString CompleteInteractiveHTML = FlowGenerator.GenerateDocumentForNodes(SelectedNodes, Settings, HTMLContext);
    
    CurrentHTMLBody = CompleteInteractiveHTML;
    
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window: Generated %d chars of HTML content"), CompleteInteractiveHTML.Len());
    
    // Update the display
    RefreshHTMLDisplay();
}

// ✅ NEW: Blueprint node detection (copied from main window logic)
TArray<UEdGraphNode*> SBlueprintExecFlowHTMLWindow::GetSelectedBlueprintNodes() const
{
    TArray<UEdGraphNode*> SelectedNodes;
    if (!GEditor) { UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("HTML Window GetSelectedNodes: GEditor is null.")); return SelectedNodes; }
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) { UE_LOG(LogBlueprintMarkdownHTML, Warning, TEXT("HTML Window GetSelectedNodes: AssetEditorSubsystem not found.")); return SelectedNodes; }
    IAssetEditorInstance* MostRecentEditorInstance = nullptr;
    double LastActivationTime = 0.0;
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    for (UObject* Asset : EditedAssets) {
        UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
        if (!Blueprint) continue;
        IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false);
        if (EditorInstance && EditorInstance->GetEditorName() == FName("BlueprintEditor")) {
            double CurrentActivationTime = EditorInstance->GetLastActivationTime();
            if (CurrentActivationTime >= LastActivationTime) {
                MostRecentEditorInstance = EditorInstance; LastActivationTime = CurrentActivationTime;
            }
        }
    }
    if (!MostRecentEditorInstance) { UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window GetSelectedNodes: No suitable Blueprint editor instance found.")); return SelectedNodes; }
    FBlueprintEditor* TargetBlueprintEditor = static_cast<FBlueprintEditor*>(MostRecentEditorInstance);
    if (!TargetBlueprintEditor) { UE_LOG(LogBlueprintMarkdownHTML, Error, TEXT("HTML Window GetSelectedNodes: Failed to cast to FBlueprintEditor.")); return SelectedNodes; }
    FGraphPanelSelectionSet SelectedNodeSet = TargetBlueprintEditor->GetSelectedNodes();
    if (SelectedNodeSet.IsEmpty()) { UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window GetSelectedNodes: No nodes selected in the active editor.")); return SelectedNodes; }
    for (UObject* SelectedObject : SelectedNodeSet) { if (UEdGraphNode* GraphNode = Cast<UEdGraphNode>(SelectedObject)) { SelectedNodes.Add(GraphNode); } }
    UE_LOG(LogBlueprintMarkdownHTML, Log, TEXT("HTML Window GetSelectedNodes: Found %d selected nodes."), SelectedNodes.Num());
    return SelectedNodes;
}