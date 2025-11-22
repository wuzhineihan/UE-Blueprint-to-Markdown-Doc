/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Models/BlueprintPin.cpp

#include "Models/BlueprintPin.h"
#include "Trace/Utils/MarkdownTracerUtils.h" // For ExtractSimpleNameFromPath
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"
#include "Logging/BP2AILog.h" // Include for LogModels (or a new LogBlueprintPin)
#include "UObject/UObjectGlobals.h"
#include "UObject/EnumProperty.h"


// Define NAME_* constants manually if needed and not accessible otherwise
// These should match definitions in other files like MarkdownFormattingUtils.cpp for consistency.
#ifndef NAME_Bool
#define NAME_Bool FName(TEXT("bool"))
#endif
#ifndef NAME_Byte
#define NAME_Byte FName(TEXT("byte"))
#endif
#ifndef NAME_Int
#define NAME_Int FName(TEXT("int"))
#endif
#ifndef NAME_Int64
#define NAME_Int64 FName(TEXT("int64")) // Make sure this is defined if used
#endif
#ifndef NAME_Float
#define NAME_Float FName(TEXT("float"))
#endif
#ifndef NAME_Double
#define NAME_Double FName(TEXT("double")) // Make sure this is defined if used
#endif
#ifndef NAME_Real // Often an alias for float or double in some contexts
#define NAME_Real FName(TEXT("real"))
#endif
#ifndef NAME_String
#define NAME_String FName(TEXT("string"))
#endif
#ifndef NAME_Text
#define NAME_Text FName(TEXT("text"))
#endif
#ifndef NAME_Name
#define NAME_Name FName(TEXT("name"))
#endif
#ifndef NAME_Object
#define NAME_Object FName(TEXT("object"))
#endif
#ifndef NAME_Class
#define NAME_Class FName(TEXT("class"))
#endif
#ifndef NAME_Interface
#define NAME_Interface FName(TEXT("interface"))
#endif
#ifndef NAME_Struct
#define NAME_Struct FName(TEXT("struct"))
#endif
#ifndef NAME_Enum
#define NAME_Enum FName(TEXT("enum")) // Define NAME_Enum
#endif
#ifndef NAME_Delegate
#define NAME_Delegate FName(TEXT("delegate"))
#endif
// Add any other NAME_ constants that might be categories

FBlueprintPin::FBlueprintPin(const FString& InId, const FString& InNodeGuid)
	: Id(InId)
	, NodeGuid(InNodeGuid)
    // Initialize other members if necessary, though default construction is often fine for FString, bool, etc.
    , bIsReference(false)
    , bIsConst(false)
{
}

bool FBlueprintPin::IsOutput() const
{
	return Direction == TEXT("EGPD_Output");
}

bool FBlueprintPin::IsInput() const
{
	// Any direction that is not Output is considered Input for simplicity here.
	// Could also be `return Direction == TEXT("EGPD_Input");` if strictly adhering.
	return Direction != TEXT("EGPD_Output");
}

bool FBlueprintPin::IsExecution() const
{
	return Category == TEXT("exec");
}

bool FBlueprintPin::IsHidden() const
{
	const FString* HiddenValue = RawProperties.Find(TEXT("bHidden"));
	if (HiddenValue)
	{
		return *HiddenValue == TEXT("true") || *HiddenValue == TEXT("True") || *HiddenValue == TEXT("TRUE");
	}
	return false; // Default to not hidden if property not found
}

bool FBlueprintPin::IsAdvancedView() const
{
	const FString* AdvancedValue = RawProperties.Find(TEXT("bAdvancedView"));
	if (AdvancedValue)
	{
		return *AdvancedValue == TEXT("true") || *AdvancedValue == TEXT("True") || *AdvancedValue == TEXT("TRUE");
	}
	return false; // Default to not advanced if property not found
}



FString FBlueprintPin::GetTypeSignature() const
{
    const FName CategoryFName(*Category);
    FString MainTypeComponent = Category; 

    if (!SubCategoryObject.IsEmpty()) {
     
        UObject* EnumCheckObject = FindObject<UObject>(nullptr, *SubCategoryObject);
        bool bIsEnum = EnumCheckObject && EnumCheckObject->IsA(UEnum::StaticClass());
 

        // Check if the category itself indicates it needs SubCategoryObject for the specific type name
        if (CategoryFName == NAME_Struct || 
            CategoryFName == NAME_Object || 
            CategoryFName == NAME_Interface || 
            CategoryFName == NAME_Class ||
            CategoryFName == NAME_Enum ||
            (CategoryFName == NAME_Byte && bIsEnum) // <<< USE THE ROBUST CHECK HERE
           ) 
        {
            UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MainType: Path fed to ExtractSimpleNameFromPath for SubCategoryObject: '%s' (Pin: %s)"), *SubCategoryObject, *Name);
            MainTypeComponent = MarkdownTracerUtils::ExtractSimpleNameFromPath(SubCategoryObject, TEXT("")); // Context for type names is usually global (empty string)
            UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MainType: ExtractSimpleNameFromPath for SubCategoryObject returned: '%s' (Pin: %s)"), *MainTypeComponent, *Name);
            if (MainTypeComponent.IsEmpty()) MainTypeComponent = Category; 
        }
        else if (CategoryFName == FName(TEXT("softobject")) || 
                 CategoryFName == FName(TEXT("softclass")) || 
                 CategoryFName == FName(TEXT("weakobject"))) 
        {
            UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - ReferenceType: Path fed to ExtractSimpleNameFromPath for SubCategoryObject: '%s' (Pin: %s)"), *SubCategoryObject, *Name);
            FString BaseType = MarkdownTracerUtils::ExtractSimpleNameFromPath(SubCategoryObject, TEXT(""));
            FString Suffix = MarkdownTracerUtils::GetReferenceTypeSuffix(Category);
            MainTypeComponent = BaseType + Suffix;
            UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - ReferenceType: BaseType='%s', Suffix='%s', Final='%s' (Pin: %s)"), *BaseType, *Suffix, *MainTypeComponent, *Name);
            if (MainTypeComponent.IsEmpty() || MainTypeComponent == Suffix) MainTypeComponent = Category; // Safety fallback
        }
        else if (CategoryFName == FName(TEXT("wildcard")) ) {
             UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MainType (Wildcard Case): Path fed to ExtractSimpleNameFromPath for SubCategoryObject: '%s' (Pin: %s)"), *SubCategoryObject, *Name);
            MainTypeComponent = MarkdownTracerUtils::ExtractSimpleNameFromPath(SubCategoryObject, TEXT(""));
            UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MainType (Wildcard Case): ExtractSimpleNameFromPath for SubCategoryObject returned: '%s' (Pin: %s)"), *MainTypeComponent, *Name);
            if (MainTypeComponent.IsEmpty()) MainTypeComponent = Category; // Fallback if somehow still empty
        }
    } 
    else if (!SubCategory.IsEmpty() && SubCategory != TEXT("None") && SubCategory != Category) {
        MainTypeComponent = SubCategory;
    }
    
    if (MainTypeComponent.IsEmpty() && !Category.IsEmpty() && Category != TEXT("None")) {
        MainTypeComponent = Category;
    } else if (MainTypeComponent.IsEmpty()) {
        MainTypeComponent = TEXT("?"); 
    }

    FString Result;
   
    if (ContainerType == TEXT("Map")) {
        FString ValueTypeStr = MapValueTerminalCategory; 
        const FName MapValueTerminalCategoryFName(*MapValueTerminalCategory);

        if (!MapValueTerminalSubCategoryObjectPath.IsEmpty()) {
            // ? --- NEW, ROBUST ENUM CHECK FOR MAP VALUE ---
            UObject* MapValueEnumCheckObject = FindObject<UObject>(nullptr, *MapValueTerminalSubCategoryObjectPath);
            bool bIsMapValueEnum = MapValueEnumCheckObject && MapValueEnumCheckObject->IsA(UEnum::StaticClass());
            // ? --- END NEW CHECK ---

            if (MapValueTerminalCategoryFName == NAME_Struct ||
                MapValueTerminalCategoryFName == NAME_Object ||
                MapValueTerminalCategoryFName == NAME_Interface ||
                MapValueTerminalCategoryFName == NAME_Class ||
                MapValueTerminalCategoryFName == NAME_Enum ||
               (MapValueTerminalCategoryFName == NAME_Byte && bIsMapValueEnum)) // <<< USE THE ROBUST CHECK
               
            {
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueType: Path fed to ExtractSimpleNameFromPath for MapValueTerminalSubCategoryObjectPath: '%s' (Pin: %s)"), *MapValueTerminalSubCategoryObjectPath, *Name);
                ValueTypeStr = MarkdownTracerUtils::ExtractSimpleNameFromPath(MapValueTerminalSubCategoryObjectPath, TEXT(""));
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueType: ExtractSimpleNameFromPath for MapValueTerminalSubCategoryObjectPath returned: '%s' (Pin: %s)"), *ValueTypeStr, *Name);
                if (ValueTypeStr.IsEmpty()) ValueTypeStr = MapValueTerminalCategory; 
            }
            else if (MapValueTerminalCategoryFName == FName(TEXT("softobject")) || 
                     MapValueTerminalCategoryFName == FName(TEXT("softclass")) || 
                     MapValueTerminalCategoryFName == FName(TEXT("weakobject"))) 
            {
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueReferenceType: Path fed to ExtractSimpleNameFromPath for MapValueTerminalSubCategoryObjectPath: '%s' (Pin: %s)"), *MapValueTerminalSubCategoryObjectPath, *Name);
                FString BaseType = MarkdownTracerUtils::ExtractSimpleNameFromPath(MapValueTerminalSubCategoryObjectPath, TEXT(""));
                FString Suffix = MarkdownTracerUtils::GetReferenceTypeSuffix(MapValueTerminalCategory);
                ValueTypeStr = BaseType + Suffix;
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueReferenceType: BaseType='%s', Suffix='%s', Final='%s' (Pin: %s)"), *BaseType, *Suffix, *ValueTypeStr, *Name);
                if (ValueTypeStr.IsEmpty() || ValueTypeStr == Suffix) ValueTypeStr = MapValueTerminalCategory; // Safety fallback
            }
            else if (MapValueTerminalCategoryFName == FName(TEXT("wildcard"))) {
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueType (Wildcard Case): Path fed to ExtractSimpleNameFromPath for MapValueTerminalSubCategoryObjectPath: '%s' (Pin: %s)"), *MapValueTerminalSubCategoryObjectPath, *Name);
                ValueTypeStr = MarkdownTracerUtils::ExtractSimpleNameFromPath(MapValueTerminalSubCategoryObjectPath, TEXT(""));
                UE_LOG(LogModels, Warning, TEXT("GetTypeSignature - MapValueType (Wildcard Case): ExtractSimpleNameFromPath for MapValueTerminalSubCategoryObjectPath returned: '%s' (Pin: %s)"), *ValueTypeStr, *Name);
                if (ValueTypeStr.IsEmpty()) ValueTypeStr = MapValueTerminalCategory; // Fallback
            }
        }
        
        if (ValueTypeStr.IsEmpty() || ValueTypeStr == TEXT("None") && MapValueTerminalCategory != TEXT("None") && !MapValueTerminalCategory.IsEmpty()) {
            ValueTypeStr = MapValueTerminalCategory;
        } else if (ValueTypeStr.IsEmpty()) {
            ValueTypeStr = TEXT("?"); 
        }
        
        UE_LOG(LogModels, Log, TEXT("GetTypeSignature MAP: PinName='%s', NodeGuid='%s'"), *Name, *NodeGuid);
        UE_LOG(LogModels, Log, TEXT("  Map Key Info: Category(Pin)='%s', SubCategory(Pin)='%s', SubCategoryObject(Pin)='%s' -> DerivedKeyType='%s'"),
            *Category, *SubCategory, *SubCategoryObject, *MainTypeComponent);
        UE_LOG(LogModels, Log, TEXT("  Map Value Info: MapValueTerminalCategory='%s', MapValueTerminalSubCategoryObjectPath='%s' -> DerivedValueType='%s'"),
            *MapValueTerminalCategory, *MapValueTerminalSubCategoryObjectPath, *ValueTypeStr);
        
        Result += ContainerType + TEXT("<") + MainTypeComponent + TEXT(", ") + ValueTypeStr + TEXT(">");

    } else if (!ContainerType.IsEmpty() && ContainerType != TEXT("None")) { 
        Result += ContainerType + TEXT("<") + MainTypeComponent + TEXT(">"); 
    } else { 
        Result += MainTypeComponent;
    }

    if (bIsReference) {
        Result += TEXT("&");
    }

    UE_LOG(LogModels, Verbose, TEXT("GetTypeSignature for Pin '%s': Result -> '%s' (Based on MainTypeComponent='%s', Container='%s')"), 
        *Name, *Result, *MainTypeComponent, *ContainerType);
    return Result;
}

