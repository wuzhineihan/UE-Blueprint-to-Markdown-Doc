/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/BP2AI.cpp


#include "BP2AI.h"
#include "Logging/BP2AILog.h"
#include "BP2AIStyle.h"
#include "BP2AICommands.h"
#include "SBlueprintMarkdownWindow.h" 

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SMarkdownOutputWindow.h"
// Removed duplicate BP2AILog.h include

#include "ToolMenus.h" 
#include "BlueprintEditorModule.h" 
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h" 
#include "LevelEditor.h" 
#include "Logging/LogMacros.h" 
#include "Styling/AppStyle.h" 



static const FName BP2AIToolbarSectionName("BP2AI"); 

#define LOCTEXT_NAMESPACE "FBP2AIModule"

void FBP2AIModule::StartupModule()
{
    
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: StartupModule() Begin."));
    FBP2AIStyle::Initialize();
    FBP2AIStyle::ReloadTextures();
    FBP2AICommands::Register();
    PluginCommands = MakeShareable(new FUICommandList);

    /* --- COMMENTED OUT LEGACY ACTION MAPPINGS ---
    PluginCommands->MapAction(
        FBP2AICommands::Get().GenerateMarkdownCommand, 
        FExecuteAction::CreateRaw(this, &FBP2AIModule::GenerateMarkdownAction), 
        FCanExecuteAction());

    PluginCommands->MapAction(
        FBP2AICommands::Get().GenerateTextMarkdownCommand,
        FExecuteAction::CreateRaw(this, &FBP2AIModule::GenerateTextBasedMarkdownAction),
        FCanExecuteAction());

    PluginCommands->MapAction(
        FBP2AICommands::Get().ShowExportDebugCommand,
        FExecuteAction::CreateRaw(this, &FBP2AIModule::ShowExportDebugAction),
        FCanExecuteAction());
    */
    
    // --- KEPT: The one remaining action mapping ---
    PluginCommands->MapAction(
        FBP2AICommands::Get().GenerateExecFlowCommand,
        FExecuteAction::CreateRaw(this, &FBP2AIModule::GenerateExecFlowAction),
        FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBP2AIModule::RegisterMenus));

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: StartupModule() End."));
}


void FBP2AIModule::ShutdownModule()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: ShutdownModule() Begin."));

    if (UObjectInitialized()) 
    {
        UToolMenus::UnRegisterStartupCallback(this); 
        UToolMenus::UnregisterOwner(this);
    }

    FBP2AIStyle::Shutdown();
    FBP2AICommands::Unregister();
    PluginCommands.Reset(); 

    /* --- COMMENTED OUT LEGACY WINDOW DESTRUCTION ---
    if (PluginWindow.IsValid())
    {
         UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Requesting plugin window destroy during shutdown."));
         PluginWindow->RequestDestroyWindow(); 
    }
    
    if (TextBasedPluginWindow.IsValid())
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Requesting text-based plugin window destroy during shutdown."));
        TextBasedPluginWindow->RequestDestroyWindow();
    }
    
    if (DebugWindow.IsValid())
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Requesting debug window destroy during shutdown."));
        DebugWindow->RequestDestroyWindow();
    }
    */

    // --- KEPT: The one remaining window destruction ---
    if (ExecFlowPluginWindow.IsValid()) ExecFlowPluginWindow->RequestDestroyWindow();
    
    /* --- COMMENTED OUT LEGACY POINTER RESETS ---
    PluginWindow.Reset();
    MarkdownWindow.Reset();
    TextBasedPluginWindow.Reset();
    TextBasedMarkdownWindow.Reset();
    DebugWindow.Reset();
    ExportDebugWindow.Reset();
    */

    // --- KEPT: The one remaining pointer reset ---
    ExecFlowPluginWindow.Reset(); ExecFlowMarkdownWindow.Reset();

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: ShutdownModule() End."));
}

