/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/GenerationShared.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/SemanticData.h"

// Forward declarations
class FBlueprintNode;
class UEdGraphNode;

// ✅ NEW: Category enumeration for user-configurable visibility
enum class BP2AI_API EDocumentationGraphCategory : uint8
{
    Functions,          // Regular, non-interface, non-pure user functions; Parent functions
    CustomEvents,       // Custom Events
    CollapsedGraphs,    // Collapsed Graphs (both pure and executable)
    ExecutableMacros,   // User-defined executable macros
    PureMacros,         // User-defined pure macros
    Interfaces,         // BPI definitions AND implementations in regular BPs
    PureFunctions       // Pure, non-interface user functions
};

// ✅ PURE STRUCTURED DATA - NO FORMATTING
struct BP2AI_API FTraceEntry
{
    FString TraceName;        
    FString NodeType;         
    TArray<FString> ExecutionLines;  // ⚠️ KEPT - For backward compatibility during migration
    FString TraceId;          
    
    // ✅ ADDED: New semantic data (DO NOT REMOVE ExecutionLines yet!)
    TArray<FSemanticExecutionStep> ExecutionSteps; // NEW - Clean semantic data
    
    FTraceEntry() = default;
    FTraceEntry(const FString& InTraceName, const FString& InNodeType, const TArray<FString>& InLines)
        : TraceName(InTraceName), NodeType(InNodeType), ExecutionLines(InLines)
    {
        TraceId = TraceName.Replace(TEXT(" "), TEXT("-")).ToLower();
    }
    
    // ✅ ADDED: New semantic constructor
    FTraceEntry(const FString& InTraceName, const FString& InNodeType, const TArray<FSemanticExecutionStep>& InSteps)
        : TraceName(InTraceName), NodeType(InNodeType), ExecutionSteps(InSteps)
    {
        TraceId = TraceName.Replace(TEXT(" "), TEXT("-")).ToLower();
        // Optionally, you could populate ExecutionLines here from InSteps for full backward compatibility immediately,
        // but Plan 3 suggests doing this via a helper method if/when needed during transition.
    }
};

// In FGraphDefinitionEntry struct
struct BP2AI_API FGraphDefinitionEntry
{
    FString GraphName;
    FString Category;
    FString AssetContext;
    FString AnchorId;
    TArray<FString> InputSpecs;
    TArray<FString> OutputSpecs;
    TArray<FString> ExecutionFlow;    // ⚠️ KEPT - For backward compatibility
    
    // ✅ ADDED: New semantic data
    TArray<FSemanticExecutionStep> SemanticExecutionFlow; // RENAMED from ExecutionSteps in plan for clarity
    
    bool bIsPure;
    
    FGraphDefinitionEntry() : bIsPure(false) {}
    // Existing constructor for FGraphDefinitionEntry doesn't take execution flow directly, it's populated later.
    // If a constructor taking semantic steps is needed, it would be added similarly to FTraceEntry.
};

struct BP2AI_API FSectionMetadata
{
    FString SectionName;
    int32 Level;
    bool bIsMajorSection;
    
    FSectionMetadata() : Level(1), bIsMajorSection(false) {}
    FSectionMetadata(const FString& InName, int32 InLevel, bool bInIsMajor = false)
        : SectionName(InName), Level(InLevel), bIsMajorSection(bInIsMajor) {}
};

// ✅ UPDATED FTracingResults - PURE STRUCTURED DATA
struct BP2AI_API FTracingResults
{
    // ✅ Structured data containers
    TArray<FTraceEntry> ExecutionTraces;              
    TArray<FGraphDefinitionEntry> GraphDefinitions;   
    TArray<FSectionMetadata> SectionHeaders;          
    FString RootBlueprintName;
    
    // ✅ Legacy arrays for TOC generation (keep for compatibility)
    TArray<FString> CollectedTraceHeaders;
    TArray<FString> CollectedGraphDefinitions;
    TMap<FString, FString> CollectedGraphDefinitionsWithCategories;
    TArray<FString> CollectedExecutableGraphs;
    TArray<FString> CollectedPureGraphs;
    
    // ✅ Clear methods
    void Clear()
    {
        ExecutionTraces.Empty();
        GraphDefinitions.Empty();
        SectionHeaders.Empty();
        CollectedTraceHeaders.Empty();
        CollectedGraphDefinitions.Empty();
        CollectedGraphDefinitionsWithCategories.Empty();
        CollectedExecutableGraphs.Empty();
        CollectedPureGraphs.Empty();
    }
};

// ✅ ENHANCED: Generation settings with category visibility
struct BP2AI_API FGenerationSettings
{
    bool bTraceAllSelected = false;
    bool bDefineUserGraphsSeparately = true;
    bool bExpandCompositesInline = false;
    bool bShowTrivialDefaultParams = true;
    bool bShouldTraceSymbolicallyForData = false;
    bool bUseSemanticData = false;

    // 🔴 ADD: Phase 4 migration control flags
    bool bUseSemanticDataGeneration = false;    // Master generation control - defaults OFF
    bool bEnableSemanticValidation = true;      // Dual-output validation during migration
    bool bDebugSemanticMigration = false;

    
    // Category visibility control
    TMap<EDocumentationGraphCategory, bool> CategoryVisibility;
    
    // Constructor: Initialize all categories as visible (default behavior)
    FGenerationSettings()
    {
        CategoryVisibility.Add(EDocumentationGraphCategory::Functions, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, true);
        CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, true);
        // 🔴 ADD: Safe defaults
        bUseSemanticDataGeneration = false;
        bEnableSemanticValidation = true;
        bDebugSemanticMigration = false;
    }
    
    // ✅ Helper method for easy visibility checking
    bool IsCategoryVisible(EDocumentationGraphCategory Category) const
    {
        const bool* VisibilityPtr = CategoryVisibility.Find(Category);
        return VisibilityPtr ? *VisibilityPtr : true; // Default to visible if not found
    }

    bool ShouldUseSemanticGeneration() const { return bUseSemanticDataGeneration; }


    
};



struct BP2AI_API FBlueprintTypeInfo
{
    FString BaseType;
    FString ContainerType;  
    FString ValueType;      // For Map<Key, Value>
    FString DisplayName;    // Generated display string
    FString CSSClasses;     // Generated CSS classes for styling
    
    FBlueprintTypeInfo() = default;
    
    explicit FBlueprintTypeInfo(const FString& InBaseType) 
        : BaseType(InBaseType) 
    {
        GenerateDisplayInfo();
    }
    
    FBlueprintTypeInfo(const FString& InBaseType, const FString& InContainerType)
        : BaseType(InBaseType), ContainerType(InContainerType)
    {
        GenerateDisplayInfo();
    }
    
    void GenerateDisplayInfo();
};

// ✅ NEW: Enhanced input/output specification with type information
struct BP2AI_API FBlueprintIOSpec
{
    FString Name;               // Pin name: "Damage Amount"
    FBlueprintTypeInfo TypeInfo; // Enhanced type information
    FString ValueExpression;    // Optional value expression for outputs
    FString Description;        // Optional description
    
    FBlueprintIOSpec() = default;
    
    FBlueprintIOSpec(const FString& InName, const FBlueprintTypeInfo& InTypeInfo)
        : Name(InName), TypeInfo(InTypeInfo)
    {
    }
    
    FBlueprintIOSpec(const FString& InName, const FBlueprintTypeInfo& InTypeInfo, const FString& InValueExpression)
        : Name(InName), TypeInfo(InTypeInfo), ValueExpression(InValueExpression)
    {
    }
};



// ✅ NEW: Category utility functions
namespace CategoryUtils
{
    // Convert helper-generated category strings to enum values
    BP2AI_API EDocumentationGraphCategory StringToCategory(const FString& CategoryString);
    
    // Convert enum to user-friendly display strings
    BP2AI_API FString CategoryToDisplayString(EDocumentationGraphCategory Category);
    
    // Get categories in required order for consistent presentation
    BP2AI_API TArray<EDocumentationGraphCategory> GetRequiredCategoryOrder();
    
    // Check if a category string should be visible based on settings
    BP2AI_API bool IsCategoryStringVisible(const FString& CategoryString, const FGenerationSettings& Settings);
}

// ✅ Clean interface for document builders
class BP2AI_API IDocumentBuilder
{
public:
    virtual ~IDocumentBuilder() = default;
    virtual FString BuildDocument(const FTracingResults& TracingData, const FGenerationSettings& Settings) = 0;
    virtual FString GetFileExtension() const = 0;
    virtual FString GetDisplayName() const = 0;
};