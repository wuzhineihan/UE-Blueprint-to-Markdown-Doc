/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Widgets/SBlueprintExecFlowHTMLWindow.h

// Public/Widgets/SBlueprintExecFlowHTMLWindow.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Trace/Generation/GenerationShared.h"
#include "Widgets/Input/SCheckBox.h"

// UE 5.5.4 - Modern web browser includes
#include "IWebBrowserWindow.h"
#include "SWebBrowser.h"
#include "WebBrowserModule.h"
#include "Logging/BP2AILog.h"

class BP2AI_API SBlueprintExecFlowHTMLWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBlueprintExecFlowHTMLWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // ✅ CORE FUNCTIONALITY
    void SetHTMLContent(const FString& InGeneratedHTMLBody);
    void UpdateSettings(bool bInTraceAll, bool bInDefineSeparately, bool bInExpandInline, bool bInShowDefaults);
    void UpdateSettingsWithCategories(
        bool bInTraceAll, bool bInDefineSeparately,
        bool bInExpandInline, bool bInShowDefaults,
        const TMap<EDocumentationGraphCategory, bool>& InCategoryVisibility
    );
    void SetCategoryChangedCallback(TFunction<void(const TMap<EDocumentationGraphCategory, bool>&)> InCallback)
    {
        OnCategoryChangedCallback = InCallback;
    }
    void SetParentWindow(TWeakPtr<SWindow> InParentWindow) { ParentWindowPtr = InParentWindow; }
    void GenerateAndDisplayInteractiveHTML();
    
    // ✅ NEW: Methods for coordinated tracing with main window
    void RefreshFromMainWindow();
    void TriggerInitialTrace();

private:
    // ✅ UI COMPONENTS
    TSharedPtr<SWebBrowser> WebBrowser;
    TSharedPtr<STextBlock> FallbackTextBlock;
    
    // ✅ STATE MANAGEMENT
    FString CurrentHTMLBody;
    bool bTraceAllSelectedExec = false;
    bool bDefineUserGraphsSeparately = true;
    bool bExpandCompositesInline = false;
    bool bShowTrivialDefaultParams = false;
    bool bUseInteractiveHTML = true;
    TWeakPtr<SWindow> ParentWindowPtr;

    // ✅ CATEGORY VISIBILITY
    TMap<EDocumentationGraphCategory, bool> CategoryVisibility;
    TSharedPtr<SCheckBox> FunctionsCheckBox;
    TSharedPtr<SCheckBox> CustomEventsCheckBox;
    TSharedPtr<SCheckBox> CollapsedGraphsCheckBox;
    TSharedPtr<SCheckBox> ExecutableMacrosCheckBox;
    TSharedPtr<SCheckBox> PureMacrosCheckBox;
    TSharedPtr<SCheckBox> InterfacesCheckBox;
    TSharedPtr<SCheckBox> PureFunctionsCheckBox;
    TFunction<void(const TMap<EDocumentationGraphCategory, bool>&)> OnCategoryChangedCallback;

    // ✅ EVENT HANDLERS
    FReply OnRefreshClicked();
    ECheckBoxState IsCategoryChecked(EDocumentationGraphCategory Category) const;
    void OnCategoryCheckboxChanged(ECheckBoxState NewState, EDocumentationGraphCategory Category);
    void SynchronizeCategorySettings(const TMap<EDocumentationGraphCategory, bool>& InCategoryVisibility);
    FGenerationSettings CreateCurrentSettings() const;

    // ✅ HTML GENERATION
    FString GenerateFullHTMLPageStructure(const FString& InHTMLBodyContent);
    void RefreshHTMLDisplay();
    
    // ✅ NEW: Blueprint node detection for independent tracing
    TArray<UEdGraphNode*> GetSelectedBlueprintNodes() const;

    // ✅ PERFORMANCE METRICS SYSTEM
    struct FPerformanceMetrics
    {
        double LastHTMLLoadTime = 0.0;
        double LastContentSetTime = 0.0;
        int32 ContentSizeBytes = 0;
        int32 CSSRuleCount = 0;
        int32 JavaScriptLineCount = 0;
        
        void LogMetrics() const
        {
            UE_LOG(LogBP2AI, Warning, TEXT("=== PERFORMANCE METRICS ==="));
            UE_LOG(LogBP2AI, Warning, TEXT("HTML Load Time: %.3f ms"), LastHTMLLoadTime * 1000.0);
            UE_LOG(LogBP2AI, Warning, TEXT("Content Set Time: %.3f ms"), LastContentSetTime * 1000.0);
            UE_LOG(LogBP2AI, Warning, TEXT("Content Size: %d bytes (%.2f KB)"), ContentSizeBytes, ContentSizeBytes / 1024.0f);
            UE_LOG(LogBP2AI, Warning, TEXT("CSS Rules: ~%d"), CSSRuleCount);
            UE_LOG(LogBP2AI, Warning, TEXT("JS Lines: ~%d"), JavaScriptLineCount);
            UE_LOG(LogBP2AI, Warning, TEXT("==============================="));
        }
    };
    
    mutable FPerformanceMetrics PerfMetrics;

    // ✅ PERFORMANCE ANALYSIS METHODS
    double GetCurrentTimeSeconds() const;
    int32 CountSubstringOccurrences(const FString& FullString, const FString& Substring) const;
    void AnalyzeHTMLContent(const FString& HTMLContent);
    void AnalyzeCurrentCSSPerformance();
    void EnableCEFDebugging();

    // ✅ TEST WRAPPER METHODS - All return FReply for button handlers
    FReply LoadTinyHTML_Wrapper();
    FReply LoadSimpleTestHTML_Wrapper();
    FReply LoadDataURL_Wrapper();
    FReply LoadGoogle_Wrapper();
    FReply LogCurrentHTML_Wrapper();
    FReply LoadMinimalPreTestHTML_Wrapper();
    FReply RunPerformanceTest_Wrapper();
    FReply TestMinimalHTML_Wrapper();
    FReply AnalyzeCSSPerformance_Wrapper();

    // ✅ ACTUAL TEST METHODS
 




    void RunPerformanceTest();

    void LoadCurrentContentWithOptimizedCSS();


 
    // ✅ NEW: Compact UI components for HTML window
    TSharedPtr<SButton> CategoryToggleButton;
    TSharedPtr<STextBlock> CategoryExpandIcon;
    TSharedPtr<STextBlock> CategorySummaryText;
    TSharedPtr<SBorder> CategoryControlsContainer;

    // ✅ NEW: Collapsible state for HTML window
    bool bCategoryControlsExpanded = false;  // Start collapsed to save space

    // ✅ NEW: Compact UI event handlers for HTML window
    FReply OnToggleCategoriesClicked();
    FReply OnSelectAllCategoriesClicked();
    FReply OnSelectNoneCategoriesClicked();

    // ✅ NEW: Dynamic UI text methods for HTML window
    FText GetCategoryToggleText() const;
    FText GetCategorySummaryText() const;
    EVisibility GetCategoryControlsVisibility() const;
};