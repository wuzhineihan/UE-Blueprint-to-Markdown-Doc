/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/Widgets/SMarkdownOutputWindow.h


#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Models/BlueprintNode.h" // Still useful for FCapturedEventData context
#include "Widgets/SBlueprintExecFlowHTMLWindow.h" // For HTML window type
#include "Framework/Application/SlateApplication.h"
#include "Trace/Generation/GenerationShared.h"


// Forward Declarations
class UEdGraphNode;
class SMultiLineEditableTextBox;
class FExecutionFlowGenerator;

// FCapturedEventData can remain here or be moved to a more common header if used elsewhere
struct FCapturedEventData
{
    FString PreservedCompPropName;
    FString PreservedDelPropName;
    FString BoundEventOwnerClassPath;

    FCapturedEventData() = default;

    explicit FCapturedEventData(TSharedPtr<const FBlueprintNode> Node)
    {
        if (Node.IsValid() && (Node->NodeType == TEXT("ComponentBoundEvent") || Node->NodeType == TEXT("ActorBoundEvent")))
        {
            PreservedCompPropName = Node->PreservedCompPropName;
            PreservedDelPropName = Node->PreservedDelPropName;
            BoundEventOwnerClassPath = Node->BoundEventOwnerClassPath;
            UE_LOG(LogTemp, Log, TEXT("FCapturedEventData: Captured Node %s - Comp='%s', Del='%s', Owner='%s'"),
                   *Node->Guid.Left(8), *PreservedCompPropName, *PreservedDelPropName, *BoundEventOwnerClassPath);
        }
        else
        {
            PreservedCompPropName = TEXT("");
            PreservedDelPropName = TEXT("");
            BoundEventOwnerClassPath = TEXT("");
            if (Node.IsValid()) {
                UE_LOG(LogTemp, Warning, TEXT("FCapturedEventData: Attempted to capture from non-bound-event node type: %s. Data will be empty."), *Node->NodeType);
            } else {
                UE_LOG(LogTemp, Warning, TEXT("FCapturedEventData: Attempted to capture from invalid node. Data will be empty."));
            }
        }
    }
};


class BP2AI_API SMarkdownOutputWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMarkdownOutputWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void UpdateFromSelection();

private:

    // ✅ NEW: Category visibility state for UI
    TMap<EDocumentationGraphCategory, bool> CategoryVisibility;
    // ✅ NEW: Category checkbox widgets (7 categories)
    TSharedPtr<SCheckBox> FunctionsCheckBox;
    TSharedPtr<SCheckBox> CustomEventsCheckBox;
    TSharedPtr<SCheckBox> CollapsedGraphsCheckBox;
    TSharedPtr<SCheckBox> ExecutableMacrosCheckBox;
    TSharedPtr<SCheckBox> PureMacrosCheckBox;
    TSharedPtr<SCheckBox> InterfacesCheckBox;
    TSharedPtr<SCheckBox> PureFunctionsCheckBox;

    // ✅ NEW: Category checkbox handlers
    ECheckBoxState IsCategoryChecked(EDocumentationGraphCategory Category) const;
    void OnCategoryCheckboxChanged(ECheckBoxState NewState, EDocumentationGraphCategory Category);
    
    // ✅ NEW: Cache integration for fast category updates
    void UpdateFromSelectionWithCacheCheck();

    
    // ✅ NEW: Helper to create FGenerationSettings from UI
    FGenerationSettings CreateCurrentSettings() const;
    
    
    // ✅ NEW: Cache management for instant category toggling
    bool bHasCachedResults = false;
    FGenerationSettings LastGenerationSettings;
    
    // ✅ NEW: Category toggle methods
    void CreateCategoryVisibilityUI(TSharedPtr<SVerticalBox> SettingsBox);
    FReply OnCategoryToggleChanged(EDocumentationGraphCategory Category, bool bNewValue);
    void UpdateCategoryCheckboxState();
    
    // ✅ NEW: Smart refresh methods
    void RefreshWithCategorySettings();
    void InvalidateCacheAndRefresh();
    bool CanUseFastPathForRefresh() const;
    
    // ✅ NEW: Settings management
    void InitializeCategoryVisibility();
    FGenerationSettings GetCurrentGenerationSettings() const;
    bool DoesTracingSettingsMatch(const FGenerationSettings& SettingsA, const FGenerationSettings& SettingsB) const;

    // ✅ MODIFY: UpdateSettings method signature (if not already modified)
    void UpdateSettings(bool bInTraceAll, bool bInDefineSeparately,
                       bool bInExpandInline, bool bInShowDefaults);
    
    
    TArray<UEdGraphNode*> GetSelectedBlueprintNodes() const;

    ECheckBoxState IsTraceAllChecked() const;
    void OnTraceAllCheckboxChanged(ECheckBoxState NewState);

    TSharedPtr<SCheckBox> TraceAllSelectedExecCheckBox;
    bool bTraceAllSelectedExec = false;

    TSharedPtr<SCheckBox> DefineUserGraphsSeparatelyCheckBox;
    bool bDefineUserGraphsSeparately = true;

    TSharedPtr<SCheckBox> ExpandCompositesInlineCheckBox;
    bool bExpandCompositesInline = false;
    
    TSharedPtr<SCheckBox> ShowTrivialDefaultsCheckBox;
    bool bShowTrivialDefaultParams = true;

    void OnShowTrivialDefaultsCheckboxChanged(ECheckBoxState NewState);
    ECheckBoxState IsShowTrivialDefaultsChecked() const;
        
    void OnDefineSeparatelyCheckboxChanged(ECheckBoxState NewState);
    ECheckBoxState IsDefineSeparatelyChecked() const;

    void OnExpandInlineCheckboxChanged(ECheckBoxState NewState);
    ECheckBoxState IsExpandInlineChecked() const;

    FReply OnRefreshClicked();
    FReply OnCopyToClipboardClicked();

    TSharedPtr<SMultiLineEditableTextBox> OutputTextBox;
    FString CurrentGeneratedHTML; // To store the HTML output

    TSharedPtr<SWindow> HTMLWindowPtr; // Changed from HTMLWindow to avoid conflict with SWindow member in SCompoundWidget
    TSharedPtr<SBlueprintExecFlowHTMLWindow> HTMLWidget;

    FReply OnOpenHTMLViewClicked();
    void OnFlowInspectorWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed);
};
