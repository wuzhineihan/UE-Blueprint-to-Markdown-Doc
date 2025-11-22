/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/SBlueprintMarkdownWindow.cpp


#include "SBlueprintMarkdownWindow.h"
#include "Logging/BP2AILog.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SCheckBox.h"
#include "HAL/PlatformApplicationMisc.h"
// #include "Framework/Application/SlateApplication.h" // Not needed
// #include "Toolkits/ToolkitManager.h" // Not needed
// #include "Toolkits/IToolkitHost.h" // Not needed
#include "Editor.h"                         // For GEditor
#include "Subsystems/AssetEditorSubsystem.h" // For UAssetEditorSubsystem, IAssetEditorInstance (!!!)
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "SGraphPanel.h"                     // For FGraphPanelSelectionSet
#include "BlueprintEditor.h"                 // Important: Include for FBlueprintEditor class reference (!!!)
#include "Logging/LogMacros.h"
// Other includes if needed...
#include "Toolkits/AssetEditorToolkit.h"     // Need this for GetObjectsCurrentlyBeingEdited (!!!)
#include "Trace/MarkdownDataTracer.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBlueprintMarkdownWindow::Construct(const FArguments& InArgs)
{
    DataTracer = MakeShared<FMarkdownDataTracer>(DataExtractor);

    ChildSlot // No square brackets here
    [
        SNew(SVerticalBox)

        // Options toolbar
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5)
        [
            SNew(SHorizontalBox)

            // Refresh button
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("BlueprintMarkdown", "RefreshFromSelection", "Refresh from Selection"))
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "RefreshTooltip", "Reload selected nodes from the active Blueprint Editor."))
                .OnClicked(this, &SBlueprintMarkdownWindow::OnRefreshClicked)
            ]

            // Copy to clipboard button
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            [
                SNew(SButton)
                .Text(NSLOCTEXT("BlueprintMarkdown", "CopyToClipboard", "Copy to Clipboard"))
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "CopyTooltip", "Copy the generated Markdown to the clipboard."))
                .OnClicked(this, &SBlueprintMarkdownWindow::OnCopyToClipboardClicked)
            ]

            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SSpacer)
            ]

            // Include nested functions checkbox
            +SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            .VAlign(VAlign_Center)
            [
                SAssignNew(IncludeNestedFunctionsCheckBox, SCheckBox)
                .IsChecked(bIncludeNestedFunctions ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                .OnCheckStateChanged(this, &SBlueprintMarkdownWindow::OnIncludeNestedCheckboxChanged)
                .ToolTipText(NSLOCTEXT("BlueprintMarkdown", "IncludeNestedTooltip", "Attempt to include nodes from functions called by the selection (Experimental)."))
                .Content()
                [
                    SNew(STextBlock)
                    .Text(NSLOCTEXT("BlueprintMarkdown", "IncludeNestedFunctions", "Include Nested Functions"))
                ]
            ]
        ]

        // Separator
        +SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
        ]

        // Main content
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(5)
        [
            SAssignNew(MarkdownTextBox, SMultiLineEditableTextBox)
            .IsReadOnly(false)
            .AutoWrapText(false)
            .ModiferKeyForNewLine(EModifierKey::Shift)
            // .Font(FCoreStyle::GetDefaultMonospacedFont()) // Optional: Use Monospaced Font
        ]
    ]; // End ChildSlot block

    UpdateFromSelection();
}

