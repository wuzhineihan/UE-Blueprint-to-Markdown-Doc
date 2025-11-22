/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Widgets/PropertyDumperWidget.cpp



// Include module private PCH before any other includes


// Then the class header
#include "Widgets/PropertyDumperWidget.h" 

#include "Trace/Utils/BlueprintAnalysisUtils.h"
#include "EditorUtilityLibrary.h"

#include "Logging/BP2AILog.h"
// Engine headers next
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Engine/Engine.h"
#include "Engine/World.h"  // Additional engine include
#include "Logging/LogMacros.h"
#include "Misc/DateTime.h"
#include "PropertyAccess.h"  // Full property access system
#include "UObject/Class.h"
#include "UObject/Field.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"  // For NAME_* constants
#include "UObject/Package.h"        // For FindObject
#include "UObject/PropertyTag.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"     // For FProperty, etc.
#include "Editor.h"                         // For GEditor
#include "Subsystems/AssetEditorSubsystem.h" // For UAssetEditorSubsystem
#include "BlueprintEditor.h"                 // For FBlueprintEditor
#include "EdGraph/EdGraphNode.h"             // For UEdGraphNode
#include "EdGraph/EdGraphPin.h"              // For UEdGraphPin
#include "SGraphPanel.h"                     // For FGraphPanelSelectionSet
#include "Engine/Blueprint.h"


// Define NAME_* constants manually if they aren't available
#ifndef NAME_ClassProperty
#define NAME_ClassProperty FName(TEXT("ClassProperty"))
#endif

#ifndef NAME_WeakObjectProperty
#define NAME_WeakObjectProperty FName(TEXT("WeakObjectProperty"))
#endif

#ifndef NAME_SoftClassProperty
#define NAME_SoftClassProperty FName(TEXT("SoftClassProperty"))
#endif

// Define the log category locally as a fallback
#ifndef LogBP2AI
#define LogBP2AI LogUI
#endif

#if !UE_BUILD_SHIPPING
#include "Test/CurrentPhaseTest.h" // Adjust path if necessary
#endif


void UPropertyDumperWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (DumpPropertiesButton)
    {
        DumpPropertiesButton->OnClicked.AddDynamic(this, &UPropertyDumperWidget::OnDumpPropertiesClicked);
    }
    if (InputClassPathsTextBox)
    {
        InputClassPathsTextBox->SetText(FText::FromString(GetInitialClassPaths()));
    }

    if (DumpSelectedNodePinsButton) // <<< NEW
    {                                     // <<< NEW
        DumpSelectedNodePinsButton->OnClicked.AddDynamic(this, &UPropertyDumperWidget::OnDumpSelectedNodePinsClicked); // <<< NEW
    }                             
    if (AnalyzeBPFunctionsButton)
    {
        AnalyzeBPFunctionsButton->OnClicked.AddDynamic(this, &UPropertyDumperWidget::OnAnalyzeBPFunctionsButtonClicked);
    }
    if (RunCurrentPhaseTestButton)
    {
        RunCurrentPhaseTestButton->OnClicked.AddDynamic(this, &UPropertyDumperWidget::OnRunCurrentPhaseTestClicked);
    }
    
    // Define relevant property types
    RelevantPropertyTypes.Add(NAME_ObjectProperty);
    RelevantPropertyTypes.Add(NAME_ClassProperty);
    RelevantPropertyTypes.Add(NAME_StructProperty);
    RelevantPropertyTypes.Add(NAME_NameProperty);
    RelevantPropertyTypes.Add(NAME_StrProperty);
    RelevantPropertyTypes.Add(NAME_IntProperty);
    RelevantPropertyTypes.Add(NAME_FloatProperty);
    RelevantPropertyTypes.Add(NAME_BoolProperty);
    RelevantPropertyTypes.Add(NAME_ByteProperty);
    RelevantPropertyTypes.Add(NAME_EnumProperty);
    RelevantPropertyTypes.Add(NAME_DelegateProperty);
    RelevantPropertyTypes.Add(NAME_ArrayProperty);
    RelevantPropertyTypes.Add(NAME_WeakObjectProperty);
    RelevantPropertyTypes.Add(NAME_SoftObjectProperty);
    RelevantPropertyTypes.Add(NAME_SoftClassProperty);
    RelevantPropertyTypes.Add(NAME_InterfaceProperty);
}

void UPropertyDumperWidget::OnDumpPropertiesClicked()
{
    if (!InputClassPathsTextBox || !OutputResultsTextBox)
    {
        UE_LOG(LogUI, Warning, TEXT("Property Dumper UI elements not bound correctly."));
        return;
    }

    FString InputPaths = InputClassPathsTextBox->GetText().ToString();
    TArray<FString> ClassPaths;
    InputPaths.ParseIntoArrayLines(ClassPaths, true);

    FString Results;
    Results.Append(FString::Printf(TEXT("Property Dump Results (%s):\n================================\n"), *FDateTime::Now().ToString()));

    for (const FString& PathString : ClassPaths)
    {
        FString CleanPathString = PathString.TrimStartAndEnd();
        if (CleanPathString.IsEmpty()) continue;

        UClass* FoundClass = FindObject<UClass>(nullptr, *CleanPathString);
        if (!FoundClass)
        {
            // Fix for Ensure error - use the enum correctly
            FoundClass = UClass::TryFindTypeSlow<UClass>(CleanPathString, EFindFirstObjectOptions::EnsureIfAmbiguous);
        }

        if (FoundClass)
        {
            Results.Append(FString::Printf(TEXT("\n--- Class: %s (%s) ---\n"), *FoundClass->GetName(), *CleanPathString));
            UE_LOG(LogUI, Log, TEXT("Processing Class: %s"), *FoundClass->GetName());

            for (TFieldIterator<FProperty> PropIt(FoundClass, EFieldIterationFlags::IncludeSuper); PropIt; ++PropIt)
            {
                FProperty* Property = *PropIt;
                if (Property)
                {
                    FName PropertyTypeName = Property->GetClass()->GetFName();
                    if (!RelevantPropertyTypes.Contains(PropertyTypeName))
                    {
                        continue;
                    }

                    FString PropName = Property->GetName();
                    FString PropType = PropertyTypeName.ToString();

                    // Get DisplayNameText (Editor-only but fine for an Editor Utility Widget)
                    FText DisplayNameText = Property->GetDisplayNameText();
                    FString DisplayNameString = DisplayNameText.ToString();
                    if (DisplayNameString.IsEmpty() || DisplayNameString == PropName)
                    {
                        DisplayNameString = TEXT("(Same as Internal Name or None)");
                    }
                    
                    // Get MetaData "DisplayName"
                    FString MetaDisplayName = Property->GetMetaData(TEXT("DisplayName"));
                    if (MetaDisplayName.IsEmpty())
                    {
                        MetaDisplayName = TEXT("(None)");
                    }

                    FString PropDetails = "";
                    if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Property)) {
                        PropDetails = FString::Printf(TEXT(" (ObjectClass: %s)"), ObjectProp->PropertyClass ? *ObjectProp->PropertyClass->GetName() : TEXT("None"));
                    } else if (FStructProperty* StructProp = CastField<FStructProperty>(Property)) { 
                        PropDetails = FString::Printf(TEXT(" (StructType: %s)"), StructProp->Struct ? *StructProp->Struct->GetName() : TEXT("None"));
                    }
                    // Add more CastField checks here for other FProperty types if you want more specific details

                    Results.Append(FString::Printf(TEXT("  - Internal Name: %s\n"), *PropName));
                    Results.Append(FString::Printf(TEXT("    Type: %s%s\n"), *PropType, *PropDetails));
                    Results.Append(FString::Printf(TEXT("    DisplayNameText: %s\n"), *DisplayNameString));
                    Results.Append(FString::Printf(TEXT("    MetaData(DisplayName): %s\n\n"), *MetaDisplayName));
                }
            
            }
        }
        else
        {
            Results.Append(FString::Printf(TEXT("\n--- Class: %s (NOT FOUND) ---\n"), *CleanPathString));
            UE_LOG(LogUI, Warning, TEXT("Could not find class: %s"), *CleanPathString);
        }
    }

    OutputResultsTextBox->SetText(FText::FromString(Results));
    UE_LOG(LogUI, Log, TEXT("Property dump complete."));
}


