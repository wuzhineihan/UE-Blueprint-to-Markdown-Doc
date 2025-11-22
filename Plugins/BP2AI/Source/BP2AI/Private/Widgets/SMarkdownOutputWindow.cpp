/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Widgets/SMarkdownOutputWindow.cpp


#include "Widgets/SMarkdownOutputWindow.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "BlueprintEditor.h"
#include "SGraphPanel.h"
#include "Engine/Blueprint.h"
#include "Logging/LogMacros.h"
#include "SlateOptMacros.h"
#include "Logging/BP2AILog.h"
#include "Styling/AppStyle.h"
#include "Trace/ExecutionFlow/ExecutionFlowGenerator.h"
#include "Trace/MarkdownGenerationContext.h" // Required for FMarkdownGenerationContext
#include "Widgets/SMarkdownOutputWindow.h"
#include "Trace/Generation/GenerationShared.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMarkdownOutputWindow::Construct(const FArguments& InArgs)
{
    // ✅ Initialize category visibility (all visible by default)
    CategoryVisibility.Add(EDocumentationGraphCategory::Functions, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, true);
    CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, true);

    ChildSlot
    [
        SNew(SVerticalBox)

        // Top Toolbar Area
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5.0f)
        [
            SNew(SHorizontalBox) // Main Toolbar Box

            // Refresh Button
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("BlueprintMarkdown", "RefreshExecFlow", "Refresh Flow"))
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "RefreshExecFlowTooltip", "Regenerate execution flow from the active Blueprint editor selection."))
                .OnClicked(this, &SMarkdownOutputWindow::OnRefreshClicked)
            ]

            // Copy Button
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("BlueprintMarkdown", "CopyExecFlow", "Copy Markdown"))
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "CopyExecFlowTooltip", "Copy the generated execution flow Markdown to the clipboard."))
                .OnClicked(this, &SMarkdownOutputWindow::OnCopyToClipboardClicked)
            ]
            
            // HTML View Button
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("BlueprintMarkdown", "FlowInspector", "Flow Inspector"))
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "OpenFlowInspectorTooltip", "Open formatted Flow Inspector in separate window"))
                .OnClicked(this, &SMarkdownOutputWindow::OnOpenHTMLViewClicked)
            ]
            
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [ SNew(SSpacer) ]
        ]

        // ✅ NEW: Tracing Settings Section
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(8.0f)
            [
                SNew(SVerticalBox)
                
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(NSLOCTEXT("BlueprintMarkdown", "TracingSettings", "Tracing Settings"))
                    .Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
                ]
                
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                [
                    SNew(SHorizontalBox)
                    
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0)
                    [
                        SAssignNew(TraceAllSelectedExecCheckBox, SCheckBox)
                        .IsChecked(this, &SMarkdownOutputWindow::IsTraceAllChecked)
                        .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnTraceAllCheckboxChanged)
                        .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "TraceAllTooltip", "If checked, trace execution from *every* selected executable node. If unchecked, prioritize standard entry points (Events, Inputs) if selected."))
                        [
                            SNew(STextBlock)
                            .Text(NSLOCTEXT("BlueprintMarkdown", "TraceAllLabel", "Trace All Selected"))
                        ]
                    ]
            
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0)
                    [
                        SAssignNew(DefineUserGraphsSeparatelyCheckBox, SCheckBox)
                        .IsChecked(this, &SMarkdownOutputWindow::IsDefineSeparatelyChecked)
                        .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnDefineSeparatelyCheckboxChanged)
                        .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "DefineSeparatelyTooltip", "If checked (Default), user Functions/Macros are defined once separately. If unchecked, their calls are just noted."))
                        [
                            SNew(STextBlock)
                            .Text(NSLOCTEXT("BlueprintMarkdown", "DefineSeparatelyLabel", "Define Separately"))
                        ]
                    ]
            
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0)
                    [
                        SAssignNew(ExpandCompositesInlineCheckBox, SCheckBox)
                        .IsChecked(this, &SMarkdownOutputWindow::IsExpandInlineChecked)
                        .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnExpandInlineCheckboxChanged)
                        .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "ExpandInlineTooltip", "If checked, Collapsed Graphs are expanded inline. If unchecked (Default), they are linked/defined separately."))
                        [
                            SNew(STextBlock)
                            .Text(NSLOCTEXT("BlueprintMarkdown", "ExpandInlineLabel", "Expand Inline"))
                        ]
                    ]
                    
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SAssignNew(ShowTrivialDefaultsCheckBox, SCheckBox)
                        .IsChecked(this, &SMarkdownOutputWindow::IsShowTrivialDefaultsChecked)
                        .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnShowTrivialDefaultsCheckboxChanged)
                        .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "ShowTrivialDefaultsTooltip", "If checked, shows input parameters even if they are using their implicit default values."))
                        [
                            SNew(STextBlock)
                            .Text(NSLOCTEXT("BlueprintMarkdown", "ShowTrivialDefaultsLabel", "Show All Defaults"))
                        ]
                    ]
                ]
            ]
        ]

        // ✅ NEW: Category Visibility Section
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(8.0f)
            [
                SNew(SVerticalBox)
                
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(NSLOCTEXT("BlueprintMarkdown", "CategoryVisibility", "Category Visibility"))
                    .Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
                ]
                
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                [
                    SNew(SHorizontalBox)
                    
                    // Left Column
                    + SHorizontalBox::Slot().FillWidth(0.5f).Padding(0, 0, 10, 0)
                    [
                        SNew(SVerticalBox)
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(FunctionsCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::Functions)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::Functions)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::Functions)))
                            ]
                        ]
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(CustomEventsCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::CustomEvents)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::CustomEvents)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::CustomEvents)))
                            ]
                        ]
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(CollapsedGraphsCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::CollapsedGraphs)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::CollapsedGraphs)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::CollapsedGraphs)))
                            ]
                        ]
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(ExecutableMacrosCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::ExecutableMacros)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::ExecutableMacros)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::ExecutableMacros)))
                            ]
                        ]
                    ]
                    
                    // Right Column
                    + SHorizontalBox::Slot().FillWidth(0.5f)
                    [
                        SNew(SVerticalBox)
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(PureMacrosCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::PureMacros)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::PureMacros)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::PureMacros)))
                            ]
                        ]
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(InterfacesCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::Interfaces)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::Interfaces)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::Interfaces)))
                            ]
                        ]
                        
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 2)
                        [
                            SAssignNew(PureFunctionsCheckBox, SCheckBox)
                            .IsChecked(this, &SMarkdownOutputWindow::IsCategoryChecked, EDocumentationGraphCategory::PureFunctions)
                            .OnCheckStateChanged(this, &SMarkdownOutputWindow::OnCategoryCheckboxChanged, EDocumentationGraphCategory::PureFunctions)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CategoryUtils::CategoryToDisplayString(EDocumentationGraphCategory::PureFunctions)))
                            ]
                        ]
                    ]
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
        ]

        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(5.0f)
        [
            SAssignNew(OutputTextBox, SMultiLineEditableTextBox)
            .IsReadOnly(false)
            .AutoWrapText(false)
            .AlwaysShowScrollbars(true) 
            .Font(FAppStyle::GetFontStyle("NormalFontMono"))
        ]
    ];

    UpdateFromSelection();
}


