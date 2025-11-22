/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/BlueprintMarkdownCSS.cpp

// Private/Trace/BlueprintMarkdownCSS.cpp

#include "Trace/BlueprintMarkdownCSS.h"

FString FBlueprintMarkdownCSS::GetCSS()
{
    return GetModernThemeCSS(); // ✅ Removed empty GetInteractiveFeatureCSS()
}

FString FBlueprintMarkdownCSS::GetUE5DarkThemeCSS()
{
    // ✅ BACKWARD COMPATIBILITY: Keep old method for any external callers
    return GetModernThemeCSS();
}

// ✅ NEW: Modern theme CSS matching target design
FString FBlueprintMarkdownCSS::GetModernThemeCSS()
{
    return GetThemeVariablesCSS() + GetLayoutStructureCSS() + GetSidebarCSS() + 
           GetMainContentCSS() + GetTypeBadgeSystemCSS() + GetSyntaxHighlightingCSS() + 
           GetScrollbarCSS();
}

FString FBlueprintMarkdownCSS::GetThemeVariablesCSS()
{
    return TEXT(R"(
        /* =================================================================== */
        /* 1. THEME & GLOBAL STYLES - PROFESSIONAL DARK THEME
        /* =================================================================== */
        :root {
            --font-sans: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", "Roboto", sans-serif;
            --font-mono: 'JetBrains Mono', 'SF Mono', 'Cascadia Code', 'Fira Code', monospace;

            --color-bg: #111014;
            --color-bg-sidebar: #19181D;
            --color-bg-content: #212026;
            --color-bg-inset: #0D0C0F;

            --color-text-primary: #EAEAEB;
            --color-text-secondary: #A3A3A8;
            --color-text-faded: #6B6A6F;

            --color-accent: #8A63D2;
            --color-accent-light: #A78BFA;
            --color-accent-glow: rgba(138, 99, 210, 0.3);
            --color-border: rgba(255, 255, 255, 0.07);
            --color-border-hover: rgba(255, 255, 255, 0.15);

            --radius-sm: 6px;
            --radius-md: 8px;
            --radius-lg: 12px;
        }

        html {
            scroll-behavior: smooth;
        }

        body {
            font-family: var(--font-sans);
            background-color: var(--color-bg);
            color: var(--color-text-primary);
            margin: 0;
            display: grid;
            grid-template-columns: 260px 1fr;
            grid-template-rows: 100vh;
            overflow: hidden;
        }
    )");
}

FString FBlueprintMarkdownCSS::GetLayoutStructureCSS()
{
    return TEXT(R"(
        /* =================================================================== */
        /* 2. LAYOUT STRUCTURE - SIDEBAR + MAIN CONTENT GRID
        /* =================================================================== */
        
        #sidebar {
            background-color: var(--color-bg-sidebar);
            border-right: 1px solid var(--color-border);
            padding: 32px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
        }

        #main-content { 
            padding: 64px; 
            overflow-y: auto; 
        }
        
        .content-wrapper { 
    max-width: min(1400px, calc(100vw - 320px));  // Instead of fixed 800px
    margin: 0 auto; 
    width: 100%;
}

        .blueprint-content {
            /* Remove old styling - now handled by grid layout */
            max-width: none;
            margin: 0;
            padding: 0;
        }
    )");
}

// ✅ PERFORMANCE FIX: Split sidebar CSS to avoid string length limits
FString FBlueprintMarkdownCSS::GetSidebarCSS()
{
FString SidebarStyles = TEXT(R"(
    /* =================================================================== */
    /* 3. SIDEBAR & TABLE OF CONTENTS STYLING - STICKY SEARCH
    /* =================================================================== */
    
    #sidebar {
        background-color: var(--color-bg-sidebar);
        border-right: 1px solid var(--color-border);
        padding: 0;
        overflow: hidden;
        display: flex;
        flex-direction: column;
    }

    #sidebar-header {
        flex-shrink: 0;
        padding: 32px 32px 24px 32px;
        background-color: var(--color-bg-sidebar);
        border-bottom: 1px solid var(--color-border);
        position: sticky;
        top: 0;
        z-index: 10;
    }

    #toc-nav {
        flex-grow: 1;
        overflow-y: auto;
        padding: 16px 32px 32px 32px;
    }

    #sidebar-title {
        font-size: 16px;
        font-weight: 600;
        margin: 0 0 16px 0;
        display: flex;
        align-items: center;
        gap: 8px;
        color: var(--color-text-primary);
    }
    
    #sidebar-title svg {
        color: var(--color-accent);
    }

    #sidebar-search-container {
        position: relative;
        margin-bottom: 8px;
        width: 100%;
        box-sizing: border-box;
    }
    
    #sidebar-search-input {
        width: 100%;
        box-sizing: border-box;
        background: var(--color-bg-inset);
        border: 1px solid var(--color-border);
        border-radius: var(--radius-md);
        padding: 10px 14px 10px 38px;
        color: var(--color-text-primary);
        font-family: var(--font-sans);
        font-size: 14px;
    }
    
    #sidebar-search-input:focus {
        outline: none;
        border-color: var(--color-accent);
        background: var(--color-bg-content);
    }
    
    #sidebar-search-input::placeholder {
        color: var(--color-text-faded);
    }

    #sidebar-search-container svg {
        position: absolute;
        left: 12px;
        top: 50%;
        transform: translateY(-50%);
        color: var(--color-text-faded);
        pointer-events: none;
        z-index: 1;
    }

    #sidebar-search-input:focus + svg {
        color: var(--color-accent);
    }
)");
    SidebarStyles += TEXT(R"(
        /* Search Controls */
        .search-controls {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 8px;
            gap: 8px;
        }

        .search-options {
            display: flex;
            gap: 4px;
        }

        .search-options label {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            font-family: var(--font-mono);
            font-size: 11px;
            font-weight: 600;
            color: var(--color-text-faded);
            background-color: var(--color-bg-inset);
            border: 1px solid var(--color-border);
            border-radius: var(--radius-sm);
            padding: 4px 6px;
            cursor: pointer;
            user-select: none;
        }

        .search-options label:hover {
            border-color: var(--color-border-hover);
            color: var(--color-text-secondary);
        }

        .search-options input[type="checkbox"] {
            display: none;
        }
        
        .search-options label.checked {
            background-color: var(--color-accent);
            border-color: var(--color-accent);
            color: white;
        }

        #search-nav-controls {
            display: flex;
            align-items: center;
            gap: 6px;
        }

        #search-counter {
            font-family: var(--font-mono);
            font-size: 12px;
            color: var(--color-text-secondary);
            background-color: var(--color-bg-inset);
            padding: 4px 8px;
            border-radius: var(--radius-sm);
            min-width: 30px;
            text-align: center;
        }

        #search-prev-btn, #search-next-btn {
            background-color: var(--color-bg-content);
            border: 1px solid var(--color-border);
            color: var(--color-text-primary);
            width: 24px;
            height: 24px;
            border-radius: var(--radius-sm);
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 0;
            font-size: 14px;
        }

        #search-prev-btn:hover, #search-next-btn:hover {
            background-color: var(--color-accent);
            border-color: var(--color-accent);
        }

        .search-highlight.current {
            background-color: #f8a100 !important;
            color: #111 !important;
            border-radius: 3px;
        }
    )");

    SidebarStyles += TEXT(R"(
        /* TOC Navigation */
        #toc-nav {
            flex-grow: 1;
        }

        .toc-heading {
            font-size: 11px;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: var(--color-text-secondary);
            margin: 24px 0 8px 0;
            padding-bottom: 4px;
        }

        .toc-separator {
            font-size: 10px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            color: var(--color-text-faded);
            margin: 32px 0 16px 0;
            padding-bottom: 8px;
            border-bottom: 1px solid var(--color-border);
        }

        .toc-list { 
            list-style: none; 
            padding: 0; 
            margin: 0; 
        }
        
        .toc-link {
            display: block;
            color: var(--color-text-secondary);
            text-decoration: none;
            font-size: 14px;
            padding: 8px 12px;
            border-radius: var(--radius-md);
            margin-bottom: 4px;
            position: relative;
        }
        
        .toc-link::before {
            content: '';
            position: absolute;
            left: 0;
            top: 20%;
            width: 3px;
            height: 60%;
            background-color: var(--color-accent);
            border-radius: 3px;
            opacity: 0;
        }
        
        .toc-link:hover { 
            background-color: var(--color-bg-content); 
            color: var(--color-text-primary); 
        }
        
        .toc-link.active { 
            color: var(--color-text-primary); 
            font-weight: 500; 
        }
        
        .toc-link.active::before { 
            opacity: 1;
        }

        /* TOC Category Color Coding */
        .toc-heading.toc-functions { color: #61AFEF !important; }
        .toc-heading.toc-custom-events { color: #D19A66 !important; }
        .toc-heading.toc-collapsed-graphs { color: #56B6C2 !important; }
        .toc-heading.toc-executable-macros { color: #ABB2BF !important; }
        .toc-heading.toc-pure-macros { color: #ABB2BF !important; }
        .toc-heading.toc-interfaces { color: #E06C75 !important; }
        .toc-heading.toc-pure-functions { color: #56B6C2 !important; }
    )");
    
    return SidebarStyles;
}


FString FBlueprintMarkdownCSS::GetMainContentCSS()
{
    return TEXT(R"(
        /* =================================================================== */
        /* 4. MAIN CONTENT AREA STYLING
        /* =================================================================== */
        
        .content-heading {
            font-size: 12px;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            color: var(--color-text-faded);
            margin: 64px 0 16px 0;
            padding-bottom: 8px;
            border-bottom: 1px solid var(--color-border);
        }
        
        .content-heading:first-of-type { 
            margin-top: 0; 
        }

        .content-section {
            border: 1px solid var(--color-border);
            background-color: var(--color-bg-sidebar);
            border-radius: var(--radius-lg);
            margin-bottom: 32px;
        }
        
        .content-section[open] {
            border-color: var(--color-border-hover);
        }

        .section-header {
            padding: 16px 24px;
            cursor: pointer;
            list-style: none;
            display: flex;
            justify-content: space-between;
            align-items: center;
            gap: 16px;
        }
        
        .section-header::-webkit-details-marker { 
            display: none; 
        }
        
        .section-header-main {
            display: flex;
            align-items: center;
            gap: 12px;
            flex-grow: 1;
        }

        .section-title { 
            font-size: 18px; 
            font-weight: 500; 
            color: var(--color-text-primary); 
            margin: 0; 
        }

        /* Right-aligned controls */
        .section-controls {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .copy-section-btn {
            background-color: var(--color-bg-content);
            border: 1px solid var(--color-border);
            color: var(--color-text-secondary);
            font-family: var(--font-sans);
            font-size: 12px;
            padding: 6px 10px;
            border-radius: var(--radius-md);
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 6px;
            flex-shrink: 0;
        }
        
        .copy-section-btn:hover { 
            background-color: var(--color-accent); 
            border-color: var(--color-accent); 
            color: white; 
        }
        
        .copy-section-btn svg { 
            flex-shrink: 0; 
        }

        .section-header .chevron { 
            color: var(--color-text-faded);        
            flex-shrink: 0; 
        }
        
        .content-section[open] > .section-header .chevron { 
            transform: rotate(90deg); 
        }

        .section-body { 
            padding: 0 24px 24px 24px; 
            border-top: 1px solid var(--color-border); 
        }
        
        .section-body h4 {
            font-size: 12px;
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: var(--color-text-secondary);
            margin: 24px 0 12px 0;
        }
        
        .io-list { 
            display: flex; 
            flex-direction: column; 
            gap: 8px; 
        }
        
        .io-item {
            display: flex;
            align-items: center;
            gap: 12px;
            background-color: var(--color-bg-inset);
            padding: 8px 12px;
            border-radius: var(--radius-sm);
            border: 1px solid var(--color-border);
        }
        
        .io-name { 
            font-family: var(--font-mono); 
            font-size: 13px; 
            color: var(--color-text-secondary); 
            flex-grow: 1;
        }
    )");
}
// ✅ Split TypeBadgeSystemCSS to avoid string length limits
FString FBlueprintMarkdownCSS::GetTypeBadgeSystemCSS()
{
    FString BaseTypeBadges = TEXT(R"(
        /* =================================================================== */
        /* 5. TYPE BADGE SYSTEM - ACTUAL UE5 COLORS WITH REDUCED SATURATION
        /* =================================================================== */
        
        /* Section Tags - Keep original saturation for labels */
        .section-tag {
            font-size: 10px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            padding: 4px 8px;
            border-radius: 4px;
            flex-shrink: 0;
        }
        
        /* Blueprint Element Types - Keep original vibrant colors */
        .section-tag.trace {
            background-color: rgba(138, 99, 210, 0.15);
            color: #A78BFA;
            border: 1px solid rgba(138, 99, 210, 0.3);
        }
        
        .section-tag.function,
        .section-tag.pure-function {
            background-color: rgba(97, 175, 239, 0.15);
            color: #61AFEF;
            border: 1px solid rgba(97, 175, 239, 0.3);
        }
        
        .section-tag.custom-event {
            background-color: rgba(209, 154, 102, 0.15);
            color: #D19A66;
            border: 1px solid rgba(209, 154, 102, 0.3);
        }
        
        .section-tag.collapsed-graph {
            background-color: rgba(86, 182, 194, 0.15);
            color: #56B6C2;
            border: 1px solid rgba(86, 182, 194, 0.3);
        }
        
        .section-tag.executable-macro,
        .section-tag.pure-macro {
            background-color: rgba(171, 178, 191, 0.15);
            color: #ABB2BF;
            border: 1px solid rgba(171, 178, 191, 0.3);
        }
        
        .section-tag.interface {
            background-color: rgba(224, 108, 117, 0.15);
            color: #E06C75;
            border: 1px solid rgba(224, 108, 117, 0.3);
        }
)");

    FString CoreTypeBadges = TEXT(R"(
        /* Variable Type Colors - ACTUAL UE5 COLORS with 20% Saturation */
        .io-type {
            font-family: var(--font-mono);
            font-size: 12px;
            padding: 4px 8px;
            border-radius: var(--radius-sm);
            flex-shrink: 0;
            font-weight: 500;
            display: inline-block;
        }
        
        /* Float types - UE5 Green (desaturated) */
        .io-type.float,
        .io-type.double,
        .io-type.real {
            color: #8FA68F;
            background-color: rgba(143, 166, 143, 0.08);
            border: 1px solid rgba(143, 166, 143, 0.25);
        }
        
        /* Integer/Byte types - UE5 Cyan (desaturated) */
        .io-type.int,
        .io-type.integer,
        .io-type.byte {
            color: #8FA6A6;
            background-color: rgba(143, 166, 166, 0.08);
            border: 1px solid rgba(143, 166, 166, 0.25);
        }
        
        /* Vector types - UE5 Yellow (desaturated) */
        .io-type.vector,
        .io-type.vector2,
        .io-type.vector2d,
        .io-type.vector3,
        .io-type.vector4 {
            color: #A6A68F;
            background-color: rgba(166, 166, 143, 0.08);
            border: 1px solid rgba(166, 166, 143, 0.25);
        }
        
        /* Rotator types - UE5 Light Blue/White (desaturated) */
        .io-type.rotator {
            color: #A6A6A6;
            background-color: rgba(166, 166, 166, 0.08);
            border: 1px solid rgba(166, 166, 166, 0.25);
        }
        
        /* Transform types - UE5 Orange (desaturated) */
        .io-type.transform {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }
        
        /* String/Name types - UE5 Purple/Magenta (desaturated) */
        .io-type.string,
        .io-type.name {
            color: #A68FA6;
            background-color: rgba(166, 143, 166, 0.08);
            border: 1px solid rgba(166, 143, 166, 0.25);
        }
        
        /* Text types - UE5 Orange (desaturated) */
        .io-type.text {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }
        
        /* Boolean types - UE5 Red (desaturated) */
        .io-type.bool,
        .io-type.boolean {
            color: #A68F8F;
            background-color: rgba(166, 143, 143, 0.08);
            border: 1px solid rgba(166, 143, 143, 0.25);
        }
)");

    FString AdvancedTypeBadges = TEXT(R"(
        /* Object/Reference types - UE5 Blue (desaturated) */
        .io-type.object,
        .io-type.actor,
        .io-type.component,
        .io-type.widget,
        .io-type.class {
            color: #8F96A6;
            background-color: rgba(143, 150, 166, 0.08);
            border: 1px solid rgba(143, 150, 166, 0.25);
        }
        
        /* Soft Reference types - UE5 Light Blue (desaturated) */
        .io-type.softref,
        .io-type.softobject,
        .io-type.softclass {
            color: #8FA1A6;
            background-color: rgba(143, 161, 166, 0.08);
            border: 1px solid rgba(143, 161, 166, 0.25);
        }
        
        /* Struct types - UE5 Blue (desaturated, same as object) */
        .io-type.struct {
            color: #8F96A6;
            background-color: rgba(143, 150, 166, 0.08);
            border: 1px solid rgba(143, 150, 166, 0.25);
        }
        
        /* Enum types - UE5 Dark Green (desaturated, distinct from float) */
        .io-type.enum {
            color: #8FA68C;
            background-color: rgba(143, 166, 140, 0.08);
            border: 1px solid rgba(143, 166, 140, 0.25);
        }
        
        /* Interface types - Red (desaturated, matching section headers) */
        .io-type.interface {
            color: #A68F91;
            background-color: rgba(166, 143, 145, 0.08);
            border: 1px solid rgba(166, 143, 145, 0.25);
        }
        
        /* DataTable types - UE5 Orange (desaturated) */
        .io-type.datatable {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }
        
        /* Delegate types - UE5 Gray (desaturated) */
        .io-type.delegate {
            color: #9FA6A6;
            background-color: rgba(159, 166, 166, 0.08);
            border: 1px solid rgba(159, 166, 166, 0.25);
        }
)");

    FString ContainerTypeBadges = TEXT(R"(
        /* Collection type indicators */
        .io-type.array::before {
            content: "[ ] ";
            font-size: 10px;
            opacity: 0.8;
        }
        
        .io-type.set::before {
            content: "{ } ";
            font-size: 10px;
            opacity: 0.8;
        }
        
        .io-type.map::before {
            content: "{k:v} ";
            font-size: 9px;
            opacity: 0.8;
        }

        /* Collection types inherit base type colors - String/Name (Purple) */
        .io-type.array.string,
        .io-type.set.string,
        .io-type.map.string,
        .io-type.array.name,
        .io-type.set.name,
        .io-type.map.name {
            color: #A68FA6;
            background-color: rgba(166, 143, 166, 0.08);
            border: 1px solid rgba(166, 143, 166, 0.25);
        }

        /* Collection types inherit base type colors - Objects (Blue) */
        .io-type.array.actor,
        .io-type.set.actor,
        .io-type.map.actor,
        .io-type.array.component,
        .io-type.set.component,
        .io-type.map.component,
        .io-type.array.object,
        .io-type.set.object,
        .io-type.map.object {
            color: #8F96A6;
            background-color: rgba(143, 150, 166, 0.08);
            border: 1px solid rgba(143, 150, 166, 0.25);
        }

        /* Collection types inherit base type colors - Vector (Yellow) */
        .io-type.array.vector,
        .io-type.set.vector,
        .io-type.map.vector {
            color: #A6A68F;
            background-color: rgba(166, 166, 143, 0.08);
            border: 1px solid rgba(166, 166, 143, 0.25);
        }

        /* Collection types inherit base type colors - Float (Green) */
        .io-type.array.float,
        .io-type.set.float,
        .io-type.map.float {
            color: #8FA68F;
            background-color: rgba(143, 166, 143, 0.08);
            border: 1px solid rgba(143, 166, 143, 0.25);
        }

        /* Collection types inherit base type colors - Integer (Cyan) */
        .io-type.array.int,
        .io-type.set.int,
        .io-type.map.int {
            color: #8FA6A6;
            background-color: rgba(143, 166, 166, 0.08);
            border: 1px solid rgba(143, 166, 166, 0.25);
        }

        /* Collection types inherit base type colors - Boolean (Red) */
        .io-type.array.bool,
        .io-type.set.bool,
        .io-type.map.bool {
            color: #A68F8F;
            background-color: rgba(166, 143, 143, 0.08);
            border: 1px solid rgba(166, 143, 143, 0.25);
        }
)");

    FString InheritanceAndFallbacks = TEXT(R"(
        /* Container type inheritance for advanced types */
        .io-type.array.struct,
        .io-type.set.struct,
        .io-type.map.struct {
            color: #8F96A6;
            background-color: rgba(143, 150, 166, 0.08);
            border: 1px solid rgba(143, 150, 166, 0.25);
        }
        
        .io-type.array.enum,
        .io-type.set.enum,
        .io-type.map.enum {
            color: #8FA68C;
            background-color: rgba(143, 166, 140, 0.08);
            border: 1px solid rgba(143, 166, 140, 0.25);
        }
        
        .io-type.array.interface,
        .io-type.set.interface,
        .io-type.map.interface {
            color: #A68F91;
            background-color: rgba(166, 143, 145, 0.08);
            border: 1px solid rgba(166, 143, 145, 0.25);
        }
        
        .io-type.array.datatable,
        .io-type.set.datatable,
        .io-type.map.datatable {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }
        
        .io-type.array.delegate,
        .io-type.set.delegate,
        .io-type.map.delegate {
            color: #9FA6A6;
            background-color: rgba(159, 166, 166, 0.08);
            border: 1px solid rgba(159, 166, 166, 0.25);
        }
        
        .io-type.array.softref,
        .io-type.set.softref,
        .io-type.map.softref,
        .io-type.array.softobject,
        .io-type.set.softobject,
        .io-type.map.softobject,
        .io-type.array.softclass,
        .io-type.set.softclass,
        .io-type.map.softclass {
            color: #8FA1A6;
            background-color: rgba(143, 161, 166, 0.08);
            border: 1px solid rgba(143, 161, 166, 0.25);
        }

        /* Text collection types */
        .io-type.array.text,
        .io-type.set.text,
        .io-type.map.text {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }

        /* Rotator collection types */
        .io-type.array.rotator,
        .io-type.set.rotator,
        .io-type.map.rotator {
            color: #A6A6A6;
            background-color: rgba(166, 166, 166, 0.08);
            border: 1px solid rgba(166, 166, 166, 0.25);
        }

        /* Transform collection types */
        .io-type.array.transform,
        .io-type.set.transform,
        .io-type.map.transform {
            color: #A6968F;
            background-color: rgba(166, 150, 143, 0.08);
            border: 1px solid rgba(166, 150, 143, 0.25);
        }
        
        /* Default fallback for unknown types */
        .io-type.unknown,
        .io-type:not([class*=" "]):not([class^="io-type "]) {
            color: var(--color-accent);
            background-color: rgba(138, 99, 210, 0.1);
            border: 1px solid rgba(138, 99, 210, 0.3);
        }

        /* Search highlighting */
        mark.search-highlight { 
            background-color: var(--color-accent) !important; 
            color: white !important; 
            border-radius: 3px; 
            padding: 1px 2px; 
            font-weight: 500; 
        }
    )");

    return BaseTypeBadges + CoreTypeBadges + AdvancedTypeBadges + ContainerTypeBadges + InheritanceAndFallbacks;
}

// Updated BlueprintMarkdownCSS.cpp - GetSyntaxHighlightingCSS() method
// Replace the existing GetSyntaxHighlightingCSS() method with this updated version

FString FBlueprintMarkdownCSS::GetSyntaxHighlightingCSS()
{
    return GetCodeBlockBaseCSS() + GetBlueprintSyntaxColorsCSS() + GetLinkStylingCSS() + GetCallKeywordCSS();
}

// Split into smaller functions to avoid string length limits
FString FBlueprintMarkdownCSS::GetCodeBlockBaseCSS()
{
    return TEXT(R"(
        /* =================================================================== */
        /* 6. CODE BLOCKS & SYNTAX HIGHLIGHTING - BASE STYLES
        /* =================================================================== */
        
        .code-container { 
            position: relative; 
        }

        pre {
            font-family: var(--font-mono);
            font-size: 13px;
            line-height: 1.7;
            background-color: var(--color-bg);
            border: 1px solid var(--color-border);
            padding: 20px;
            border-radius: var(--radius-md);
            overflow-x: auto;
            margin: 0;
        }
        
        .copy-btn {
            position: absolute;
            top: 12px;
            right: 12px;
            background-color: var(--color-bg-content);
            border: 1px solid var(--color-border);
            color: var(--color-text-secondary);
            font-family: var(--font-sans);
            font-size: 12px;
            padding: 6px 10px;
            border-radius: var(--radius-md);
            cursor: pointer;
            opacity: 0;
            visibility: hidden;
        }
        
        .code-container:hover .copy-btn { 
            opacity: 1; 
            visibility: visible; 
        }
        
        .copy-btn:hover { 
            background-color: var(--color-accent); 
            border-color: var(--color-accent); 
            color: white; 
        }
    )");
}

FString FBlueprintMarkdownCSS::GetBlueprintSyntaxColorsCSS()
{
    return TEXT(R"(
        /* Blueprint Syntax Colors - New Color Scheme */
        .bp-keyword { 
            color: #f0f0f0; 
            font-weight: 500;
        }
        
        .bp-event-name, 
        .bp-func-name { 
            color: #B8D0D6; 
            font-weight: 500;
        }
        
        .bp-param-name { 
            color: #d8c5b0; 
        }
        
        .bp-data-type {
            color: #d8c5b0;
        }
        
        .bp-pin-name {
            color: #b5b5b5;
        }
        
        .bp-var { 
            color: #9ccbe8; 
        }
        
        .bp-literal-number { 
            color: #90c695; 
        }
        
        .bp-literal-string {
            color: #f1a6f7;
        }
        
        .bp-literal-bool {
            color: #e06c75;
        }
        
        .bp-literal-object {
            color: #d2cdc6;
        }
        
        .bp-literal-name {
            color: #e0b3ff;
        }
        
        .bp-literal-text {
            color: #ffc0cb;
        }
        
        .bp-literal-tag {
            color: #9ad4e0;
        }
        
        .bp-literal-container {
            color: #94cfff;
        }
        
        .bp-info { 
            color: #6a6a6a; 
        }
        
        .bp-operator { 
            color: #d4d4d4; 
        }
        
        .bp-error {
            color: #e06c75;
        }
    )");
}

FString FBlueprintMarkdownCSS::GetCallKeywordCSS()
{
    return TEXT(R"(
        /* Call Keyword Colors */
        .bp-call-function {
            color: #bee6f9;
            font-style: italic;
        }
        
        .bp-call-macro {
            color: #bee6f9;
            font-style: italic;
        }
        
        .bp-call-custom-event {
            color: #dfa4a9;
            font-style: italic;
        }
        
        .bp-call-interface {
            color: #dfa5aa;
            font-style: italic;
        }
        
        .bp-call-collapsed-graph {
            color: #56B6C2;
            font-style: italic;
        }
        
        .bp-call-parent-function {
            color: #bee6f9;
            font-style: italic;
        }
    )");
}

FString FBlueprintMarkdownCSS::GetLinkStylingCSS()
{
    return TEXT(R"(
        /* Enhanced Graph Link Colors - Complete Coverage */
        a.graph-link { 
            text-decoration: underline;
            text-decoration-style: dotted;
            text-underline-offset: 3px;
            font-weight: 500;
        }
        
        a.graph-link:hover {
            text-decoration-style: solid;
        }
        
        /* Function links - Blue (#4FC3F7) */
        a.function-link,
        a.graph-link.function-link {
            color: #4FC3F7 f;
        }
        
        a.function-link:hover,
        a.graph-link.function-link:hover {
            background-color: rgba(79, 195, 247, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Macro links - Blue (#4FC3F7) */
        a.macro-link,
        a.graph-link.macro-link {
            color: #4FC3F7;
        }
        
        a.macro-link:hover,
        a.graph-link.macro-link:hover {
            background-color: rgba(79, 195, 247, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Custom Event links - Red (#dd7881) */
        a.custom-event-link,
        a.graph-link.custom-event-link {
            color: #dd7881;
        }
        
        a.custom-event-link:hover,
        a.graph-link.custom-event-link:hover {
            background-color: rgba(221, 120, 129, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Interface links - Red (#dd7881) */
        a.interface-link,
        a.graph-link.interface-link {
            color: #dd7881;
        }
        
        a.interface-link:hover,
        a.graph-link.interface-link:hover {
            background-color: rgba(221, 120, 129, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Collapsed Graph links - Cyan (#56B6C2) */
        a.collapsed-graph-link,
        a.graph-link.collapsed-graph-link {
            color: #56B6C2;
        }
        
        a.collapsed-graph-link:hover,
        a.graph-link.collapsed-graph-link:hover {
            background-color: rgba(86, 182, 194, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Legacy event-link class (if still used) - Red (#dd7881) */
        a.event-link,
        a.graph-link.event-link {
            color: #dd7881;
        }
        
        a.event-link:hover,
        a.graph-link.event-link:hover {
            background-color: rgba(221, 120, 129, 0.2);
            border-radius: 3px;
            padding: 1px 2px;
        }
        
        /* Default graph link fallback - Cyan (#56B6C2) */
        a.graph-link {
            color: #56B6C2;
        }
        
        
        /* Search highlighting */
        mark.search-highlight { 
            background-color: var(--color-accent); 
            color: white; 
            border-radius: 3px; 
            padding: 1px 2px; 
            font-weight: 500; 
        }
        
        mark.search-highlight.current {
            background-color: #f8a100;
            color: #111;
            border-radius: 3px;
        }
    )");
}






FString FBlueprintMarkdownCSS::GetScrollbarCSS()
{
    return TEXT(R"(
        /* =================================================================== */
        /* 7. CUSTOM SCROLLBARS & RESPONSIVE DESIGN
        /* =================================================================== */
        
        ::-webkit-scrollbar { 
            width: 10px; 
        }
        
        ::-webkit-scrollbar-track { 
            background: transparent; 
        }
        
        ::-webkit-scrollbar-thumb { 
            background: rgba(255, 255, 255, 0.1); 
            border-radius: 5px;
        }
        
        ::-webkit-scrollbar-thumb:hover { 
            background: rgba(255, 255, 255, 0.2); 
        }

        /* Responsive breakpoints for smaller screens */
        @media (max-width: 1024px) {
            body {
                grid-template-columns: 240px 1fr;
            }
            
            #sidebar {
                padding: 24px;
            }
            
            #main-content {
                padding: 48px;
            }
        }
        
        @media (max-width: 768px) {
            body {
                grid-template-columns: 1fr;
                grid-template-rows: auto 1fr;
            }
            
            #sidebar {
                height: auto;
                max-height: 200px;
                overflow-y: auto;
            }
            
            #main-content {
                padding: 24px;
            }
        }
    )");
}


