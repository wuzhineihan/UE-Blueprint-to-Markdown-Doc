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
#include "Engine/Blueprint.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/GarbageCollection.h"
#include "Exporters/BP2AIBatchExporter.h"
#include "Misc/Paths.h"
#include "Settings/BP2AIExportConfig.h"



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

    RegisterContentBrowserAssetMenu();
    RegisterContentBrowserFolderMenu();

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Toolbar button registration complete."));
}

void FBP2AIModule::RegisterContentBrowserAssetMenu()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: RegisterContentBrowserAssetMenu called."));

    if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu"))
    {
        Menu->AddDynamicSection("BP2AI.DynamicSection", FNewToolMenuDelegate::CreateLambda([this](UToolMenu* InMenu)
        {
            if (!InMenu) return;

            UContentBrowserAssetContextMenuContext* Context = InMenu->FindContext<UContentBrowserAssetContextMenuContext>();
            if (!Context || Context->SelectedAssets.Num() != 1) return;

            const FAssetData& AssetData = Context->SelectedAssets[0];
            if (!CanExportBlueprintAsset(AssetData)) return;

            FToolMenuSection& Section = InMenu->FindOrAddSection("AssetContextAssetActions");
            
            Section.AddMenuEntry(
                "BP2AI_ExportSingleBlueprint",
                LOCTEXT("BP2AI_ExportSingleBlueprint_Label", "Export Blueprint (BP2AI)"),
                LOCTEXT("BP2AI_ExportSingleBlueprint_Tooltip", "Export the selected blueprint or interface to a BP2AI Markdown document."),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([this, AssetData]()
                {
                    HandleSingleAssetExport(AssetData);
                }))
            );
        }));
    }
}

void FBP2AIModule::RegisterContentBrowserFolderMenu()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: RegisterContentBrowserFolderMenu called."));

    if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
    {
        Menu->AddDynamicSection("BP2AI.FolderDynamicSection", FNewToolMenuDelegate::CreateLambda([this](UToolMenu* InMenu)
        {
            if (!InMenu)
            {
                return;
            }

            const UContentBrowserFolderContext* FolderContext = InMenu->FindContext<UContentBrowserFolderContext>();
            if (!FolderContext)
            {
                return;
            }

            TArray<FString> SelectedPathsCopy;
            if (FolderContext->bCanBeModified && FolderContext->SelectedPackagePaths.Num() > 0)
            {
                for (const FString& PathString : FolderContext->SelectedPackagePaths)
                {
                    SelectedPathsCopy.Add(PathString);
                }
            }

            if (SelectedPathsCopy.Num() == 0)
            {
                return;
            }

            FToolMenuSection& Section = InMenu->FindOrAddSection("FolderContextBulkOperations");
            Section.AddMenuEntry(
                "BP2AI_ExportSelectedFolders",
                LOCTEXT("BP2AI_ExportSelectedFolders_Label", "Export Folders (BP2AI)"),
                LOCTEXT("BP2AI_ExportSelectedFolders_Tooltip", "Export all blueprints within the selected folders using BP2AI."),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([this, SelectedPathsCopy]()
                {
                    HandleFolderExport(SelectedPathsCopy);
                }))
            );
        }));
    }
}

bool FBP2AIModule::CanExportBlueprintAsset(const FAssetData& AssetData) const
{
    if (!AssetData.IsValid())
    {
        return false;
    }

    const UClass* AssetClass = AssetData.GetClass();
    if (!AssetClass)
    {
        return false;
    }

    return AssetClass->IsChildOf(UBlueprint::StaticClass());
}