ECheckBoxState SMarkdownOutputWindow::IsTraceAllChecked() const
{
    return bTraceAllSelectedExec ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMarkdownOutputWindow::OnTraceAllCheckboxChanged(ECheckBoxState NewState)
{
    bTraceAllSelectedExec = (NewState == ECheckBoxState::Checked);
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Trace All Selected Exec setting changed to: %s"), bTraceAllSelectedExec ? TEXT("true") : TEXT("false"));
    UpdateFromSelectionWithCacheCheck(); // ✅ Use cache-aware update
}


ECheckBoxState SMarkdownOutputWindow::IsDefineSeparatelyChecked() const
{
    return bDefineUserGraphsSeparately ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMarkdownOutputWindow::OnDefineSeparatelyCheckboxChanged(ECheckBoxState NewState)
{
    bDefineUserGraphsSeparately = (NewState == ECheckBoxState::Checked);
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Define User Graphs Separately setting changed to: %s"), bDefineUserGraphsSeparately ? TEXT("true") : TEXT("false"));
    UpdateFromSelectionWithCacheCheck(); // ✅ Use cache-aware update
}

ECheckBoxState SMarkdownOutputWindow::IsExpandInlineChecked() const
{
    return bExpandCompositesInline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMarkdownOutputWindow::OnExpandInlineCheckboxChanged(ECheckBoxState NewState)
{
    bExpandCompositesInline = (NewState == ECheckBoxState::Checked);
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Expand Collapsed Graphs Inline setting changed to: %s"), bExpandCompositesInline ? TEXT("true") : TEXT("false"));
    UpdateFromSelectionWithCacheCheck(); // ✅ Use cache-aware update
}
ECheckBoxState SMarkdownOutputWindow::IsShowTrivialDefaultsChecked() const
{
    return bShowTrivialDefaultParams ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMarkdownOutputWindow::OnShowTrivialDefaultsCheckboxChanged(ECheckBoxState NewState)
{
    bShowTrivialDefaultParams = (NewState == ECheckBoxState::Checked);
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Show Trivial Default Params setting changed to: %s"), bShowTrivialDefaultParams ? TEXT("true") : TEXT("false"));
    UpdateFromSelectionWithCacheCheck(); // ✅ Use cache-aware update
}
/*
void SBlueprintExecFlowWindow::UpdateFromSelection()
{
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Updating from selection..."));

    TArray<UEdGraphNode*> SelectedEditorNodes = GetSelectedBlueprintNodes();
    if (OutputTextBox.IsValid() && SelectedEditorNodes.Num() == 0)
    {
       OutputTextBox->SetText(NSLOCTEXT("BlueprintMarkdown", "NoNodesSelectedExec", "Select one or more Event or other execution starting nodes in a Blueprint Editor."));
       
       CurrentGeneratedHTML = TEXT("<p style=\"color: #ff8080; padding:10px;\">No nodes selected in Blueprint Editor.</p>");
       if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
       {
           if (FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef()).IsValid())
           {
                HTMLWidget->SetHTMLContent(CurrentGeneratedHTML); 
           }
           else
           {
                HTMLWindowPtr.Reset(); 
                HTMLWidget.Reset();
                UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: HTMLWindowPtr was valid but window not found by Slate during no-node update. Resetting pointers."));
           }
       }
       return;
    }

    // Create settings from UI checkboxes
    FGenerationSettings Settings;
    Settings.bTraceAllSelected = bTraceAllSelectedExec;
    Settings.bDefineUserGraphsSeparately = bDefineUserGraphsSeparately;
    Settings.bExpandCompositesInline = bExpandCompositesInline;
    Settings.bShowTrivialDefaultParams = bShowTrivialDefaultParams;
    Settings.bShouldTraceSymbolicallyForData = false;

    FExecutionFlowGenerator FlowGenerator;

    // 1. Generate Raw Markdown (for text box)
    FMarkdownGenerationContext MarkdownContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    FString RawMarkdownOutput = FlowGenerator.GenerateDocumentForNodes(SelectedEditorNodes, Settings, MarkdownContext);

    if (OutputTextBox.IsValid())
    {
        OutputTextBox->SetText(FText::FromString(RawMarkdownOutput));
    }
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Raw Markdown generated (%d chars)"), RawMarkdownOutput.Len());

    // 2. Generate Complete Interactive HTML Document
    FMarkdownGenerationContext HTMLContext(FMarkdownGenerationContext::EOutputFormat::StyledHTML);
    FString CompleteInteractiveHTML = FlowGenerator.GenerateDocumentForNodes(SelectedEditorNodes, Settings, HTMLContext);

    UE_LOG(LogUI, Display, TEXT("SBlueprintExecFlowWindow: Complete Interactive HTML generated (Length: %d chars)"), CompleteInteractiveHTML.Len());

    // Store the complete HTML document (not just body content)
    CurrentGeneratedHTML = CompleteInteractiveHTML;
    
    // 3. Update HTML window if it's open and valid
    if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
    {
        TSharedPtr<SWindow> FoundWindow = FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef());
        if (FoundWindow.IsValid()) 
        {
            HTMLWidget->SetHTMLContent(CurrentGeneratedHTML);
            HTMLWidget->GenerateAndDisplayInteractiveHTML();
        }
        else
        {
            HTMLWindowPtr.Reset(); 
            HTMLWidget.Reset();
            UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: HTMLWindowPtr was valid but window not found by Slate during content update. Resetting pointers."));
        }
    }
    
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Update complete."));
}

*/



TArray<UEdGraphNode*> SMarkdownOutputWindow::GetSelectedBlueprintNodes() const
{
    TArray<UEdGraphNode*> SelectedNodes;
    if (!GEditor) { UE_LOG(LogUI, Error, TEXT("ExecFlowWindow GetSelectedNodes: GEditor is null.")); return SelectedNodes; }
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) { UE_LOG(LogUI, Warning, TEXT("ExecFlowWindow GetSelectedNodes: AssetEditorSubsystem not found.")); return SelectedNodes; }
    IAssetEditorInstance* MostRecentEditorInstance = nullptr;
    double LastActivationTime = 0.0;
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    for (UObject* Asset : EditedAssets) {
        UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
        if (!Blueprint) continue;
        IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false);
        if (EditorInstance) {
            FName EditorName = EditorInstance->GetEditorName();
            if (EditorName == FName("BlueprintEditor") || EditorName == FName("WidgetBlueprintEditor")) {
                double CurrentActivationTime = EditorInstance->GetLastActivationTime();
                if (CurrentActivationTime >= LastActivationTime) {
                    MostRecentEditorInstance = EditorInstance; LastActivationTime = CurrentActivationTime;
                }
            }
        }
    }
    if (!MostRecentEditorInstance) { UE_LOG(LogUI, Log, TEXT("ExecFlowWindow GetSelectedNodes: No suitable Blueprint editor instance found.")); return SelectedNodes; }
    FBlueprintEditor* TargetBlueprintEditor = static_cast<FBlueprintEditor*>(MostRecentEditorInstance);
    if (!TargetBlueprintEditor) { UE_LOG(LogUI, Error, TEXT("ExecFlowWindow GetSelectedNodes: Failed to cast to FBlueprintEditor.")); return SelectedNodes; }
    FGraphPanelSelectionSet SelectedNodeSet = TargetBlueprintEditor->GetSelectedNodes();
    if (SelectedNodeSet.IsEmpty()) { UE_LOG(LogUI, Log, TEXT("ExecFlowWindow GetSelectedNodes: No nodes selected in the active editor.")); return SelectedNodes; }
    for (UObject* SelectedObject : SelectedNodeSet) { if (UEdGraphNode* GraphNode = Cast<UEdGraphNode>(SelectedObject)) { SelectedNodes.Add(GraphNode); } }
    UE_LOG(LogUI, Log, TEXT("ExecFlowWindow GetSelectedNodes: Found %d selected nodes."), SelectedNodes.Num());
    return SelectedNodes;
}

FReply SMarkdownOutputWindow::OnRefreshClicked()
{
    
    // ✅ Force full regeneration by calling UpdateFromSelection() directly 
    // This bypasses the cache-aware logic and always does a full refresh
    UpdateFromSelection();
    
    return FReply::Handled();
}

FReply SMarkdownOutputWindow::OnCopyToClipboardClicked()
{
    if (OutputTextBox.IsValid()) {
        FString MarkdownText = OutputTextBox->GetText().ToString();
        if (!MarkdownText.IsEmpty()) { FPlatformApplicationMisc::ClipboardCopy(*MarkdownText); }
    }
    return FReply::Handled();
}

FReply SMarkdownOutputWindow::OnOpenHTMLViewClicked()
{
    // Check if the TSharedPtr itself is valid AND if the window it points to still exists
    if (!HTMLWindowPtr.IsValid() || !FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef()))
    {
        // If either check fails, reset both pointers and create a new window
        HTMLWidget.Reset(); 
        HTMLWindowPtr.Reset();

        HTMLWidget = SNew(SBlueprintExecFlowHTMLWindow);
        
        TSharedPtr<SWindow> NewSWindow = SNew(SWindow)
            .Title(NSLOCTEXT("BlueprintMarkdown", "FlowInspectorWindowTitle", "Blueprint Execution Flow - Flow Inspector"))
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(1200, 800))
            .MinWidth(800)
            .MinHeight(600)
            .SupportsMaximize(true)
            .SupportsMinimize(true)
            .HasCloseButton(true)
            .Content()
            [
                HTMLWidget.ToSharedRef()
            ];
        
        HTMLWindowPtr = NewSWindow; 
        HTMLWidget->SetParentWindow(HTMLWindowPtr);
        
        // Add window closed callback to properly reset pointers when user closes the window
        HTMLWindowPtr->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &SMarkdownOutputWindow::OnFlowInspectorWindowClosed));
        
        FSlateApplication::Get().AddWindow(NewSWindow.ToSharedRef());
        
        UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Created new HTML window with category controls."));
    }
    else
    {
        // HTMLWindowPtr is valid and the window exists, bring it to front
        HTMLWindowPtr->BringToFront();
        UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Brought existing HTML window to front."));
    }
    
    if (HTMLWidget.IsValid())
    {
        HTMLWidget->SetHTMLContent(CurrentGeneratedHTML); 
        
        // ✅ Synchronize category settings with HTML window
        HTMLWidget->UpdateSettingsWithCategories(
            bTraceAllSelectedExec,
            bDefineUserGraphsSeparately,
            bExpandCompositesInline,
            bShowTrivialDefaultParams,
            CategoryVisibility  // Pass current category visibility state
        );
        
        // ✅ ALWAYS: Trigger independent HTML trace when Flow Inspector opens
        UE_LOG(LogUI, Log, TEXT("Main Window: Flow Inspector opened, triggering independent trace"));
        HTMLWidget->TriggerInitialTrace();
        
        // ✅ NEW: Set callback for HTML window category changes
        HTMLWidget->SetCategoryChangedCallback([this](const TMap<EDocumentationGraphCategory, bool>& NewCategoryVisibility) {
            UE_LOG(LogUI, Log, TEXT("Main Window: Received category change from HTML window - updating and regenerating"));
            
            // Update main window's category state
            CategoryVisibility = NewCategoryVisibility;
            
            // Trigger regeneration with new category settings
            UpdateFromSelectionWithCacheCheck();
        });
    }
    
    return FReply::Handled();
}
// ✅ NEW: Category checkbox handlers
ECheckBoxState SMarkdownOutputWindow::IsCategoryChecked(EDocumentationGraphCategory Category) const
{
    const bool* VisibilityPtr = CategoryVisibility.Find(Category);
    bool bIsVisible = VisibilityPtr ? *VisibilityPtr : true;
    return bIsVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMarkdownOutputWindow::OnCategoryCheckboxChanged(ECheckBoxState NewState, EDocumentationGraphCategory Category)
{
    bool bNewVisibility = (NewState == ECheckBoxState::Checked);
    CategoryVisibility.Add(Category, bNewVisibility);
    
    FString CategoryName = CategoryUtils::CategoryToDisplayString(Category);
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Category '%s' visibility changed to: %s"), 
        *CategoryName, bNewVisibility ? TEXT("visible") : TEXT("hidden"));
    
    // Use cache-aware update for instant feedback
    UpdateFromSelectionWithCacheCheck();
}

// ✅ Create current settings from UI state
FGenerationSettings SMarkdownOutputWindow::CreateCurrentSettings() const
{
    FGenerationSettings Settings;
    Settings.bTraceAllSelected = bTraceAllSelectedExec;
    Settings.bDefineUserGraphsSeparately = bDefineUserGraphsSeparately;
    Settings.bExpandCompositesInline = bExpandCompositesInline;
    Settings.bShowTrivialDefaultParams = bShowTrivialDefaultParams;
    Settings.bShouldTraceSymbolicallyForData = false;
    
    // Copy category visibility from UI
    Settings.CategoryVisibility = CategoryVisibility;
    
    return Settings;
}


// ✅ Cache-aware update method (if not already implemented)
void SMarkdownOutputWindow::UpdateFromSelectionWithCacheCheck()
{
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: UpdateFromSelectionWithCacheCheck (Context-Aware)..."));

    TArray<UEdGraphNode*> SelectedEditorNodes = GetSelectedBlueprintNodes();
    if (OutputTextBox.IsValid() && SelectedEditorNodes.Num() == 0)
    {
       OutputTextBox->SetText(NSLOCTEXT("BlueprintMarkdown", "NoNodesSelectedExec", "Select one or more Event or other execution starting nodes in a Blueprint Editor."));
       
       CurrentGeneratedHTML = TEXT("<p style=\"color: #ff8080; padding:10px;\">No nodes selected in Blueprint Editor.</p>");
       if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
       {
           if (FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef()).IsValid())
           {
                HTMLWidget->SetHTMLContent(CurrentGeneratedHTML); 
           }
           else
           {
                HTMLWindowPtr.Reset(); 
                HTMLWidget.Reset();
           }
       }
       return;
    }

    FGenerationSettings Settings = CreateCurrentSettings();
    static FExecutionFlowGenerator FlowGenerator; // Keep static for cache persistence

    UE_LOG(LogUI, Log, TEXT("🎯 FAST: Generating only Markdown for main window"));
    
    // ✅ FAST: Generate only Markdown for main window (Flow Inspector will trace independently)
    FMarkdownGenerationContext MarkdownContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    FString RawMarkdownOutput = FlowGenerator.GenerateDocumentForNodes(SelectedEditorNodes, Settings, MarkdownContext);

    if (OutputTextBox.IsValid())
    {
        OutputTextBox->SetText(FText::FromString(RawMarkdownOutput));
    }

    // ✅ FAST: No HTML generation here - Flow Inspector will handle its own tracing
    CurrentGeneratedHTML.Empty(); // Ensure it's always empty so Flow Inspector always traces
    
    UE_LOG(LogUI, Log, TEXT("🎯 FAST: Markdown generation complete (%d chars)"), 
        RawMarkdownOutput.Len());
    
    // Update HTML window if open (coordinated refresh)
    if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
    {
        TSharedPtr<SWindow> FoundWindow = FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef());
        if (FoundWindow.IsValid()) 
        {
            // ✅ COORDINATED: Trigger independent HTML trace in Flow Inspector
            UE_LOG(LogUI, Log, TEXT("Main Window: Coordinated refresh - triggering Flow Inspector trace"));
            HTMLWidget->RefreshFromMainWindow();
        }
        else
        {
            HTMLWindowPtr.Reset(); 
            HTMLWidget.Reset();
        }
    }
    
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Context-aware cache-aware update complete."));
}


// ✅ ENHANCED: Update the existing UpdateFromSelection method to use cache-aware logic
void SMarkdownOutputWindow::UpdateFromSelection()
{
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: UpdateFromSelection (forced full refresh with contexts)"));

    TArray<UEdGraphNode*> SelectedEditorNodes = GetSelectedBlueprintNodes();
    if (OutputTextBox.IsValid() && SelectedEditorNodes.Num() == 0)
    {
       OutputTextBox->SetText(NSLOCTEXT("BlueprintMarkdown", "NoNodesSelectedExec", "Select one or more Event or other execution starting nodes in a Blueprint Editor."));
       
       CurrentGeneratedHTML = TEXT("<p style=\"color: #ff8080; padding:10px;\">No nodes selected in Blueprint Editor.</p>");
       if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
       {
           if (FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef()).IsValid())
           {
                HTMLWidget->SetHTMLContent(CurrentGeneratedHTML); 
           }
           else
           {
                HTMLWindowPtr.Reset(); 
                HTMLWidget.Reset();
           }
       }
       return;
    }

    FGenerationSettings Settings = CreateCurrentSettings();
    static FExecutionFlowGenerator FlowGenerator; // Keep static for cache persistence

    // ✅ FORCE CACHE INVALIDATION for fresh generation
    FlowGenerator.InvalidateCache();
    UE_LOG(LogUI, Log, TEXT("🔄 FORCED REGENERATION: Cache invalidated, fresh Markdown generation"));
    
    // ✅ Generate only Markdown with proper context
    FMarkdownGenerationContext MarkdownContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    FString RawMarkdownOutput = FlowGenerator.GenerateDocumentForNodes(SelectedEditorNodes, Settings, MarkdownContext);

    if (OutputTextBox.IsValid())
    {
        OutputTextBox->SetText(FText::FromString(RawMarkdownOutput));
    }

    // ✅ FAST: No HTML generation here - Flow Inspector will handle its own tracing
    CurrentGeneratedHTML.Empty(); // Ensure it's always empty so Flow Inspector always traces
    
    UE_LOG(LogUI, Log, TEXT("🔄 FORCED REGENERATION: Complete (Markdown: %d chars)"), 
        RawMarkdownOutput.Len());
    
    // Update HTML window if open (coordinated refresh)
    if (HTMLWidget.IsValid() && HTMLWindowPtr.IsValid())
    {
        TSharedPtr<SWindow> FoundWindow = FSlateApplication::Get().FindWidgetWindow(HTMLWindowPtr.ToSharedRef());
        if (FoundWindow.IsValid()) 
        {
            // ✅ COORDINATED: Update settings and trigger independent HTML trace
            HTMLWidget->UpdateSettingsWithCategories(
                bTraceAllSelectedExec,
                bDefineUserGraphsSeparately,
                bExpandCompositesInline,
                bShowTrivialDefaultParams,
                CategoryVisibility
            );
            
            UE_LOG(LogUI, Log, TEXT("Main Window: Coordinated refresh (full) - triggering Flow Inspector trace"));
            HTMLWidget->RefreshFromMainWindow();
        }
        else
        {
            HTMLWindowPtr.Reset(); 
            HTMLWidget.Reset();
        }
    }
    
    UE_LOG(LogUI, Log, TEXT("SBlueprintExecFlowWindow: Forced full regeneration with contexts complete."));
}

void SMarkdownOutputWindow::OnFlowInspectorWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
    UE_LOG(LogUI, Log, TEXT("SMarkdownOutputWindow: OnFlowInspectorWindowClosed Called. Resetting HTML window pointers."));
    
    if (WindowBeingClosed == HTMLWindowPtr)
    {
        // Clear the callback to prevent orphaned references
        if (HTMLWidget.IsValid())
        {
            HTMLWidget->SetCategoryChangedCallback(nullptr);
        }
        
        // Reset the pointers
        HTMLWidget.Reset();
        HTMLWindowPtr.Reset();
        
        // Clear the stored HTML content to force regeneration on next open
        CurrentGeneratedHTML.Empty();
        
        UE_LOG(LogUI, Log, TEXT("SMarkdownOutputWindow: Flow Inspector window pointers reset successfully."));
    }
    else
    {
        UE_LOG(LogUI, Warning, TEXT("SMarkdownOutputWindow: OnFlowInspectorWindowClosed called for an unexpected window."));
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION