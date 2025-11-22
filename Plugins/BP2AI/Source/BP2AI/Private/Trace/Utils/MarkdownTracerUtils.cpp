/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownTracerUtils.cpp


#include "MarkdownTracerUtils.h" // Include self header
#include "Models/BlueprintPin.h" // Include necessary models/headers
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"
#include "Misc/DefaultValueHelper.h"
#include "Math/UnrealMathUtility.h"
#include "Trace/FMarkdownPathTracer.h"


// Define NAME_ constants manually if needed and not accessible otherwise
#ifndef NAME_Bool
#define NAME_Bool FName(TEXT("bool"))
#endif
#ifndef NAME_Byte
#define NAME_Byte FName(TEXT("byte"))
#endif
#ifndef NAME_Int
#define NAME_Int FName(TEXT("int"))
#endif
#ifndef NAME_Float
#define NAME_Float FName(TEXT("float"))
#endif
#ifndef NAME_Real
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
#ifndef NAME_Delegate
#define NAME_Delegate FName(TEXT("delegate"))
#endif

namespace MarkdownTracerUtils
{

// Static set of known core type paths and their desired simple names
static const TMap<FString, FString> CoreTypePathToSimpleNameMap = {
    {TEXT("/Script/GameplayTags.GameplayTag"), TEXT("GameplayTag")},
    {TEXT("/Script/GameplayTags.GameplayTagContainer"), TEXT("GameplayTagContainer")},
{TEXT("/Script/CoreUObject.Vector"), TEXT("Vector")},
{TEXT("/Script/CoreUObject.Rotator"), TEXT("Rotator")},
{TEXT("/Script/CoreUObject.Transform"), TEXT("Transform")},
{TEXT("/Script/CoreUObject.LinearColor"), TEXT("LinearColor")}, // Common struct for colors
{TEXT("/Script/CoreUObject.Color"), TEXT("Color")},
{TEXT("/Script/Engine.Actor"), TEXT("Actor")},
{TEXT("/Script/Engine.Pawn"), TEXT("Pawn")},
{TEXT("/Script/Engine.Character"), TEXT("Character")},
{TEXT("/Script/Engine.PlayerController"), TEXT("PlayerController")},
{TEXT("/Script/Engine.GameModeBase"), TEXT("GameModeBase")}

};

// Static set of known common static Blueprint function library simplified names
    static const TSet<FString> KnownStaticBlueprintLibrariesSet = {
        TEXT("GameplayTags.BlueprintGameplayTagLibrary"),
        TEXT("Engine.KismetSystemLibrary"),
        TEXT("Engine.KismetMathLibrary"),
        TEXT("Engine.KismetTextLibrary"),
        TEXT("Engine.KismetStringLibrary"),
        TEXT("Engine.BlueprintArrayLibrary"),
        TEXT("Engine.BlueprintMapLibrary"),
        TEXT("Engine.BlueprintSetLibrary"),
        TEXT("Engine.GameplayStatics"),
        TEXT("Engine.BlueprintPathsLibrary"),
        TEXT("Engine.BlueprintPlatformLibrary"),
        TEXT("Engine.AnimationBlueprintLibrary"),
        TEXT("Engine.DataTableFunctionLibrary")
        
    // Add more known static libraries as identified
};

/*
     static const TSet<FString> KnownStaticBlueprintLibrariesSet = {
        // --- Existing Core Libraries ---
        TEXT("Engine.KismetSystemLibrary"),
        TEXT("Engine.KismetMathLibrary"),
        TEXT("Engine.KismetStringLibrary"),
        TEXT("Engine.GameplayStatics"),

        // --- Existing Container/Path Libraries ---
        TEXT("Engine.BlueprintArrayLibrary"),
        TEXT("Engine.BlueprintMapLibrary"),
        TEXT("Engine.BlueprintSetLibrary"),
        TEXT("Engine.BlueprintPathsLibrary"),
        TEXT("Engine.BlueprintPlatformLibrary"),

        // --- Existing Domain-Specific Libraries ---
        TEXT("Engine.AnimationBlueprintLibrary"),
        TEXT("Engine.DataTableFunctionLibrary"),

        // =================================================================
        // ? NEWLY ADDED LIBRARIES (Fixes and Enhancements)
        // =================================================================

        // --- Added Core Kismet Libraries ---
        TEXT("Engine.KismetTextLibrary"),       // Fixes the original reported issue
        TEXT("Engine.KismetRenderingLibrary"),
        TEXT("Engine.KismetInputLibrary"),
        TEXT("Engine.KismetMaterialLibrary"),

        // --- Added UMG (UI) Libraries ---
        TEXT("UMG.WidgetBlueprintLibrary"),
        TEXT("UMG.WidgetLayoutLibrary"),
        
        // --- Added Gameplay Ability System (GAS) Library ---
        TEXT("GameplayAbilities.GameplayAbilitiesBlueprintLibrary"),

        // --- Added Enhanced Input Library ---
        TEXT("EnhancedInput.EnhancedInputLibrary"),

        // --- Added Gameplay Tasks Library ---
        TEXT("GameplayTasks.GameplayTasksBlueprintLibrary")
    };
*/
const TSet<FString>& GetKnownStaticBlueprintLibraries()
{
    return KnownStaticBlueprintLibrariesSet;
}

bool IsTrivialDefault(TSharedPtr<const FBlueprintPin> Pin)
    {
        if (!Pin.IsValid()) { UE_LOG(LogDataTracer, Warning, TEXT("IsTrivialDefault: Invalid Pin!")); return true; }
        UE_LOG(LogDataTracer, Verbose, TEXT("IsTrivialDefault: Pin=%s (%s)"), *Pin->Name, *Pin->Id);

        if (Pin->SourcePinFor.Num() > 0) return false;

        const FString& ValStr = Pin->DefaultValue;
        const FString& ObjStr = Pin->DefaultObject;
        const TMap<FString, FString>& StructMap = Pin->DefaultStruct;
        const FString AutoValStr = Pin->RawProperties.FindRef(TEXT("AutogeneratedDefaultValue"));

        if (ValStr.IsEmpty() && ObjStr.IsEmpty() && StructMap.IsEmpty()) return true;

        const FName Category = FName(*Pin->Category);

        if (!AutoValStr.IsEmpty() && !ValStr.IsEmpty()) {
            if (ValStr.TrimQuotes().Equals(AutoValStr.TrimQuotes(), ESearchCase::IgnoreCase)) {
                if (Category != NAME_Struct && Category != NAME_Object && Category != NAME_Class && Category != NAME_Interface && Category != FName(TEXT("asset")) && Category != FName(TEXT("assetclass")) && Category != FName(TEXT("softobject")) && Category != FName(TEXT("softclass"))) {
                     UE_LOG(LogDataTracer, Verbose, TEXT("IsTrivialDefault: Pin %s matches autogenerated default '%s'"), *Pin->Name, *AutoValStr);
                     return true;
                }
            }
        }

        if (Category == NAME_Bool) return ValStr.Equals(TEXT("false"), ESearchCase::IgnoreCase);
        if (Category == NAME_Byte || Category == NAME_Int || Category == FName(TEXT("int64")) || Category == NAME_Real || Category == NAME_Float || Category == FName(TEXT("double"))) {
            if (ValStr.IsEmpty()) return true;
            TOptional<double> NumVal = FCString::Atod(*ValStr);
            return NumVal.IsSet() && FMath::IsNearlyZero(NumVal.GetValue());
        }
        if (Category == NAME_String || Category == NAME_Text) return ValStr.IsEmpty();
        if (Category == NAME_Name) return ValStr.Equals(TEXT("None"), ESearchCase::IgnoreCase);
        if (Category == NAME_Object || Category == NAME_Class || Category == NAME_Interface || Category == FName(TEXT("asset")) || Category == FName(TEXT("assetclass")) || Category == FName(TEXT("softobject")) || Category == FName(TEXT("softclass"))) {
            return ObjStr.IsEmpty() || ObjStr.Equals(TEXT("None"), ESearchCase::IgnoreCase) || ObjStr.Equals(TEXT("NULL"), ESearchCase::IgnoreCase);
        }

        if (Category == NAME_Struct) {
            FString StructRep = ValStr;
            if (StructRep.IsEmpty() && StructMap.Num() > 0) {
                StructRep += TEXT("(");
                for(const auto& Pair : StructMap) { StructRep += FString::Printf(TEXT("%s=%s,"), *Pair.Key, *Pair.Value); }
                if (StructRep.EndsWith(TEXT(","))) { StructRep.LeftChopInline(1); }
                StructRep += TEXT(")");
            }

            if (StructRep.IsEmpty() || StructRep == TEXT("()") || StructRep == TEXT("{}")) return true;

            FString Content = StructRep.TrimStartAndEnd().TrimChar(TEXT('(')).TrimChar(TEXT(')'));
            TArray<FString> Parts;
            Content.ParseIntoArray(Parts, TEXT(","));
            bool bAllDefault = true;
            if (Parts.Num() == 0 && !Content.IsEmpty() && Content.ToLower() != TEXT("tagname=")) {
                 FString SingleValue = Content.TrimStartAndEnd().TrimQuotes();
                 if (!SingleValue.IsEmpty() &&
                     !SingleValue.Equals(TEXT("0")) &&
                     !SingleValue.Equals(TEXT("0.0")) &&
                     !SingleValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) &&
                     !SingleValue.Equals(TEXT("None"), ESearchCase::IgnoreCase) &&
                     SingleValue.ToLower() != TEXT("tagname=")) { bAllDefault = false; }
            } else {
                 for(const FString& Part : Parts) {
                     FString Key, ValuePart;
                     if (Part.Split(TEXT("="), &Key, &ValuePart)) {
                         ValuePart = ValuePart.TrimQuotes().TrimStartAndEnd();
                         if (Key.TrimStartAndEnd().Equals(TEXT("TagName"), ESearchCase::IgnoreCase)) {
                             if (!ValuePart.IsEmpty() && !ValuePart.Equals(TEXT("None"), ESearchCase::IgnoreCase)) { bAllDefault = false; break; }
                         } else {
                             if (!ValuePart.IsEmpty() && !ValuePart.Equals(TEXT("0")) && !ValuePart.Equals(TEXT("0.0")) && !ValuePart.Equals(TEXT("false"), ESearchCase::IgnoreCase) && !ValuePart.Equals(TEXT("None"), ESearchCase::IgnoreCase)) { bAllDefault = false; break; }
                         }
                     } else {
                         TArray<FString> Values; Part.ParseIntoArray(Values, TEXT(","));
                         for(const FString& Value : Values) {
                              FString CleanValue = Value.TrimStartAndEnd().TrimQuotes();
                              if (!CleanValue.IsEmpty() && !CleanValue.Equals(TEXT("0")) && !CleanValue.Equals(TEXT("0.0")) && !CleanValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) && !CleanValue.Equals(TEXT("None"), ESearchCase::IgnoreCase)) { bAllDefault = false; break; }
                         }
                         if (!bAllDefault) break;
                     }
                 }
            }
            if (bAllDefault) return true;
            return false;
        }

        const FName Container = FName(*Pin->ContainerType);
        if (Container == FName(TEXT("Array")) || Container == FName(TEXT("Set")) || Container == FName(TEXT("Map"))) {
            return ValStr.IsEmpty() || ValStr == TEXT("()") || ValStr == TEXT("[]") || ValStr == TEXT("{}");
        }
    
        return false;
    }
