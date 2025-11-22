/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Public/BP2AI.h


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Widgets/SWindow.h"

// Forward declarations
class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;

class SMarkdownOutputWindow;

class FBP2AIModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /** Command handler for generating EXEC FLOW markdown from selection */
    void GenerateExecFlowAction();

    /** Access the singleton instance */
    static FBP2AIModule& Get();

private:
    void RegisterMenus();
   
	TSharedRef<SWidget> CreateBlueprintExecFlowWindow();

 
    /** Handler for when the NEW Exec Flow window is closed */
    void OnExecFlowWindowClosed(const TSharedRef<SWindow>& Window);

    /** Exec Flow window references */
    TSharedPtr<SWindow> ExecFlowPluginWindow;
    TSharedPtr<class SMarkdownOutputWindow, ESPMode::ThreadSafe> ExecFlowMarkdownWindow;

    /** Commands handler */
    TSharedPtr<FUICommandList> PluginCommands;
};