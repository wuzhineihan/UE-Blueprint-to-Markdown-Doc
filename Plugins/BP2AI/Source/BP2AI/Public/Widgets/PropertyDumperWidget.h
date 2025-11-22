/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/Widgets/PropertyDumperWidget.h

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/Button.h"
#include "PropertyDumperWidget.generated.h"


// ✅ ADD FORWARD DECLARATION for the test namespace/function
#if !UE_BUILD_SHIPPING
namespace BP2AITests
{
	BP2AI_API void ExecuteCurrentPhaseTest();
}
#endif



/**
 * Editor Utility Widget to dump reflected properties of specified Blueprint node classes.
 */
UCLASS()
class BP2AI_API UPropertyDumperWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()


	
public:
	/** Text box for inputting class paths, one per line. */
	UPROPERTY(meta = (BindWidget))
	UMultiLineEditableTextBox* InputClassPathsTextBox;

	/** Button to trigger the property dumping process. */
	UPROPERTY(meta = (BindWidget))
	UButton* DumpPropertiesButton;

	/** Text box to display the dumped property results. */
	UPROPERTY(meta = (BindWidget))
	UMultiLineEditableTextBox* OutputResultsTextBox;

	
	UPROPERTY(meta = (BindWidget))
	UButton* AnalyzeBPFunctionsButton;

	
	UPROPERTY(meta = (BindWidget)) // <<< MAKE SURE TO BIND THIS IN YOUR UMG EDITOR AFTER ADDING
	UButton* DumpSelectedNodePinsButton;
	FReply OnAnalyzeBlueprintsButtonClicked();

	/** ✅ NEW: Button to trigger the current phase's test script. */
	UPROPERTY(meta = (BindWidget))
	UButton* RunCurrentPhaseTestButton; // Ensure this name matches your UMG Widget's button name

	
protected:
	virtual void NativeConstruct() override;

	/** Called when the Dump Properties button is clicked. */
	UFUNCTION()
	void OnDumpPropertiesClicked();

	UFUNCTION() // <<< NEW
	void OnDumpSelectedNodePinsClicked(); // <<< NEW
	
	UFUNCTION()
	void OnAnalyzeBPFunctionsButtonClicked();

	UFUNCTION()
	void OnRunCurrentPhaseTestClicked();

	
private:
	/** List of property types we are interested in dumping. */
	TSet<FName> RelevantPropertyTypes;

	/** Helper to get the initial list of node types needing investigation. */
	FString GetInitialClassPaths() const;
};