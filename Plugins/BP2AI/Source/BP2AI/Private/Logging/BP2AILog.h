/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Logging/BP2AILog.h


#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h" // Ensures ELogVerbosity and its members (All, Log, etc.) are known

// --- Developer Compile-Time Log Toggles for NON-SHIPPING builds ---
// These control the MAXIMUM verbosity level compiled IN.
// Requires a recompile after changing.
// Values are ELogVerbosity members like All, Verbose, Log, Warning, NoLogging, Fatal, Off.

#define DEV_COMPILE_TIME_VERBOSITY_LogBP2AI Log
#define DEV_COMPILE_TIME_VERBOSITY_LogBlueprintNodeFactory Verbose
#define DEV_COMPILE_TIME_VERBOSITY_LogDataTracer VeryVerbose
#define DEV_COMPILE_TIME_VERBOSITY_LogPathTracer Verbose
#define DEV_COMPILE_TIME_VERBOSITY_LogFormatter Verbose
#define DEV_COMPILE_TIME_VERBOSITY_LogUI Log
#define DEV_COMPILE_TIME_VERBOSITY_LogExtractor Verbose
#define DEV_COMPILE_TIME_VERBOSITY_LogModels Verbose
#define DEV_COMPILE_TIME_VERBOSITY_LogBlueprintMarkdownHTML Verbose

// --- Log Category Declarations ---
// For non-shipping builds, the CompileTimeVerbosity is controlled by the DEV_COMPILE_TIME_VERBOSITY_... macros.
// For shipping builds, CompileTimeVerbosity is set to a restrictive level (e.g., Warning).
// The DefaultRuntimeVerbosity (second arg) is typically 'Log'.

#if !UE_BUILD_SHIPPING
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogBP2AI, Log, DEV_COMPILE_TIME_VERBOSITY_LogBP2AI);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintNodeFactory, Log, DEV_COMPILE_TIME_VERBOSITY_LogBlueprintNodeFactory);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogDataTracer, Log, DEV_COMPILE_TIME_VERBOSITY_LogDataTracer);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogPathTracer, Log, DEV_COMPILE_TIME_VERBOSITY_LogPathTracer);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogFormatter, Log, DEV_COMPILE_TIME_VERBOSITY_LogFormatter);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogUI, Log, DEV_COMPILE_TIME_VERBOSITY_LogUI);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogExtractor, Log, DEV_COMPILE_TIME_VERBOSITY_LogExtractor);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogModels, Log, DEV_COMPILE_TIME_VERBOSITY_LogModels);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintMarkdownHTML, Log, DEV_COMPILE_TIME_VERBOSITY_LogBlueprintMarkdownHTML);

#else // UE_BUILD_SHIPPING
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogBP2AI, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintNodeFactory, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogDataTracer, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogPathTracer, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogFormatter, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogUI, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogExtractor, Warning, Warning);
    BP2AI_API DECLARE_LOG_CATEGORY_EXTERN(LogModels, Warning, Warning);
#endif