void FBP2AIModule::HandleSingleAssetExport(const FAssetData& AssetData) const
{
    UObject* LoadedObject = AssetData.GetAsset();
    UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject);
    if (!Blueprint)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Selected asset '%s' is not a blueprint or failed to load."), *AssetData.AssetName.ToString());
        return;
    }

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Exporting blueprint '%s' via Content Browser action."), *Blueprint->GetName());

    const FCompleteBlueprintData CompleteData = FBP2AIBatchExporter::ExportCompleteBlueprint(Blueprint, true);
    const FString TargetFilePath = BuildExportFilePath(AssetData.PackagePath.ToString(), CompleteData.BlueprintName);

    if (!FBP2AIBatchExporter::WriteCompleteBlueprintMarkdown(CompleteData, TargetFilePath, true))
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Failed to save blueprint export to '%s'."), *TargetFilePath);
        return;
    }

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Export complete -> %s"), *TargetFilePath);
}

void FBP2AIModule::HandleFolderExport(const TArray<FString>& SelectedPaths) const
{
    if (SelectedPaths.Num() == 0)
    {
        return;
    }

    // 批量导出时静默内部技术日志分类（可通过配置关闭）
    ELogVerbosity::Type OriginalDataTracer = ELogVerbosity::NoLogging;
    ELogVerbosity::Type OriginalPathTracer = ELogVerbosity::NoLogging;
    ELogVerbosity::Type OriginalFormatter = ELogVerbosity::NoLogging;
    ELogVerbosity::Type OriginalExtractor = ELogVerbosity::NoLogging;
    ELogVerbosity::Type OriginalNodeFactory = ELogVerbosity::NoLogging;
    ELogVerbosity::Type OriginalModels = ELogVerbosity::NoLogging;

    if (BP2AIExportConfig::bSilenceInternalCategoriesDuringBatchExport)
    {
        OriginalDataTracer = LogDataTracer.GetVerbosity();
        OriginalPathTracer = LogPathTracer.GetVerbosity();
        OriginalFormatter = LogFormatter.GetVerbosity();
        OriginalExtractor = LogExtractor.GetVerbosity();
        OriginalNodeFactory = LogBlueprintNodeFactory.GetVerbosity();
        OriginalModels = LogModels.GetVerbosity();

        LogDataTracer.SetVerbosity(ELogVerbosity::NoLogging);
        LogPathTracer.SetVerbosity(ELogVerbosity::NoLogging);
        LogFormatter.SetVerbosity(ELogVerbosity::NoLogging);
        LogExtractor.SetVerbosity(ELogVerbosity::NoLogging);
        LogBlueprintNodeFactory.SetVerbosity(ELogVerbosity::NoLogging);
        LogModels.SetVerbosity(ELogVerbosity::NoLogging);
    }

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Folder export started (%d folder%s)."), SelectedPaths.Num(), SelectedPaths.Num() > 1 ? TEXT("s") : TEXT(""));

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());

    for (const FString& Path : SelectedPaths)
    {
        if (!Path.IsEmpty())
        {
            Filter.PackagePaths.Add(FName(*Path));
        }
    }

    TArray<FAssetData> BlueprintAssets;
    AssetRegistry.GetAssets(Filter, BlueprintAssets);

    if (BlueprintAssets.Num() == 0)
    {
        UE_LOG(LogBP2AI, Log, TEXT("BP2AI: No blueprint assets found under selected folders."));

        // 恢复日志级别
        if (BP2AIExportConfig::bSilenceInternalCategoriesDuringBatchExport)
        {
            LogDataTracer.SetVerbosity(OriginalDataTracer);
            LogPathTracer.SetVerbosity(OriginalPathTracer);
            LogFormatter.SetVerbosity(OriginalFormatter);
            LogExtractor.SetVerbosity(OriginalExtractor);
            LogBlueprintNodeFactory.SetVerbosity(OriginalNodeFactory);
            LogModels.SetVerbosity(OriginalModels);
        }

        return;
    }

    const int32 TotalAssets = BlueprintAssets.Num();
    int32 SuccessCount = 0;
    int32 FailureCount = 0;
    int32 ProcessedSinceGc = 0;
    const int32 GcInterval = 10;

    // 创建进度条对话框
    FScopedSlowTask Progress(TotalAssets, LOCTEXT("BP2AIExportProgress", "Exporting Blueprints..."));
    Progress.MakeDialog(true); // true = 允许取消

    for (int32 Index = 0; Index < TotalAssets; ++Index)
    {
        // 检查用户是否取消
        if (Progress.ShouldCancel())
        {
            UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Export cancelled by user. Processed: %d/%d"), SuccessCount + FailureCount, TotalAssets);
            break;
        }

        const FAssetData& AssetData = BlueprintAssets[Index];
        if (!CanExportBlueprintAsset(AssetData))
        {
            Progress.EnterProgressFrame(1.0f);
            continue;
        }

        // 更新进度条显示当前资产路径
        const FString AssetFullPath = AssetData.GetObjectPathString();
        Progress.EnterProgressFrame(1.0f, FText::Format(
            LOCTEXT("ExportingAsset", "[{0}/{1}] {2}"),
            FText::AsNumber(Index + 1),
            FText::AsNumber(TotalAssets),
            FText::FromString(AssetFullPath)
        ));

        UObject* LoadedObject = AssetData.GetAsset();
        UBlueprint* Blueprint = Cast<UBlueprint>(LoadedObject);
        if (!Blueprint)
        {
            ++FailureCount;
            UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: Failed to load blueprint asset '%s'."), *AssetData.AssetName.ToString());
            continue;
        }

        const FCompleteBlueprintData CompleteData = FBP2AIBatchExporter::ExportCompleteBlueprint(Blueprint, true);
        const FString TargetFilePath = BuildExportFilePath(AssetData.PackagePath.ToString(), CompleteData.BlueprintName);

        const bool bSaved = FBP2AIBatchExporter::WriteCompleteBlueprintMarkdown(CompleteData, TargetFilePath, true);
        const int32 ProgressIndex = SuccessCount + FailureCount + 1;

        if (bSaved)
        {
            ++SuccessCount;
            UE_LOG(LogBP2AI, Log, TEXT("BP2AI: [%d/%d] %s"), ProgressIndex, TotalAssets, *AssetFullPath);
        }
        else
        {
            ++FailureCount;
            UE_LOG(LogBP2AI, Warning, TEXT("BP2AI: [%d/%d] %s (Failed to save)"), ProgressIndex, TotalAssets, *AssetFullPath);
        }

        ++ProcessedSinceGc;
        if (ProcessedSinceGc >= GcInterval)
        {
            ProcessedSinceGc = 0;
            CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        }
    }

    UE_LOG(LogBP2AI, Log, TEXT("BP2AI: Folder export summary -> Success: %d | Failed: %d | Total: %d"), SuccessCount, FailureCount, TotalAssets);

    // 恢复原始日志级别
    if (BP2AIExportConfig::bSilenceInternalCategoriesDuringBatchExport)
    {
        LogDataTracer.SetVerbosity(OriginalDataTracer);
        LogPathTracer.SetVerbosity(OriginalPathTracer);
        LogFormatter.SetVerbosity(OriginalFormatter);
        LogExtractor.SetVerbosity(OriginalExtractor);
        LogBlueprintNodeFactory.SetVerbosity(OriginalNodeFactory);
        LogModels.SetVerbosity(OriginalModels);
    }
}


FString FBP2AIModule::BuildExportFilePath(const FString& PackagePath, const FString& BlueprintName) const
{
    const FString BaseDir = FPaths::ProjectSavedDir() / TEXT("BP2AI/Exports");

    FString RelativePath = PackagePath;
    const FString GameRoot = TEXT("/Game");
    if (RelativePath.StartsWith(GameRoot))
    {
        RelativePath = RelativePath.Mid(GameRoot.Len());
    }
    RelativePath.RemoveFromStart(TEXT("/"));

    FString OutputDir = BaseDir;
    if (!RelativePath.IsEmpty())
    {
        OutputDir = FPaths::Combine(BaseDir, RelativePath);
    }

    return FPaths::Combine(OutputDir, BlueprintName + TEXT(".md"));
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