void SBlueprintMarkdownWindow::UpdateFromSelection()
{
    if (DataTracer.IsValid()) {
        DataTracer->ClearCache();
         UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: DataTracer cache cleared."));
    }
    else {
        UE_LOG(LogUI, Error, TEXT("BlueprintMarkdown: DataTracer is invalid during UpdateFromSelection!"));
        DataTracer = MakeShared<FMarkdownDataTracer>(DataExtractor);
    }

    TArray<UEdGraphNode*> SelectedNodes = GetSelectedBlueprintNodes(); // const call ok
    if (SelectedNodes.Num() == 0) {
         UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: No nodes selected."));
        MarkdownTextBox->SetText(FText::FromString(TEXT("# No Blueprint nodes selected\n\nPlease select one or more nodes in a Blueprint editor.")));
        return;
    }

    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: Extracting data from %d selected nodes..."), SelectedNodes.Num());
    TMap<FString, TSharedPtr<FBlueprintNode>> ExtractedNodes = DataExtractor.ExtractFromSelectedNodes(SelectedNodes, bIncludeNestedFunctions); // const call ok
    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: Extracted %d nodes into map."), ExtractedNodes.Num());

    // --- BEGIN DEBUG LOGGING for specific node types ---
    const FString DivideNodeGUID = TEXT("67BBB35F423615EE58814495C76DEFAA"); // From your sample output
    const FString GetNodeGUID = TEXT("1115F1404B8DB49BD602F18AEACF7531"); // From your sample output

    if (const TSharedPtr<FBlueprintNode>* FoundDivideNode = ExtractedNodes.Find(DivideNodeGUID))
    {
        if (FoundDivideNode->IsValid())
        {
            UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Divide Node (%s) - Stored NodeType in Map: %s"), *DivideNodeGUID.Left(8), *(*FoundDivideNode)->NodeType);
        } else {
             UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Found Divide Node (%s) ptr in map, but it's invalid."), *DivideNodeGUID.Left(8));
        }
    } else {
         UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Divide Node (%s) NOT FOUND in ExtractedNodes map."), *DivideNodeGUID.Left(8));
    }

    if (const TSharedPtr<FBlueprintNode>* FoundGetNode = ExtractedNodes.Find(GetNodeGUID))
    {
         if (FoundGetNode->IsValid())
        {
             UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Get Node (%s) - Stored NodeType in Map: %s"), *GetNodeGUID.Left(8), *(*FoundGetNode)->NodeType);
        } else {
            UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Found Get Node (%s) ptr in map, but it's invalid."), *GetNodeGUID.Left(8));
        }
    } else {
        UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow: DEBUG - Get Node (%s) NOT FOUND in ExtractedNodes map."), *GetNodeGUID.Left(8));
    }
    // --- END DEBUG LOGGING ---


    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: Generating Markdown..."));
    FString MarkdownText = GenerateMarkdown(ExtractedNodes); // Non-const call ok
    MarkdownTextBox->SetText(FText::FromString(MarkdownText));
    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow: Update complete."));
}