/* --- COMMENTED OUT LEGACY IMPLEMENTATION ---
void FBP2AIModule::GenerateMarkdownAction()
{
   
    if (PluginWindow.IsValid())
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Found existing PluginWindow TSharedPtr. Attempting BringToFront()."));
        PluginWindow->BringToFront();
        return;
    }

    PluginWindow.Reset();
    MarkdownWindow.Reset();

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Creating new plugin window."));
    SAssignNew(PluginWindow, SWindow)
        .Title(LOCTEXT("WindowTitle", "Blueprint Markdown Generator"))
        .ClientSize(FVector2D(800, 600))
        .SizingRule(ESizingRule::UserSized)
        .SupportsMaximize(true)
        .SupportsMinimize(true);

    if (PluginWindow.IsValid())
        {
        PluginWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FBP2AIModule::OnPluginWindowClosed));
        }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AI: Failed to create SWindow!"));
        return; 
    }

    PluginWindow->SetContent(CreateBP2AIWindow()); 

    TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    if (RootWindow.IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(PluginWindow.ToSharedRef(), RootWindow.ToSharedRef());
    }
    else
    {
        FSlateApplication::Get().AddWindow(PluginWindow.ToSharedRef());
    }
  
}
*/
/*
void FBP2AIModule::GenerateTextBasedMarkdownAction()
{
    if (TextBasedPluginWindow.IsValid())
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Found existing TextBasedPluginWindow TSharedPtr. Attempting BringToFront()."));
        TextBasedPluginWindow->BringToFront();
        return;
    }

    TextBasedPluginWindow.Reset();
    TextBasedMarkdownWindow.Reset();

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Creating new text-based plugin window."));
    SAssignNew(TextBasedPluginWindow, SWindow)
        .Title(LOCTEXT("TextWindowTitle", "Text-Based Blueprint Markdown Generator"))
        .ClientSize(FVector2D(800, 600))
        .SizingRule(ESizingRule::UserSized)
        .SupportsMaximize(true)
        .SupportsMinimize(true);

    if (TextBasedPluginWindow.IsValid())
    {
        TextBasedPluginWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FBP2AIModule::OnTextBasedPluginWindowClosed));
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AI: Failed to create text-based SWindow!"));
        return;
    }

    TextBasedPluginWindow->SetContent(CreateTextBasedMarkdownWindow());

    TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    if (RootWindow.IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(TextBasedPluginWindow.ToSharedRef(), RootWindow.ToSharedRef());
    }
    else
    {
        FSlateApplication::Get().AddWindow(TextBasedPluginWindow.ToSharedRef());
    }
}
*/
void FBP2AIModule::GenerateExecFlowAction()
{
    if (ExecFlowPluginWindow.IsValid())
    {
        ExecFlowPluginWindow->BringToFront();
        return;
    }
    ExecFlowPluginWindow.Reset();
    ExecFlowMarkdownWindow.Reset();

    SAssignNew(ExecFlowPluginWindow, SWindow)
        .Title(LOCTEXT("ExecFlowWindowTitle", "Blueprint Execution Flow")) 
        .ClientSize(FVector2D(900, 700)) 
        .SizingRule(ESizingRule::UserSized)
        .SupportsMaximize(true)
        .SupportsMinimize(true);

    if (ExecFlowPluginWindow.IsValid())
    {
        ExecFlowPluginWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FBP2AIModule::OnExecFlowWindowClosed)); 
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AI: Failed to create Exec Flow SWindow!"));
        return;
    }

    ExecFlowPluginWindow->SetContent(CreateBlueprintExecFlowWindow()); 

    TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    if (RootWindow.IsValid()) FSlateApplication::Get().AddWindowAsNativeChild(ExecFlowPluginWindow.ToSharedRef(), RootWindow.ToSharedRef());
    else FSlateApplication::Get().AddWindow(ExecFlowPluginWindow.ToSharedRef());
}
/* --- COMMENTED OUT LEGACY IMPLEMENTATION ---
void FBP2AIModule::ShowExportDebugAction()
{
    if (DebugWindow.IsValid())
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Found existing DebugWindow TSharedPtr. Attempting BringToFront()."));
        DebugWindow->BringToFront();
        return;
    }

    DebugWindow.Reset();
    ExportDebugWindow.Reset();

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Creating new debug window."));
    SAssignNew(DebugWindow, SWindow)
        .Title(LOCTEXT("DebugWindowTitle", "Blueprint Export Debug"))
        .ClientSize(FVector2D(900, 700))
        .SizingRule(ESizingRule::UserSized)
        .SupportsMaximize(true)
        .SupportsMinimize(true);

    if (DebugWindow.IsValid())
    {
        DebugWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FBP2AIModule::OnExportDebugWindowClosed));
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AI: Failed to create debug SWindow!"));
        return;
    }

    DebugWindow->SetContent(CreateExportDebugWindow());

    TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    if (RootWindow.IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(DebugWindow.ToSharedRef(), RootWindow.ToSharedRef());
    }
    else
    {
        FSlateApplication::Get().AddWindow(DebugWindow.ToSharedRef());
    }
}

TSharedRef<SWidget> FBP2AIModule::CreateBP2AIWindow()
{
    MarkdownWindow = SNew(SBP2AIWindow);
    return MarkdownWindow.ToSharedRef();
}
*/
TSharedRef<SWidget> FBP2AIModule::CreateBlueprintExecFlowWindow()
{
    SAssignNew(ExecFlowMarkdownWindow, SMarkdownOutputWindow); 
    return ExecFlowMarkdownWindow.ToSharedRef();
}
/* --- COMMENTED OUT LEGACY IMPLEMENTATION ---
TSharedRef<SWidget> FBP2AIModule::CreateTextBasedMarkdownWindow()
{
    TextBasedMarkdownWindow = SNew(STextExportMarkdownWindow);
    return TextBasedMarkdownWindow.ToSharedRef();
}

TSharedRef<SWidget> FBP2AIModule::CreateExportDebugWindow()
{
    ExportDebugWindow = SNew(SBlueprintExportDebugWindow);
    return ExportDebugWindow.ToSharedRef();
}
*/
void FBP2AIModule::RegisterMenus()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: RegisterMenus() Called."));

    FToolMenuOwnerScoped OwnerScoped(this);

    if (!PluginCommands.IsValid())
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AI: PluginCommands is invalid during menu registration!"));
        return;
    }

    const FSlateIcon ButtonIcon = FSlateIcon(FBP2AIStyle::GetStyleSetName(), "BP2AI.GenerateMarkdown");

    // Register button for regular Blueprint Editor
    UToolMenu* BlueprintToolbar = UToolMenus::Get()->ExtendMenu("AssetEditor.BlueprintEditor.ToolBar");
    if (BlueprintToolbar)
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Successfully found BlueprintEditor.ToolBar menu."));
        FToolMenuSection& BPSection = BlueprintToolbar->FindOrAddSection("Settings");
        FToolMenuEntry& BPEntry = BPSection.AddEntry(FToolMenuEntry::InitToolBarButton(
            FBP2AICommands::Get().GenerateExecFlowCommand,
            LOCTEXT("BP2AILabel", "BP2AI"),
            LOCTEXT("BP2AITooltip", "Analyze Blueprint and generate AI-readable documentation"),
            ButtonIcon
        ));
        BPEntry.SetCommandList(PluginCommands);
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Button added to BlueprintEditor toolbar."));
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Failed to extend BlueprintEditor.ToolBar."));
    }

    // Register button for Widget Blueprint Editor (for EditorUtilityWidgets)
    UToolMenu* WidgetToolbar = UToolMenus::Get()->ExtendMenu("AssetEditor.WidgetBlueprintEditor.ToolBar");
    if (WidgetToolbar)
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Successfully found WidgetBlueprintEditor.ToolBar menu."));
        FToolMenuSection& WidgetSection = WidgetToolbar->FindOrAddSection("Settings");
        FToolMenuEntry& WidgetEntry = WidgetSection.AddEntry(FToolMenuEntry::InitToolBarButton(
            FBP2AICommands::Get().GenerateExecFlowCommand,
            LOCTEXT("BP2AILabel", "BP2AI"),
            LOCTEXT("BP2AITooltip", "Analyze Blueprint and generate AI-readable documentation"),
            ButtonIcon
        ));
        WidgetEntry.SetCommandList(PluginCommands);
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Button added to WidgetBlueprintEditor toolbar."));
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Failed to extend WidgetBlueprintEditor.ToolBar."));
    }

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Toolbar button registration complete."));
}
/* --- COMMENTED OUT LEGACY IMPLEMENTATION ---
void FBP2AIModule::OnPluginWindowClosed(const TSharedRef<SWindow>& Window)
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: OnPluginWindowClosed Called. Resetting pointers."));

    if (Window == PluginWindow)
    {
        PluginWindow.Reset();
        MarkdownWindow.Reset(); 
    }
    else
    {
         UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: OnPluginWindowClosed called for an unexpected window."));
    }
}

*/
void FBP2AIModule::OnExecFlowWindowClosed(const TSharedRef<SWindow>& Window)
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: OnExecFlowWindowClosed Called. Resetting pointers."));
    if (Window == ExecFlowPluginWindow)
    {
        ExecFlowPluginWindow.Reset();
        ExecFlowMarkdownWindow.Reset();
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: OnExecFlowWindowClosed called for an unexpected window."));
    }
}
/* --- COMMENTED OUT LEGACY IMPLEMENTATION ---
void FBP2AIModule::OnTextBasedPluginWindowClosed(const TSharedRef<SWindow>& Window)
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: OnTextBasedPluginWindowClosed Called. Resetting pointers."));

    if (Window == TextBasedPluginWindow)
    {
        TextBasedPluginWindow.Reset();
        TextBasedMarkdownWindow.Reset();
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: OnTextBasedPluginWindowClosed called for an unexpected window."));
    }
}

void FBP2AIModule::OnExportDebugWindowClosed(const TSharedRef<SWindow>& Window)
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: OnExportDebugWindowClosed Called. Resetting pointers."));

    if (Window == DebugWindow)
    {
        DebugWindow.Reset();
        ExportDebugWindow.Reset();
    }
    else
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: OnExportDebugWindowClosed called for an unexpected window."));
    }
}
*/
FBP2AIModule& FBP2AIModule::Get()
{
    return FModuleManager::LoadModuleChecked<FBP2AIModule>("BP2AI");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBP2AIModule, BP2AI)