/*
FString MarkdownTracerUtils::ExtractSimpleNameFromPath(const FString& Path, const FString& CurrentBlueprintContextName)
{
    UE_LOG(LogDataTracer, Log, TEXT("ExtractSimpleNameFromPath IN: Path='%s', Context='%s'"), *Path, *CurrentBlueprintContextName);

    if (Path.IsEmpty())
    {
        return TEXT("");
    }

    if (const FString* SimpleCoreTypeName = CoreTypePathToSimpleNameMap.Find(Path))
    {
        UE_LOG(LogDataTracer, Log, TEXT("ExtractSimpleNameFromPath OUT (Core Type Override): Path='%s' -> Returns='%s'"), *Path, **SimpleCoreTypeName);
        return *SimpleCoreTypeName;
    }

    FString CleanedPath = Path.TrimStartAndEnd();

    if (CleanedPath.StartsWith(TEXT("/Script/")))
    {
        FString PathAfterScript = CleanedPath.RightChop(8);
        int32 FirstDotIndex;
        if (PathAfterScript.FindChar(TEXT('.'), FirstDotIndex))
        {
            FString ModuleName = PathAfterScript.Left(FirstDotIndex);
            FString ClassName = PathAfterScript.Mid(FirstDotIndex + 1);

            ClassName.RemoveFromStart(TEXT("Default__"));
            if (ClassName.EndsWith(TEXT("_C")))
            {
                ClassName.LeftChopInline(2);
            }

            // --- REFACTORED: Use the shared, single source of truth ---
            if (GetKnownStaticBlueprintLibraries().Contains(ModuleName + TEXT(".") + ClassName))
            {
                return TEXT("");
            }
            
            if (ModuleName.Equals(TEXT("Engine"), ESearchCase::IgnoreCase) ||
                ModuleName.Equals(TEXT("CoreUObject"), ESearchCase::IgnoreCase) ||
                ModuleName.Equals(TEXT("UMG"), ESearchCase::IgnoreCase))
            {
                return ClassName;
            }
            else
            {
                return FString::Printf(TEXT("%s:%s"), *ModuleName, *ClassName);
            }
        }
        else
        {
            return PathAfterScript;
        }
    }

    FString Name = FPaths::GetCleanFilename(CleanedPath);

    int32 DefaultPos = Name.Find(TEXT("Default__"));
    if (DefaultPos > 0)
    {
        Name = Name.Mid(DefaultPos);
    }
    
    int32 DotPos;
    if (Name.FindChar(TEXT('.'), DotPos))
    {
        FString PartBeforeDot = Name.Left(DotPos);
        FString PartAfterDot = Name.Mid(DotPos + 1);
        if (PartAfterDot.StartsWith(TEXT("Default__")))
        {
             PartAfterDot.RemoveFromStart(TEXT("Default__"));
        }
        if (PartAfterDot.EndsWith(TEXT("_C")))
        {
            PartAfterDot.LeftChopInline(2);
        }
        if (PartBeforeDot.Equals(PartAfterDot, ESearchCase::IgnoreCase))
        {
            Name = PartBeforeDot;
        }
    }

    Name.RemoveFromStart(TEXT("Default__"));
    Name.RemoveFromStart(TEXT("SKEL_"));
    Name.RemoveFromStart(TEXT("BP_"));
    Name.RemoveFromStart(TEXT("WBP_"));
    Name.RemoveFromStart(TEXT("ABP_"));
    Name.RemoveFromStart(TEXT("K2Node_"));
    Name.RemoveFromStart(TEXT("EdGraphNode_"));
    if (Name.EndsWith(TEXT("_C")))
    {
        Name.LeftChopInline(2);
    }
    
    if (Name.Contains(TEXT("::")))
    {
        Name = Name.Left(Name.Find(TEXT("::")));
    }
    
    // Check again for simple names after cleaning
    if (GetKnownStaticBlueprintLibraries().Contains(TEXT("Engine.") + Name))
    {
        return TEXT("");
    }

    if (Name.StartsWith(TEXT("Engine:")))
    {
        Name.RightChopInline(7);
    }

    UE_LOG(LogDataTracer, Log, TEXT("ExtractSimpleNameFromPath OUT: Final clean name: '%s'"), *Name);
    return Name;
} */
/* LEGACY */
FString MarkdownTracerUtils::ExtractSimpleNameFromPath(const FString& Path, const FString& CurrentBlueprintContextName)
{
    UE_LOG(LogDataTracer, Log, TEXT("ExtractSimpleNameFromPath IN: Path='%s', Context='%s'"), *Path, *CurrentBlueprintContextName);

    if (const FString* SimpleCoreTypeName = CoreTypePathToSimpleNameMap.Find(Path))
    {
        UE_LOG(LogDataTracer, Log, TEXT("ExtractSimpleNameFromPath OUT (Core Type Override): Path='%s' -> Returns='%s'"), *Path, **SimpleCoreTypeName);
        return *SimpleCoreTypeName;
    }

    FString OriginalPathForLog = Path;
    FString PathToProcess = Path;

    FString DerivedContext = TEXT("");
    FString DerivedItemName; 

    // Step 1: Initial Parsing to separate a potential context from the item name
    if (PathToProcess.StartsWith(TEXT("/Script/")))
    {
        FString PathAfterScript = PathToProcess.RightChop(8);
        int32 FirstDotIndex;
        if (PathAfterScript.FindChar(TEXT('.'), FirstDotIndex))
        {
            DerivedContext = PathAfterScript.Left(FirstDotIndex);
            DerivedItemName = PathAfterScript.Mid(FirstDotIndex + 1);
        }
        else
        {
            DerivedItemName = PathAfterScript;
            UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: /Script/ path without module.type: %s. ItemName set to: %s"), *PathToProcess, *DerivedItemName);
        }
    }
    else 
    {
        int32 ColonPos;
        if (PathToProcess.FindLastChar(TEXT(':'), ColonPos))
        {
            FString ContextPart = PathToProcess.Left(ColonPos); // Store this for final log
            DerivedItemName = PathToProcess.Mid(ColonPos + 1);
            DerivedContext = FPaths::GetBaseFilename(ContextPart); 
            
            int32 ContextDotPos;
            if (DerivedContext.FindChar(TEXT('.'), ContextDotPos)) {
                FString CtxPartBeforeDot = DerivedContext.Left(ContextDotPos);
                FString CtxPartAfterDot = DerivedContext.Mid(ContextDotPos + 1);
                if (CtxPartBeforeDot.Equals(CtxPartAfterDot, ESearchCase::IgnoreCase)) {
                    DerivedContext = CtxPartBeforeDot;
                }
            }
             UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: Colon found. ContextPart: '%s', DerivedContext: '%s', ItemName: '%s'"), *ContextPart, *DerivedContext, *DerivedItemName);
        }
        else
        {
            DerivedItemName = FPaths::GetCleanFilename(PathToProcess);
            UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: No colon. Clean Filename ItemName: '%s'"), *DerivedItemName);
        }
    }

    // Store copies for logging after initial parse
    const FString Log_ContextAfterInitialParse = DerivedContext;
    const FString Log_ItemNameAfterInitialParse = DerivedItemName;

    // Step 2: Simplify DerivedItemName 
    DerivedItemName.RemoveFromStart(TEXT("Default__"));
    if (DerivedItemName.EndsWith(TEXT("_C"))) { 
        DerivedItemName.LeftChopInline(2); 
    }

    // SKEL_ Pattern Simplification
    FString SkelPatternPrefix = TEXT(".SKEL_");
    int32 SkelSubPos = DerivedItemName.Find(SkelPatternPrefix);

    if (SkelSubPos != INDEX_NONE) 
    {
        FString PartBeforeSkel = DerivedItemName.Left(SkelSubPos); 
        FString PartAfterSkelSegment = DerivedItemName.Mid(SkelSubPos + SkelPatternPrefix.Len()); 

        FString NormPartBefore = PartBeforeSkel;
        if (NormPartBefore.StartsWith(TEXT("_"))) NormPartBefore.RightChopInline(1);
        
        FString NormPartAfter = PartAfterSkelSegment;
        if (NormPartAfter.StartsWith(TEXT("_"))) NormPartAfter.RightChopInline(1);
        
        if (NormPartBefore.Equals(NormPartAfter, ESearchCase::IgnoreCase) && !NormPartBefore.IsEmpty())
        {
            UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: Simplifying SKEL pattern from '%s' to '%s' (PartBefore: '%s', NormPartBefore: '%s', NormPartAfter: '%s')"), 
                *DerivedItemName, *PartBeforeSkel, *PartBeforeSkel, *NormPartBefore, *NormPartAfter);
            DerivedItemName = PartBeforeSkel;
        } else {
             UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: SKEL pattern (with dot) found but parts did not match for simplification. OriginalItem: '%s', PartBefore: '%s', NormPartBefore: '%s', NormPartAfter: '%s'"),
                *Log_ItemNameAfterInitialParse, *PartBeforeSkel, *NormPartBefore, *NormPartAfter);
        }
    }
    else if (DerivedItemName.StartsWith(TEXT("SKEL_"))) 
    {
        FString TempName = DerivedItemName.RightChop(FString(TEXT("SKEL_")).Len());
        if (TempName.StartsWith(TEXT("_"))) { // Handles SKEL__MyBP
            TempName.RightChopInline(1);
        }
        if (!TempName.IsEmpty()){
             UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: Stripped leading SKEL_ from '%s', now '%s'"), *DerivedItemName, *TempName);
            DerivedItemName = TempName;
        }
    }
    
    const FString Log_ItemAfterSkelStrip = DerivedItemName; // For logging

    if (DerivedItemName.Contains(TEXT(".")))
    {
        FString OriginalItemNameForDotSplitLog = DerivedItemName;
        int32 LastDotIndex;
        DerivedItemName.FindLastChar(TEXT('.'), LastDotIndex);

        if (LastDotIndex > 0 && LastDotIndex < DerivedItemName.Len() - 1 && !FChar::IsDigit(DerivedItemName[LastDotIndex+1]))
        {
            FString PartBeforeDot = DerivedItemName.Left(LastDotIndex);
            FString PartAfterDot = DerivedItemName.Mid(LastDotIndex + 1);
            
            if (PartBeforeDot.Equals(PartAfterDot, ESearchCase::IgnoreCase)) {
                DerivedItemName = PartBeforeDot; 
                UE_LOG(LogDataTracer, Verbose, TEXT("ExtractSimpleNameFromPath: Dot-split simplified '%s' to '%s'"), *OriginalItemNameForDotSplitLog, *DerivedItemName);
            }
        }
    }
    
    DerivedItemName.RemoveFromStart(TEXT("K2Node_"));
    DerivedItemName.RemoveFromStart(TEXT("EdGraphNode_"));
    DerivedItemName.RemoveFromStart(TEXT("AnimGraphNode_"));
    if (DerivedItemName.EndsWith(TEXT("_Graph"))) { 
        DerivedItemName.LeftChopInline(6); 
    }

    FString FinalDisplayNameToReturn;

    if (DerivedContext.Equals(TEXT("StandardMacros"), ESearchCase::IgnoreCase)) {
        FinalDisplayNameToReturn = DerivedItemName;
    }
    else if (!DerivedContext.IsEmpty() && 
             (MarkdownTracerUtils::GetKnownStaticBlueprintLibraries().Contains(DerivedContext + TEXT(".") + DerivedItemName) ||
              (DerivedContext.Equals(TEXT("Engine"), ESearchCase::IgnoreCase) && MarkdownTracerUtils::GetKnownStaticBlueprintLibraries().Contains(DerivedItemName))
             )
            )
    {
        FinalDisplayNameToReturn = FString::Printf(TEXT("%s.%s"), *DerivedContext, *DerivedItemName);
    }
    else if (!CurrentBlueprintContextName.IsEmpty() && DerivedContext.Equals(CurrentBlueprintContextName, ESearchCase::IgnoreCase)) {
        FinalDisplayNameToReturn = DerivedItemName; 
    }
    else if (!DerivedContext.IsEmpty()) { 
        FinalDisplayNameToReturn = FString::Printf(TEXT("%s:%s"), *DerivedContext, *DerivedItemName);
    }
    else { 
        FinalDisplayNameToReturn = DerivedItemName;
    }
    if (FinalDisplayNameToReturn.StartsWith(TEXT("Engine:"))) {
        FinalDisplayNameToReturn = FinalDisplayNameToReturn.RightChop(7); // Remove "Engine:"
    }
    UE_LOG(LogDataTracer, Warning, TEXT("ExtractSimpleNameFromPath OUT: Path='%s', Context='%s' -> InitialCtx='%s', InitialItem='%s', ItemAfterSkelStrip='%s', FinalItemAfterDotAndPrefixStrips='%s' -> Returns='%s'"),
        *OriginalPathForLog,
        *CurrentBlueprintContextName,
        *Log_ContextAfterInitialParse, 
        *Log_ItemNameAfterInitialParse, 
        *Log_ItemAfterSkelStrip,
        *DerivedItemName, // This is the fully simplified item name before contextual prefixing
        *FinalDisplayNameToReturn);

    
    return FinalDisplayNameToReturn;
}

// NormalizeConversionName remains unchanged
FString MarkdownTracerUtils::NormalizeConversionName(const FString& FuncName, const TMap<FString, FString>& ConversionMap)
{
    if (FuncName.IsEmpty()) return FuncName;
    if (ConversionMap.Contains(FuncName)) return FuncName;

    static const FRegexPattern ToPattern(TEXT("^(To(?:String|Text|Name|Bool))\\s*\\((.*?)\\)$"));
    FRegexMatcher ToMatcher(ToPattern, FuncName);
    if (ToMatcher.FindNext()) {
        FString BaseFunc = ToMatcher.GetCaptureGroup(1);
        FString InputType = ToMatcher.GetCaptureGroup(2).Replace(TEXT(" "), TEXT(""));
        FString TargetType = BaseFunc.RightChop(2);
        FString NormalizedKey = FString::Printf(TEXT("Conv_%sTo%s"), *InputType, *TargetType);

        if (ConversionMap.Contains(NormalizedKey)) {
             UE_LOG(LogDataTracer, Verbose, TEXT("NormalizeConversionName: Normalized '%s' to '%s'"), *FuncName, *NormalizedKey);
             return NormalizedKey;
        }
    }
    return FuncName;
}

    FString GetCanonicalDefKey(
        const FString& GraphPath, 
        const FString& GraphNameHint, 
        FMarkdownPathTracer::EUserGraphType GraphType)
    {
        FString CanonicalBase;
        
        switch (GraphType) 
        {
            case FMarkdownPathTracer::EUserGraphType::CustomEventGraph:
                // Custom events from same EventGraph need unique DefKeys
                // Use normalized hint to distinguish them
                CanonicalBase = MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(GraphNameHint, GraphType);
                break;
                
            case FMarkdownPathTracer::EUserGraphType::Function:
            case FMarkdownPathTracer::EUserGraphType::Macro: 
            case FMarkdownPathTracer::EUserGraphType::CollapsedGraph:
                // These are unique by asset path, use normalized path as base
                CanonicalBase = MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(GraphPath, GraphType);
                // But fallback to hint if path is empty or problematic
                if (CanonicalBase.IsEmpty() || CanonicalBase == TEXT("/") || CanonicalBase.Contains(TEXT("None")))
                {
                    CanonicalBase = MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(GraphNameHint, GraphType);
                }
                break;
            
            case FMarkdownPathTracer::EUserGraphType::Interface:
            {
                // Interfaces always use interface name for consistency
                // All calls to same interface (regardless of target) get same DefKey
                auto [InterfaceName, FunctionName] = MarkdownTracerUtils::FGraphNameNormalizer::SplitContextAndItem(GraphNameHint);
                if (!InterfaceName.IsEmpty() && !FunctionName.IsEmpty()) {
                    CanonicalBase = InterfaceName + TEXT(":") + FunctionName;
                    UE_LOG(LogPathTracer, Warning, TEXT("Interface DefKey normalization: '%s' → '%s'"), 
                        *GraphNameHint, *CanonicalBase);
                } else {
                    CanonicalBase = MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(GraphNameHint, GraphType);
                }
                break;
            }
            
            default:
                // Unknown types use normalized hint
                CanonicalBase = MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(GraphNameHint, GraphType);
                break;
        }
        
        FString DefKey = CanonicalBase + TEXT("_DEF");
        
        UE_LOG(LogPathTracer, Verbose, TEXT("GetCanonicalDefKey: Path='%s', Hint='%s', Type=%d → DefKey='%s'"), 
            *GraphPath, *GraphNameHint, static_cast<int32>(GraphType), *DefKey);
        
        return DefKey;
    }

    

    FString GetReferenceTypeSuffix(const FString& Category)
{
    static const TMap<FString, FString> ReferenceSuffixMap = {
        {TEXT("object"), TEXT("")},                    // Direct reference - no suffix
        {TEXT("softobject"), TEXT(" (Soft)")},         // Soft reference
        {TEXT("class"), TEXT(" (Class)")},             // Class reference  
        {TEXT("softclass"), TEXT(" (Soft Class)")},    // Soft class reference
        {TEXT("weakobject"), TEXT(" (Weak)")}          // Weak reference
    };
    
    const FString* Suffix = ReferenceSuffixMap.Find(Category);
    return Suffix ? *Suffix : TEXT(""); // Graceful fallback for unknown categories
}


    
    
    
} // namespace MarkdownTracerUtils



FString MarkdownTracerUtils::FGraphNameNormalizer::NormalizeForDefKey(
    const FString& GraphNameHint, 
    FMarkdownPathTracer::EUserGraphType GraphType)
{
    FString Normalized = GraphNameHint;
    
    // Step 1: Normalize separators
    Normalized = NormalizeSeparators(Normalized);
    
    // Step 2: Clean variable affixes
    Normalized = CleanVariableAffixes(Normalized);
    
    // Step 3: Type-specific normalization
    switch (GraphType)
    {
    case FMarkdownPathTracer::EUserGraphType::CustomEventGraph:
        // Custom events need unique names even from same graph
        // Use the full normalized hint
        break;
            
    case FMarkdownPathTracer::EUserGraphType::Function:
    case FMarkdownPathTracer::EUserGraphType::Macro:
    case FMarkdownPathTracer::EUserGraphType::CollapsedGraph:
        // These should be unique by asset path, but we normalize the hint for consistency
        break;
            
    default:
        break;
    }
    
    UE_LOG(LogDataTracer, Verbose, TEXT("NormalizeForDefKey: '%s' → '%s' (Type: %d)"), 
        *GraphNameHint, *Normalized, static_cast<int32>(GraphType));
    
    return Normalized;
}

TTuple<FString, FString> MarkdownTracerUtils::FGraphNameNormalizer::SplitContextAndItem(const FString& GraphNameHint)
{
    FString Context, Item;
    
    // Find the last separator (preference order: :: > : > .)
    int32 SeparatorPos = -1;
    if (GraphNameHint.FindLastChar(TEXT(':'), SeparatorPos) && 
        SeparatorPos > 0 && 
        GraphNameHint[SeparatorPos-1] == TEXT(':'))
    {
        // Found :: separator
        Context = GraphNameHint.Left(SeparatorPos - 1);
        Item = GraphNameHint.Mid(SeparatorPos + 1);
    }
    else if (GraphNameHint.FindLastChar(TEXT(':'), SeparatorPos))
    {
        // Found : separator
        Context = GraphNameHint.Left(SeparatorPos);
        Item = GraphNameHint.Mid(SeparatorPos + 1);
    }
    else if (GraphNameHint.FindLastChar(TEXT('.'), SeparatorPos))
    {
        // Found . separator
        Context = GraphNameHint.Left(SeparatorPos);
        Item = GraphNameHint.Mid(SeparatorPos + 1);
    }
    else
    {
        // No separator found
        Context = TEXT("");
        Item = GraphNameHint;
    }
    
    return MakeTuple(Context.TrimStartAndEnd(), Item.TrimStartAndEnd());
}

FString MarkdownTracerUtils::FGraphNameNormalizer::NormalizeSeparators(const FString& Input)
{
    FString Result = Input;
    
    // Replace :: with :
    Result.ReplaceInline(TEXT("::"), TEXT(":"));
    
    // Replace . with : for consistency
    Result.ReplaceInline(TEXT("."), TEXT(":"));
    
    return Result;
}

FString MarkdownTracerUtils::FGraphNameNormalizer::CleanVariableAffixes(const FString& Input)
{
    FString Result = Input;
    
    // Remove common variable prefixes
    Result.RemoveFromStart(TEXT("Default__"));
    
    // Remove _C suffix
    if (Result.EndsWith(TEXT("_C")))
    {
        Result.LeftChopInline(2);
    }
    
    return Result.TrimStartAndEnd();
}