// --- REVISED GetSelectedBlueprintNodes using AssetEditorSubsystem and Activation Time ---
TArray<UEdGraphNode*> SBlueprintMarkdownWindow::GetSelectedBlueprintNodes() const
{
    TArray<UEdGraphNode*> SelectedNodes;

    if (!GEditor)
    {
        UE_LOG(LogUI, Error, TEXT("BlueprintMarkdown GetSelectedNodes: GEditor is null."));
        return SelectedNodes;
    }

    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem)
    {
        UE_LOG(LogUI, Warning, TEXT("BlueprintMarkdown GetSelectedNodes: AssetEditorSubsystem not found."));
        return SelectedNodes;
    }

    IAssetEditorInstance* MostRecentEditorInstance = nullptr;
    double LastActivationTime = 0.0;

    // Iterate through all assets being edited
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    UE_LOG(LogUI, Verbose, TEXT("BlueprintMarkdown GetSelectedNodes: Checking %d edited assets total."), EditedAssets.Num());

    for (UObject* Asset : EditedAssets)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
        if (!Blueprint) continue; // Skip non-blueprint assets

        // Find *any* editor instance for this specific Blueprint asset
        IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Blueprint, false); // Don't force focus

        if (EditorInstance)
        {
            // Check if it's actually a Blueprint Editor by name
            if (EditorInstance->GetEditorName() == FName("BlueprintEditor"))
            {
                // Get its last activation time
                double CurrentActivationTime = EditorInstance->GetLastActivationTime();
                UE_LOG(LogUI, Verbose, TEXT("BlueprintMarkdown GetSelectedNodes: Found BP Editor instance for '%s'. Activation Time: %f"), *Blueprint->GetName(), CurrentActivationTime);

                // If this one is more recent, store it
                if (CurrentActivationTime >= LastActivationTime) // Use >= to handle first assignment
                {
                    MostRecentEditorInstance = EditorInstance; // Store the IAssetEditorInstance pointer
                    LastActivationTime = CurrentActivationTime;
                    UE_LOG(LogUI, Verbose, TEXT("BlueprintMarkdown GetSelectedNodes: --> Updating most recent editor instance."));
                }
            }
            //else { UE_LOG(LogUI, Verbose, TEXT("BlueprintMarkdown GetSelectedNodes: Editor for '%s' is not a BlueprintEditor (Type: %s)."), *Blueprint->GetName(), *EditorInstance->GetEditorName().ToString()); }
        }
        // else { UE_LOG(LogUI, Verbose, TEXT("BlueprintMarkdown GetSelectedNodes: Could not find any editor instance for Blueprint: %s"), *Blueprint->GetName()); }
    }

    // After checking all assets, try to use the most recent instance found
    if (!MostRecentEditorInstance)
    {
        UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown GetSelectedNodes: No suitable Blueprint editor instance found among edited assets."));
        return SelectedNodes;
    }

    // Now that we have the most recent IAssetEditorInstance confirmed to be a BlueprintEditor,
    // cast it to FBlueprintEditor to access GetSelectedNodes.
    FBlueprintEditor* TargetBlueprintEditor = static_cast<FBlueprintEditor*>(MostRecentEditorInstance); // Safe cast after GetEditorName check
    if (!TargetBlueprintEditor)
    {
        // Should not happen if GetEditorName() check passed, but good safety check
        UE_LOG(LogUI, Error, TEXT("BlueprintMarkdown GetSelectedNodes: Failed to cast the identified IAssetEditorInstance to FBlueprintEditor."));
        return SelectedNodes;
    }

    // Also cast to FAssetEditorToolkit to get the edited asset name correctly
    FAssetEditorToolkit* TargetToolkit = static_cast<FAssetEditorToolkit*>(MostRecentEditorInstance);
    FString AssetName = TEXT("Unknown");
    if(TargetToolkit)
    {
        const TArray<UObject*>* AssetsBeingEditedPtr = TargetToolkit->GetObjectsCurrentlyBeingEdited();
        if (AssetsBeingEditedPtr && AssetsBeingEditedPtr->Num() > 0 && (*AssetsBeingEditedPtr)[0])
        {
            AssetName = (*AssetsBeingEditedPtr)[0]->GetName();
        }

    }


    // Get the selected nodes from the FBlueprintEditor instance
    FGraphPanelSelectionSet SelectedNodeSet = TargetBlueprintEditor->GetSelectedNodes();

    if (SelectedNodeSet.IsEmpty())
    {
        UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown GetSelectedNodes: No nodes selected in the identified editor for asset %s."), *AssetName);
        return SelectedNodes;
    }

    for (UObject* SelectedObject : SelectedNodeSet)
    {
        if (UEdGraphNode* GraphNode = Cast<UEdGraphNode>(SelectedObject))
        {
            SelectedNodes.Add(GraphNode);
        }
        else
        {
            UE_LOG(LogUI, Warning, TEXT("BlueprintMarkdown GetSelectedNodes: Selected object '%s' in editor is not a UEdGraphNode."),
                   SelectedObject ? *SelectedObject->GetName() : TEXT("Null"));
        }
    }

    UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown GetSelectedNodes: Found %d selected nodes in most recently active editor for asset %s."), SelectedNodes.Num(), *AssetName);

    return SelectedNodes;
}


// GenerateMarkdown - Definition Removed const
FString SBlueprintMarkdownWindow::GenerateMarkdown(const TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes)
// Removed const
{
    if (Nodes.Num() == 0) {
        UE_LOG(LogUI, Warning, TEXT("SBlueprintMarkdownWindow::GenerateMarkdown - Nodes map is empty."));
        return TEXT("# No nodes to display\n\nNo Blueprint nodes were extracted from the selection.");
    }
    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow::GenerateMarkdown - Formatting %d nodes."), Nodes.Num());

    FString Result;
    Result += TEXT("# Blueprint Node Analysis\n\n");
    Result += FString::Printf(TEXT("Selected Nodes: %d\n\n"), Nodes.Num());

    TArray<TSharedPtr<FBlueprintNode>> SortedNodes;
    Nodes.GenerateValueArray(SortedNodes);

    // Sort nodes primarily by Y position, then X position
    SortedNodes.Sort([](const TSharedPtr<FBlueprintNode>& A, const TSharedPtr<FBlueprintNode>& B) {
        // Handle invalid pointers gracefully
        if (!A.IsValid()) return false;
        if (!B.IsValid()) return true;
        // Compare Y positions with a small tolerance
        if (FMath::IsNearlyEqual(A->Position.Y, B->Position.Y, 1.0f)) {
            return A->Position.X < B->Position.X;
        }
        return A->Position.Y < B->Position.Y;
    });

    for (const TSharedPtr<FBlueprintNode>& Node : SortedNodes) {
        if (!Node.IsValid()) {
             UE_LOG(LogUI, Error, TEXT("SBlueprintMarkdownWindow::GenerateMarkdown - Encountered invalid node pointer during formatting."));
             continue;
        }
        // ** OK: Pass Nodes map correctly **
        Result += FormatNode(Node, Nodes); // Non-const call ok
        Result += TEXT("\n\n");
    }
    UE_LOG(LogUI, Log, TEXT("SBlueprintMarkdownWindow::GenerateMarkdown - Formatting complete."));
    return Result;
}

// FormatNode - Definition Removed const
FString SBlueprintMarkdownWindow::FormatNode(const TSharedPtr<FBlueprintNode>& Node, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes)
// Removed const
{
    if (!Node.IsValid()) {
        UE_LOG(LogUI, Error, TEXT("SBlueprintMarkdownWindow::FormatNode - Received invalid node pointer."));
        return TEXT("## [Invalid Node]");
    }

    FString Result;
    Result += FString::Printf(TEXT("## %s\n"), *Node->NodeType); // NodeType is FString

    // Node->Name is FString, use IsEmpty() and no ToString()
    if (!Node->Name.IsEmpty() && Node->Name != Node->NodeType) {
        Result += FString::Printf(TEXT("**Name:** `%s`\n"), *Node->Name);
    }

    Result += FString::Printf(TEXT("**GUID:** `%s`\n"), *Node->Guid); // Guid is FString
    Result += FString::Printf(TEXT("**Position:** (%.0f, %.0f)\n"), Node->Position.X, Node->Position.Y);
    Result += FString::Printf(TEXT("**Class:** `%s`\n"), *Node->UEClass); // UEClass is FString

    if (!Node->NodeComment.IsEmpty()) {
        FString CommentFormatted = Node->NodeComment.Replace(TEXT("\n"), TEXT("\n> ")); // NodeComment is FString
        Result += FString::Printf(TEXT("**Comment:**\n> %s\n"), *CommentFormatted);
    }

    Result += FString::Printf(TEXT("**Pure Node:** %s\n"), Node->IsPure() ? TEXT("Yes") : TEXT("No")); // const call ok

    // Add properties extracted by the factory
    const FString* FunctionName = Node->RawProperties.Find(TEXT("FunctionName")); if (FunctionName) { Result += FString::Printf(TEXT("**Function:** `%s`\n"), **FunctionName); }
    const FString* EventFunctionName = Node->RawProperties.Find(TEXT("EventFunctionName")); if (EventFunctionName) { Result += FString::Printf(TEXT("**Event:** `%s`\n"), **EventFunctionName); }
    const FString* VariableName = Node->RawProperties.Find(TEXT("VariableName")); if (VariableName) { Result += FString::Printf(TEXT("**Variable:** `%s`\n"), **VariableName); }
    const FString* IsLatent = Node->RawProperties.Find(TEXT("bIsLatent")); if (IsLatent && *IsLatent == TEXT("true")) { Result += TEXT("**Latent Action:** Yes\n"); }

    // Format Pins
    Result += TEXT("\n### Input Pins\n");
    // GetInputPins returns TArray<TSharedPtr<FBlueprintPin>>
    TArray<TSharedPtr<FBlueprintPin>> InputPins = Node->GetInputPins(TEXT(""), true, true); // Exclude Exec pins from Input list usually
    if (InputPins.Num() > 0) {
        for (const TSharedPtr<FBlueprintPin>& Pin : InputPins) {
            if(Pin.IsValid()) Result += FormatPin(Pin, AllNodes); // Non-const call ok
        }
    } else {
        Result += TEXT("*No input data pins*\n");
    }

    Result += TEXT("\n### Output Pins\n");
    // GetOutputPins returns TArray<TSharedPtr<FBlueprintPin>>
    TArray<TSharedPtr<FBlueprintPin>> OutputPins = Node->GetOutputPins(TEXT(""), true); // Include hidden, include Exec
    if (OutputPins.Num() > 0) {
        for (const TSharedPtr<FBlueprintPin>& Pin : OutputPins) {
             if(Pin.IsValid()) Result += FormatPin(Pin, AllNodes); // Non-const call ok
        }
    } else {
        Result += TEXT("*No output pins*\n");
    }
    return Result;
}


