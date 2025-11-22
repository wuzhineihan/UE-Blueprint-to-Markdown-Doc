/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/BlueprintAnalysisUtils.cpp

#include "Trace/Utils/BlueprintAnalysisUtils.h" // Adjust to your actual path

// Core UObject system headers
#include "UObject/Object.h"
#include "UObject/Class.h"        // Brings in UClass, UFunction, TFieldIterator, etc.
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

// Engine specific headers
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h" // For UBlueprintGeneratedClass

// Editor specific headers (if needed for editor-only functionality)
#include "EditorFramework/AssetImportData.h" // Example for UAssetImportData
#if WITH_EDITOR // Guard editor-only includes
#include "Kismet2/BlueprintEditorUtils.h" // For FBlueprintEditorUtils if used
#endif

// For logging
#include "Logging/LogMacros.h"

// Define a log category if you don't have one for your trace utilities
// If you already have one (e.g., LogPathTracer from previous examples), you can use that.
// For a quick test, LogTemp is fine and always available.
// DEFINE_LOG_CATEGORY_STATIC(LogBlueprintAnalysis, Log, All); // Or use an existing one

void FBlueprintAnalysisUtils::LogBlueprintFunctionAnalysis(const TArray<UObject*>& SelectedAssets)
{
    UE_LOG(LogTemp, Display, TEXT("--- Starting Blueprint Function Analysis ---"));

    if (SelectedAssets.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No assets provided for analysis."));
        UE_LOG(LogTemp, Display, TEXT("--- Blueprint Function Analysis Complete (No Assets) ---"));
        return;
    }

    for (UObject* SelectedAsset : SelectedAssets)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(SelectedAsset);
        if (!Blueprint)
        {
            UE_LOG(LogTemp, Log, TEXT("Skipping asset '%s' as it is not a UBlueprint."), *SelectedAsset->GetFullName());
            continue;
        }

        // Forcing a load if it's just an asset data can sometimes be useful, but GetGeneratedClass() handles this often.
        // Blueprint->ConditionalPostLoad(); // This can be heavy, use with caution.

        UClass* GeneratedBPClass = Blueprint->GeneratedClass; // More direct way to get it.
                                                             // Blueprint->ParentClass is the C++ parent.

        UE_LOG(LogTemp, Display, TEXT("Analyzing Blueprint: %s (Path: %s)"), *Blueprint->GetName(), *Blueprint->GetPathName());

        if (Blueprint->BlueprintType == BPTYPE_Interface)
        {
            UE_LOG(LogTemp, Log, TEXT("  [TYPE]: This IS an Interface Blueprint (BPI)."));
            if (GeneratedBPClass)
            {
                for (TFieldIterator<UFunction> FuncIt(GeneratedBPClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
                {
                    UFunction* Function = *FuncIt;
                    if (Function && Function->GetOuter() == GeneratedBPClass)
                    {
                         UE_LOG(LogTemp, Log, TEXT("    L Interface Function Definition (in BPI): %s"), *Function->GetName());
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("    Interface Blueprint '%s' has no GeneratedClass. Functions cannot be listed."), *Blueprint->GetName());
            }
        }
        else if (Blueprint->BlueprintType == BPTYPE_Normal || 
                 Blueprint->BlueprintType == BPTYPE_FunctionLibrary || 
                 Blueprint->BlueprintType == BPTYPE_MacroLibrary)
        {
            FString BPTypeStr;
            switch(Blueprint->BlueprintType)
            {
                case BPTYPE_Normal: BPTypeStr = TEXT("Normal"); break;
                case BPTYPE_FunctionLibrary: BPTypeStr = TEXT("Function Library"); break;
                case BPTYPE_MacroLibrary: BPTypeStr = TEXT("Macro Library"); break;
                default: BPTypeStr = TEXT("Other Regular"); break; // Should not happen given the if condition
            }

            UE_LOG(LogTemp, Log, TEXT("  [TYPE]: This is a %s Blueprint."), *BPTypeStr);

            if (!GeneratedBPClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("    Blueprint '%s' has no GeneratedClass. Skipping function analysis."), *Blueprint->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("    Implemented Interfaces (from Blueprint->ImplementedInterfaces property):"));
                if (Blueprint->ImplementedInterfaces.Num() == 0)
                {
                    UE_LOG(LogTemp, Log, TEXT("      (None directly declared on Blueprint asset property)"));
                }
                for (const FBPInterfaceDescription& InterfaceDesc : Blueprint->ImplementedInterfaces)
                {
                    if (InterfaceDesc.Interface) // This is a UClass* that should be an Interface class
                    {
                        UE_LOG(LogTemp, Log, TEXT("      - %s (Referenced by Blueprint asset)"), *InterfaceDesc.Interface->GetName());
                    }
                }
                
                UE_LOG(LogTemp, Log, TEXT("    Implemented Interfaces (from GeneratedClass->Interfaces array):"));
                 if (GeneratedBPClass->Interfaces.Num() == 0)
                {
                    UE_LOG(LogTemp, Log, TEXT("      (None found on GeneratedClass->Interfaces array)"));
                }
                for(const FImplementedInterface& ImplementedInterface : GeneratedBPClass->Interfaces)
                {
                    if(ImplementedInterface.Class) // This is the UClass* of the interface
                    {
                        UE_LOG(LogTemp, Log, TEXT("      - %s (From GeneratedClass, Implemented by K2: %s)"), 
                            *ImplementedInterface.Class->GetName(), 
                            ImplementedInterface.bImplementedByK2 ? TEXT("Yes") : TEXT("No"));
                    }
                }

                UE_LOG(LogTemp, Log, TEXT("    Functions declared in this Blueprint's class (%s):"), *GeneratedBPClass->GetName());
                for (TFieldIterator<UFunction> FuncIt(GeneratedBPClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
                {
                    UFunction* Function = *FuncIt;
                    // Ensure the function is directly part of this Blueprint's generated class, not from a C++ parent class
                    if (Function && Function->GetOuter() == GeneratedBPClass) 
                    {
                        FString ImplementedInterfaceName;
                        bool bIsInterfaceImpl = IsInterfaceImplementation(Blueprint, Function, ImplementedInterfaceName);

                        FString PurityString = Function->HasAllFunctionFlags(FUNC_BlueprintPure) ? TEXT("(Pure)") : TEXT("");
                        FString FlagsString = FString::Printf(TEXT("Flags: 0x%08X"), Function->FunctionFlags);
                        if(Function->HasAllFunctionFlags(FUNC_Event)) FlagsString += TEXT(" EVENT");
                        if(Function->HasAllFunctionFlags(FUNC_BlueprintEvent)) FlagsString += TEXT(" BPEVENT"); // Often comes with EVENT for BP interfaces
                        if(Function->HasAllFunctionFlags(FUNC_Net)) FlagsString += TEXT(" NET"); // Could be RPC related
                        if(Function->HasAllFunctionFlags(FUNC_Native)) FlagsString += TEXT(" NATIVE"); // C++ base exists

                        if (bIsInterfaceImpl)
                        {
                            UE_LOG(LogTemp, Log, TEXT("      L Function: '%s' %s - IS an IMPLEMENTATION of interface: '%s'. %s"), 
                                *Function->GetName(), *PurityString, *ImplementedInterfaceName, *FlagsString);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Log, TEXT("      L Function: '%s' %s - Is a regular Blueprint function/event. %s"), 
                                *Function->GetName(), *PurityString, *FlagsString);
                        }
                    }
                }
            }
        }
        else
        {
            // Example of how to get enum as string:
            // const UEnum* BPTypeEnum = StaticEnum<EBlueprintType>();
            // FString BPTypeEnumName = BPTypeEnum ? BPTypeEnum->GetNameStringByValue(static_cast<int64>(Blueprint->BlueprintType)) : FString::FromInt(static_cast<int32>(Blueprint->BlueprintType));
            // UE_LOG(LogTemp, Log, TEXT("  [TYPE]: This is another Blueprint type: %s"), *BPTypeEnumName);
             UE_LOG(LogTemp, Log, TEXT("  [TYPE]: This is another Blueprint type (ID: %d)"), static_cast<int32>(Blueprint->BlueprintType));
        }
        UE_LOG(LogTemp, Display, TEXT("--- End of Blueprint: %s ---"), *Blueprint->GetName());
    }
    UE_LOG(LogTemp, Display, TEXT("--- Blueprint Function Analysis Complete ---"));
}

bool FBlueprintAnalysisUtils::IsInterfaceImplementation(const UBlueprint* Blueprint, const UFunction* Function, FString& OutImplementedInterfaceName)
{
    if (!Blueprint || !Function || !Blueprint->GeneratedClass)
    {
        return false;
    }

    UClass* BPClass = Blueprint->GeneratedClass;

    // Only consider functions that are Blueprint events, as these are how BP implements interfaces.
    // Or if they are native and overridden (but that's harder to check simply here without signature matching)
    if (!Function->HasAnyFunctionFlags(FUNC_Event | FUNC_Native)) // Native for C++ overrides, Event for BP overrides
    {
        // If it's not an Event or a Native func, it's unlikely to be a direct BP interface implementation
        // (unless it's a pure C++ override of a native interface func without BlueprintNativeEvent/ImplementableEvent,
        // which would usually not appear in the TFieldIterator<UFunction>(BPClass, EFieldIteratorFlags::ExcludeSuper)
        // if it's purely from a C++ parent).
        // However, if a C++ parent implements an interface and this BP function overrides THAT C++ function,
        // then it's more complex.
        // Let's focus on functions that *could* be Blueprint interface implementations first.
        // A regular BP function cannot "implement" an interface message in the same way an event does.
    }

    // Iterate through the interfaces actually implemented by the GeneratedClass
    // BPClass->Interfaces contains FImplementedInterface structs.
    for (const FImplementedInterface& ImplementedInterfaceInfo : BPClass->Interfaces)
    {
        if (UClass* InterfaceClass = ImplementedInterfaceInfo.Class)
        {
            // Check if the InterfaceClass itself declares a function with the same FName as our current Function.
            // Using FindFunctionByName directly on the InterfaceClass.
            UFunction* FunctionInInterface = InterfaceClass->FindFunctionByName(Function->GetFName());

            if (FunctionInInterface)
            {
                // If a function with the same name exists in the interface,
                // and our current function is marked as an Event (typical for BP implementations),
                // it's a strong candidate.
                // FUNC_BlueprintEvent is specifically for events that are implementations of blueprint interface functions
                // or overridable functions from a native C++ parent.
                if (Function->HasAllFunctionFlags(FUNC_BlueprintEvent)) // This is a key flag!
                {
                    OutImplementedInterfaceName = InterfaceClass->GetName();
                    return true;
                }
                
                // Handle BlueprintNativeEvent: if Function is MyFunc_Implementation and MyFunc is in Interface
                FString CurrentFuncName = Function->GetName();
                if (CurrentFuncName.EndsWith(TEXT("_Implementation")))
                {
                    FName BaseName = FName(CurrentFuncName.LeftChop(15)); // Length of "_Implementation"
                    if (InterfaceClass->FindFunctionByName(BaseName) != nullptr) {
                        OutImplementedInterfaceName = InterfaceClass->GetName();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


