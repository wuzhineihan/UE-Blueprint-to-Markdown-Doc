/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/SBlueprintMarkdownWindow.h


#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h" // Include the tracer
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"

class UEdGraphNode;
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer; // Forward declare

/**
 * Main window for the Blueprint Markdown generator
 */
class BP2AI_API SBlueprintMarkdownWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBlueprintMarkdownWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void UpdateFromSelection();

private:
	// GetSelectedBlueprintNodes can remain const as it doesn't modify tracer state
	TArray<UEdGraphNode*> GetSelectedBlueprintNodes() const;

	// --- MODIFIED: Removed const ---
	FString GenerateMarkdown(const TMap<FString, TSharedPtr<FBlueprintNode>>& Nodes);
	FString FormatNode(const TSharedPtr<FBlueprintNode>& Node, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes);
	FString FormatPin(const TSharedPtr<FBlueprintPin>& Pin, const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes);
	// --- END MODIFIED ---

	FReply OnCopyToClipboardClicked();
	FReply OnRefreshClicked();
	void OnIncludeNestedCheckboxChanged(ECheckBoxState NewState);

	TSharedPtr<SMultiLineEditableTextBox, ESPMode::ThreadSafe> MarkdownTextBox;
	TSharedPtr<SCheckBox, ESPMode::ThreadSafe> IncludeNestedFunctionsCheckBox;

	FBlueprintDataExtractor DataExtractor;
	TSharedPtr<FMarkdownDataTracer> DataTracer;

	bool bIncludeNestedFunctions = false;
};