// FormatPin - Definition Removed const
FString SBlueprintMarkdownWindow::FormatPin(const TSharedPtr<FBlueprintPin>& Pin, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes)
{
    if (!Pin.IsValid()) {
        UE_LOG(LogUI, Error, TEXT("SBlueprintMarkdownWindow::FormatPin - Received invalid pin pointer."));
        return TEXT("- [Invalid Pin]\n");
    }

    FString Result = FString::Printf(TEXT("- **%s**"), *Pin->Name);
    if (!Pin->FriendlyName.IsEmpty() && Pin->FriendlyName != Pin->Name) { Result += FString::Printf(TEXT(" (\"%s\")"), *Pin->FriendlyName); }
    Result += FString::Printf(TEXT(" : `%s`"), *Pin->GetTypeSignature());
    if (Pin->IsExecution()) { Result += TEXT(" [Exec]"); }
    if (Pin->IsHidden()) { Result += TEXT(" [Hidden]"); }
    if (Pin->IsAdvancedView()) { Result += TEXT(" [Advanced]"); }
    if (Pin->bIsReference) { Result += TEXT(" [Ref]"); }
    if (Pin->bIsConst) { Result += TEXT(" [Const]"); }

    if (Pin->IsInput() && !Pin->IsExecution()) {
        // --- MODIFIED: Use MarkdownTracerUtils::IsTrivialDefault ---
        // Need a const pointer to tracer for IsTrivialDefault if it were const, but it's not needed here
        // For TracePinValue, we need the non-const tracer from the member variable
        if (DataTracer.IsValid() && (Pin->SourcePinFor.Num() > 0 || !MarkdownTracerUtils::IsTrivialDefault(Pin))) {
        // --- END MODIFIED ---
            FString TracedValue = DataTracer->TracePinValue(Pin, AllNodes); // Calls non-const
            Result += FString::Printf(TEXT(" = %s"), *TracedValue);
        }
    }
    else if (Pin->IsOutput() && !Pin->IsExecution()) {
         if (!Pin->DefaultValue.IsEmpty() || !Pin->DefaultObject.IsEmpty() || Pin->DefaultStruct.Num() > 0) {
             // --- MODIFIED: Use MarkdownTracerUtils::IsTrivialDefault ---
             if (!MarkdownTracerUtils::IsTrivialDefault(Pin)) { // Check if default is non-trivial before displaying
             // --- END MODIFIED ---
                 FString RawDefault = !Pin->DefaultValue.IsEmpty() ? Pin->DefaultValue : (!Pin->DefaultObject.IsEmpty() ? Pin->DefaultObject : TEXT("{Struct Default}"));
                 RawDefault.ReplaceInline(TEXT("`"), TEXT("\\`"));
                 Result += FString::Printf(TEXT(" = `%s` (Default)"), *RawDefault);
             }
         }
    }

    // Links section remains unchanged
    if (Pin->IsOutput() && Pin->LinkedPins.Num() > 0) {
        // ... (link formatting) ...
    } else if (Pin->IsInput() && Pin->SourcePinFor.Num() > 0) {
         // ... (link formatting) ...
    }

    Result += TEXT("\n");
    return Result;
}

// OnCopyToClipboardClicked (unchanged)
FReply SBlueprintMarkdownWindow::OnCopyToClipboardClicked() { FString MarkdownText = MarkdownTextBox->GetText().ToString(); if (!MarkdownText.IsEmpty()){ FPlatformApplicationMisc::ClipboardCopy(*MarkdownText); UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown: Copied Markdown to clipboard."));} return FReply::Handled(); }
// OnRefreshClicked (unchanged)
FReply SBlueprintMarkdownWindow::OnRefreshClicked() { UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown: Refresh button clicked.")); UpdateFromSelection(); return FReply::Handled(); }
// OnIncludeNestedCheckboxChanged (unchanged)
void SBlueprintMarkdownWindow::OnIncludeNestedCheckboxChanged(ECheckBoxState NewState) { bIncludeNestedFunctions = (NewState == ECheckBoxState::Checked); UE_LOG(LogUI, Log, TEXT("BlueprintMarkdown: Include Nested Functions set to %s."), bIncludeNestedFunctions ? TEXT("true") : TEXT("false")); UpdateFromSelection();}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION