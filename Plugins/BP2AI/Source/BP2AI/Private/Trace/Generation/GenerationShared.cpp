/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Generation/GenerationShared.cpp


#include "GenerationShared.h"
#include "Logging/BP2AILog.h"

namespace CategoryUtils
{
    EDocumentationGraphCategory StringToCategory(const FString& CategoryString)
    {
        // Map helper-generated category strings to enum values
        // Based on CategoryAnalysisHelper output strings
        
        if (CategoryString == TEXT("Functions"))
        {
            return EDocumentationGraphCategory::Functions;
        }
        else if (CategoryString == TEXT("Pure Functions"))
        {
            return EDocumentationGraphCategory::PureFunctions;
        }
        else if (CategoryString == TEXT("Callable Custom Events"))
        {
            return EDocumentationGraphCategory::CustomEvents;
        }
        else if (CategoryString == TEXT("Collapsed Graphs") || CategoryString == TEXT("Pure Collapsed Graphs"))
        {
            // Both pure and executable collapsed graphs go to same category
            return EDocumentationGraphCategory::CollapsedGraphs;
        }
        else if (CategoryString == TEXT("Executable Macros"))
        {
            return EDocumentationGraphCategory::ExecutableMacros;
        }
        else if (CategoryString == TEXT("Pure Macros"))
        {
            return EDocumentationGraphCategory::PureMacros;
        }
        else if (CategoryString == TEXT("Interfaces"))
        {
            return EDocumentationGraphCategory::Interfaces;
        }
        else
        {
            // Unknown category - log warning and default to Functions
            UE_LOG(LogBP2AI, Warning, TEXT("CategoryUtils::StringToCategory: Unknown category string '%s', defaulting to Functions"), *CategoryString);
            return EDocumentationGraphCategory::Functions;
        }
    }
    
    
    FString CategoryToDisplayString(EDocumentationGraphCategory Category)
    {
        // Convert enum to user-friendly display names for UI
        switch (Category)
        {
        case EDocumentationGraphCategory::Functions:
            return TEXT("Functions");
        case EDocumentationGraphCategory::CustomEvents:
            // --- FIX: This string must match the output of CategoryAnalysisHelper ---
            return TEXT("Callable Custom Events");
        case EDocumentationGraphCategory::CollapsedGraphs:
            return TEXT("Collapsed Graphs");
        case EDocumentationGraphCategory::ExecutableMacros:
            return TEXT("Executable Macros");
        case EDocumentationGraphCategory::PureMacros:
            return TEXT("Pure Macros");
        case EDocumentationGraphCategory::Interfaces:
            return TEXT("Interfaces");
        case EDocumentationGraphCategory::PureFunctions:
            return TEXT("Pure Functions");
        default:
            return TEXT("Unknown Category");
        }
    }
    
    TArray<EDocumentationGraphCategory> GetRequiredCategoryOrder()
    {
        // Return categories in exact required order per requirements:
        // 1. Functions, 2. Custom Events, 3. Collapsed Graphs, 4. Executable Macros,
        // 5. Pure Macros, 6. Interfaces, 7. Pure Functions
        
        TArray<EDocumentationGraphCategory> OrderedCategories;
        OrderedCategories.Add(EDocumentationGraphCategory::Functions);
        OrderedCategories.Add(EDocumentationGraphCategory::CustomEvents);
        OrderedCategories.Add(EDocumentationGraphCategory::CollapsedGraphs);
        OrderedCategories.Add(EDocumentationGraphCategory::ExecutableMacros);
        OrderedCategories.Add(EDocumentationGraphCategory::PureMacros);
        OrderedCategories.Add(EDocumentationGraphCategory::Interfaces);
        OrderedCategories.Add(EDocumentationGraphCategory::PureFunctions);
        
        return OrderedCategories;
    }
    
    bool IsCategoryStringVisible(const FString& CategoryString, const FGenerationSettings& Settings)
    {
        // Convert string to enum and check visibility
        EDocumentationGraphCategory Category = StringToCategory(CategoryString);
        return Settings.IsCategoryVisible(Category);
    }

    
}

void FBlueprintTypeInfo::GenerateDisplayInfo()
{
    // Generate display name
    if (!ContainerType.IsEmpty())
    {
        if (ContainerType.ToLower() == TEXT("map") && !ValueType.IsEmpty())
        {
            DisplayName = FString::Printf(TEXT("Map<%s, %s>"), *BaseType, *ValueType);
        }
        else
        {
            DisplayName = FString::Printf(TEXT("%s<%s>"), *ContainerType, *BaseType);
        }
    }
    else
    {
        DisplayName = BaseType;
    }
    
    // Generate CSS classes for styling
    TArray<FString> Classes;
    
    // Add base type class (always add the original type)
    Classes.Add(BaseType.ToLower());
    
    // Add container type class if present
    if (!ContainerType.IsEmpty())
    {
        Classes.Add(ContainerType.ToLower());
    }
    
    // ✅ ENHANCED: Add semantic type categories for comprehensive styling
    FString LowerBaseType = BaseType.ToLower();
    
    // Primitive numeric types - Green family
    if (LowerBaseType == TEXT("float") || LowerBaseType == TEXT("double") || LowerBaseType == TEXT("real"))
    {
        Classes.Add(TEXT("float"));
    }
    else if (LowerBaseType == TEXT("int") || LowerBaseType == TEXT("integer") || LowerBaseType == TEXT("byte"))
    {
        Classes.Add(TEXT("int"));
    }
    // Boolean types - Red
    else if (LowerBaseType == TEXT("bool") || LowerBaseType == TEXT("boolean"))
    {
        Classes.Add(TEXT("bool"));
    }
    // String types - Pink/Magenta  
    else if (LowerBaseType == TEXT("string") || LowerBaseType == TEXT("text") || LowerBaseType == TEXT("name"))
    {
        Classes.Add(TEXT("string"));
    }
    // Vector/Transform types - Yellow
    else if (LowerBaseType == TEXT("vector") || LowerBaseType == TEXT("vector2") || LowerBaseType == TEXT("vector2d") ||
             LowerBaseType == TEXT("vector3") || LowerBaseType == TEXT("vector4") || 
             LowerBaseType == TEXT("rotator") || LowerBaseType == TEXT("transform"))
    {
        Classes.Add(TEXT("vector"));
    }
    // Object reference types - Blue family
    else if (LowerBaseType == TEXT("object") || LowerBaseType == TEXT("actor") || LowerBaseType == TEXT("component") || 
             LowerBaseType == TEXT("widget") || LowerBaseType == TEXT("class"))
    {
        Classes.Add(TEXT("object"));
    }
    // ✅ NEW: Soft reference types - Light Blue
    else if (LowerBaseType == TEXT("softobject") || LowerBaseType == TEXT("softclass"))
    {
        Classes.Add(TEXT("softref"));
    }
    // ✅ NEW: Struct types - Teal
    else if (LowerBaseType.EndsWith(TEXT("struct")) || 
             (LowerBaseType != TEXT("bool") && LowerBaseType != TEXT("int") && LowerBaseType != TEXT("float") && 
              LowerBaseType != TEXT("string") && LowerBaseType != TEXT("name") && LowerBaseType != TEXT("object") && 
              LowerBaseType != TEXT("actor") && LowerBaseType != TEXT("component") && LowerBaseType != TEXT("class") &&
              !LowerBaseType.StartsWith(TEXT("e_")) && !LowerBaseType.Contains(TEXT("interface")) &&
              !LowerBaseType.Contains(TEXT("delegate")) && !LowerBaseType.Contains(TEXT("datatable"))))
    {
        Classes.Add(TEXT("struct"));
    }
    // ✅ NEW: Enum types - missing correct categorisation implementation currently Hardcoded 
    else if (LowerBaseType.StartsWith(TEXT("e_")) || LowerBaseType.EndsWith(TEXT("enum")))
    {
        Classes.Add(TEXT("enum"));
    }
    // ✅ NEW: Interface types - Purple
    else if (LowerBaseType.Contains(TEXT("interface")) || LowerBaseType.StartsWith(TEXT("bpi_")))
    {
        Classes.Add(TEXT("interface"));
    }
    // ✅ NEW: DataTable types - Cyan
    else if (LowerBaseType.Contains(TEXT("datatable")))
    {
        Classes.Add(TEXT("datatable"));
    }
    // ✅ NEW: Delegate types - Gray
    else if (LowerBaseType.Contains(TEXT("delegate")))
    {
        Classes.Add(TEXT("delegate"));
    }
    // Default fallback for unknown types
    else
    {
        Classes.Add(TEXT("unknown"));
    }
    
    // Remove duplicates using TSet and join
    TSet<FString> UniqueClasses(Classes);
    Classes = UniqueClasses.Array();
    CSSClasses = FString::Join(Classes, TEXT(" "));
    
    UE_LOG(LogBP2AI, Verbose, TEXT("GenerateDisplayInfo: BaseType='%s', Container='%s' -> Display='%s', CSS='%s'"),
        *BaseType, *ContainerType, *DisplayName, *CSSClasses);
}



