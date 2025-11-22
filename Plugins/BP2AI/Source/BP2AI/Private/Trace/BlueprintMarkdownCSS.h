/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/BlueprintMarkdownCSS.h

#pragma once

#include "CoreMinimal.h"

/**
 * CSS theme management for Blueprint Markdown HTML output
 * Provides styling compatible with UE5 dark theme with interactive features
 */
class BP2AI_API FBlueprintMarkdownCSS
{
public:
    /** Get the complete CSS for HTML output (main entry point) */
    static FString GetCSS();
    
    /** Get UE5 dark theme CSS (includes base + syntax) - BACKWARD COMPATIBILITY */
    static FString GetUE5DarkThemeCSS();
    
    // ✅ NEW: Modern theme CSS methods for Phase 2
    /** Get complete modern theme CSS matching target design */
    static FString GetModernThemeCSS();
    
    /** Get CSS custom properties and global theme variables */
    static FString GetThemeVariablesCSS();
    
    /** Get layout structure CSS (grid, sidebar, main content) */
    static FString GetLayoutStructureCSS();
    
    /** Get sidebar and TOC navigation CSS */
    static FString GetSidebarCSS();
    
    /** Get main content area styling CSS */
    static FString GetMainContentCSS();
    
    /** Get color-coded type badge system CSS */
    static FString GetTypeBadgeSystemCSS();
    
    /** Get enhanced syntax highlighting CSS */
    static FString GetSyntaxHighlightingCSS();
    
    /** Get custom scrollbar and responsive CSS */
    static FString GetScrollbarCSS();
    
    // ✅ LEGACY METHODS - Keep for backward compatibility
    /** Get base document styling - LEGACY */
    static FString GetBaseDocumentCSS();
    
    /** Get Blueprint syntax highlighting CSS - LEGACY */
    static FString GetBlueprintSyntaxCSS();
    
    /** Get quality of life CSS features - LEGACY */
    static FString GetQualityOfLifeCSS();

    
    static FString GetPerformanceOptimizedCSS();

    /** Get interactive features CSS (search, clipboard, collapsible) */
    static FString GetInteractiveFeatureCSS();


    static FString GetCodeBlockBaseCSS();
    static FString GetBlueprintSyntaxColorsCSS();
    static FString GetCallKeywordCSS(); 
    static FString GetLinkStylingCSS();

    
};