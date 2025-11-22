/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/BP2AIStyle.cpp


#include "BP2AIStyle.h"
#include "Logging/BP2AILog.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/StyleColors.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h" // Include for FPaths
#include "Logging/LogMacros.h" // Include for logging

TSharedPtr<FSlateStyleSet> FBP2AIStyle::StyleInstance = nullptr;

void FBP2AIStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FBP2AIStyle::Shutdown()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FBP2AIStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("BP2AIStyle"));
    return StyleSetName;
}

const ISlateStyle& FBP2AIStyle::Get()
{
    // Ensure StyleInstance is valid before dereferencing
    check(StyleInstance.IsValid());
    return *StyleInstance;
}


void FBP2AIStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}
TSharedRef<FSlateStyleSet> FBP2AIStyle::Create()
{
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("BP2AI"); // Get the plugin pointer

    if (Plugin.IsValid()) // Check if the plugin was found
    {
        Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));

        // *** UPDATED: Point to your new icon file name here ***
        FString IconPath = Style->RootToContentDir(TEXT("BP2AI_Icon.png")); // Or whatever you named your file
        
        // Normalize path for consistent logging
        FString NormalizedIconPath = FPaths::ConvertRelativePathToFull(IconPath);
        UE_LOG(LogBP2AI, Log, TEXT("BP2AIStyle: Attempting to load icon from resolved path: %s"), *NormalizedIconPath);

        // Check if file exists (Basic check)
        if (!FPaths::FileExists(NormalizedIconPath))
        {
            UE_LOG(LogBP2AI, Warning, TEXT("BP2AIStyle: Icon file NOT FOUND at path: %s"), *NormalizedIconPath);
        }


        // Add the icon for the toolbar button - MUST match the name used in FSlateIcon
        // Using 16x16 for the display size in the toolbar
        Style->Set("BP2AI.GenerateMarkdown", new FSlateImageBrush(IconPath, FVector2D(16.0f, 16.0f)));
    }
    else
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AIStyle: Could not find plugin 'BP2AI'. Style set may be incomplete."));
    }


    return Style;
}