FString UPropertyDumperWidget::GetInitialClassPaths() const
{
    TArray<FString> InitialPaths;
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_GetClassDefaults"));
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_Timeline"));
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_DynamicCast"));
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_CallArrayFunction"));
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_CreateDelegate"));
    InitialPaths.Add(TEXT("/Script/Engine.KismetMathLibrary")); // Example: Check properties of a common static library
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_PromotableOperator")); // Verify FunctionReference/OperationName properties
    InitialPaths.Add(TEXT("/Script/BlueprintGraph.K2Node_BreakStruct")); // Verify StructType property/accessors

    return FString::Join(InitialPaths, TEXT("\n"));
}


void UPropertyDumperWidget::OnDumpSelectedNodePinsClicked()
{
    if (!OutputResultsTextBox)
    {
        UE_LOG(LogUI, Warning, TEXT("PropertyDumperWidget: OutputResultsTextBox not bound."));
        return;
    }

    FString Results;
    Results.Append(FString::Printf(TEXT("Selected Node Pin Dump Results (%s):\n================================\n"), *FDateTime::Now().ToString()));

    if (!GEditor)
    {
        Results.Append(TEXT("GEditor is null. Cannot get active Blueprint editor.\n"));
        OutputResultsTextBox->SetText(FText::FromString(Results));
        return;
    }

    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem)
    {
        Results.Append(TEXT("AssetEditorSubsystem not found.\n"));
        OutputResultsTextBox->SetText(FText::FromString(Results));
        return;
    }

    // --- NEW LOGIC TO FIND THE MOST RECENTLY ACTIVE BLUEPRINT EDITOR ---
    IAssetEditorInstance* MostRecentBlueprintEditorInstance = nullptr;
    double LastActivationTime = 0.0;

    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    for (UObject* Asset : EditedAssets)
    {
        UBlueprint* BlueprintAsset = Cast<UBlueprint>(Asset);
        if (!BlueprintAsset) continue; // Skip non-blueprint assets

        IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(BlueprintAsset, false); // Don't force focus
        if (EditorInstance)
        {
            // Check if it's actually a Blueprint Editor by name
            if (EditorInstance->GetEditorName() == FName("BlueprintEditor"))
            {
                double CurrentActivationTime = EditorInstance->GetLastActivationTime();
                if (CurrentActivationTime >= LastActivationTime) 
                {
                    MostRecentBlueprintEditorInstance = EditorInstance;
                    LastActivationTime = CurrentActivationTime;
                }
            }
        }
    }

    FBlueprintEditor* BlueprintEditor = MostRecentBlueprintEditorInstance ? static_cast<FBlueprintEditor*>(MostRecentBlueprintEditorInstance) : nullptr;
    // --- END NEW LOGIC ---

    if (!BlueprintEditor)
    {
        Results.Append(TEXT("No active Blueprint editor found or the active editor is not a Blueprint Editor.\n"));
        OutputResultsTextBox->SetText(FText::FromString(Results));
        return;
    }

    FGraphPanelSelectionSet SelectedNodes = BlueprintEditor->GetSelectedNodes();
    if (SelectedNodes.IsEmpty())
    {
        Results.Append(TEXT("No nodes selected in the active Blueprint editor.\n"));
        OutputResultsTextBox->SetText(FText::FromString(Results));
        return;
    }

    for (UObject* SelectedObject : SelectedNodes)
    {
        UEdGraphNode* GraphNode = Cast<UEdGraphNode>(SelectedObject);
        if (GraphNode)
        {
            Results.Append(FString::Printf(TEXT("\n--- Node: %s (GUID: %s, Class: %s) ---\n"),
                *GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
                *GraphNode->NodeGuid.ToString(),
                *GraphNode->GetClass()->GetName()));

            if (GraphNode->Pins.Num() == 0)
            {
                Results.Append(TEXT("  No pins on this node.\n"));
            } else {
                for (UEdGraphPin* PinRef : GraphNode->Pins)
                {
                    if (PinRef)
                    {
                        Results.Append(FString::Printf(TEXT("  Pin:\n")));
                        Results.Append(FString::Printf(TEXT("    Internal Name (PinName): %s\n"), *PinRef->PinName.ToString()));
                        Results.Append(FString::Printf(TEXT("    Friendly Name (PinFriendlyName): %s\n"), *PinRef->PinFriendlyName.ToString()));
                        Results.Append(FString::Printf(TEXT("    Direction: %s\n"), (PinRef->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"))));
                        Results.Append(FString::Printf(TEXT("    Category: %s\n"), *PinRef->PinType.PinCategory.ToString()));
                        Results.Append(FString::Printf(TEXT("    SubCategory: %s\n"), *PinRef->PinType.PinSubCategory.ToString()));
                        Results.Append(FString::Printf(TEXT("    SubCategoryObject: %s\n"), PinRef->PinType.PinSubCategoryObject.IsValid() ? *PinRef->PinType.PinSubCategoryObject->GetPathName() : TEXT("None")));
                        Results.Append(FString::Printf(TEXT("    DefaultValue: '%s'\n"), *PinRef->DefaultValue));
                        Results.Append(FString::Printf(TEXT("    DefaultObject: %s\n"), PinRef->DefaultObject ? *PinRef->DefaultObject->GetPathName() : TEXT("None")));
                        Results.Append(FString::Printf(TEXT("    IsLinked: %s\n"), PinRef->LinkedTo.Num() > 0 ? TEXT("Yes") : TEXT("No")));
                    }
                }
            }
        }
    }
    OutputResultsTextBox->SetText(FText::FromString(Results));
    UE_LOG(LogUI, Log, TEXT("Selected node pin dump complete."));
}

void UPropertyDumperWidget::OnAnalyzeBPFunctionsButtonClicked()
{
    UE_LOG(LogBP2AI, Log, TEXT("Analyze Selected BP Functions button clicked.")); // Use your existing log category

    // Get selected assets from the Content Browser
    // This requires the "EditorScriptingUtilities" plugin to be enabled for your project,
    // and you'll need to add "EditorScriptingUtilities" to your module's .Build.cs PrivateDependencyModuleNames
    // For quick testing, UEditorUtilityLibrary::GetSelectedAssets() is often available in editor utility contexts.
    TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();

    if (SelectedAssets.Num() > 0)
    {
        FBlueprintAnalysisUtils::LogBlueprintFunctionAnalysis(SelectedAssets); // Call your static utility function
        if (OutputResultsTextBox)
        {
            // You can also put a summary or confirmation in your text box
            OutputResultsTextBox->SetText(FText::FromString(FString::Printf(TEXT("Blueprint function analysis initiated for %d assets. Check Output Log."), SelectedAssets.Num())));
        }
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("No assets selected in Content Browser. Please select Blueprint assets to analyze."));
        if (OutputResultsTextBox)
        {
            OutputResultsTextBox->SetText(FText::FromString(TEXT("No assets selected in Content Browser. Please select one or more Blueprint assets and try again.")));
        }
    }
}


void UPropertyDumperWidget::OnRunCurrentPhaseTestClicked()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogUI, Warning, TEXT("PropertyDumperWidget: 'Run Current Phase Test' button clicked."));
    if (OutputResultsTextBox)
    {
        // Clear previous results or indicate test is running
        OutputResultsTextBox->SetText(FText::FromString(FString::Printf(TEXT("Executing current phase test (%s)... Check Output Log."),
            *FDateTime::Now().ToString())));
    }

    // Call the centralized test function
    BP2AITests::ExecuteCurrentPhaseTest();

    if (OutputResultsTextBox)
    {
        // Optionally update with a completion message, though primary output is the log
        OutputResultsTextBox->SetText(FText::FromString(FString::Printf(TEXT("Current phase test execution finished (%s). See Output Log for details."),
            *FDateTime::Now().ToString())));
    }
#else
    UE_LOG(LogUI, Warning, TEXT("PropertyDumperWidget: Test button clicked, but tests are disabled in shipping builds."));
    if (OutputResultsTextBox)
    {
        OutputResultsTextBox->SetText(FText::FromString(TEXT("Test execution is disabled in shipping builds.")));
    }
#endif
}

