/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Utils/MarkdownTracerUtils.h


#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h" // Added for TSet
#include "Containers/StringFwd.h" // Added for FString forward declaration if TMap key needs it (not strictly here, but good include)
#include "Containers/Map.h" // Added for TMap
#include "Trace/FMarkdownPathTracer.h"


// Forward declarations
class FBlueprintPin;

/**
 * Namespace containing general utility functions used by the Markdown Tracer.
 */
namespace MarkdownTracerUtils
{
	BP2AI_API bool IsTrivialDefault(
		TSharedPtr<const FBlueprintPin> Pin
	);

	BP2AI_API FString ExtractSimpleNameFromPath(
		const FString& Path,
		const FString& CurrentBlueprintContextName = TEXT("")
	);

	BP2AI_API FString NormalizeConversionName(
		const FString& FuncName,
		const TMap<FString, FString>& ConversionMap // Pass map for checking
	);
	
	// New declaration for known static libraries
	BP2AI_API const TSet<FString>& GetKnownStaticBlueprintLibraries();
	
	/**
	 * Generate canonical DefKey for graph to ensure consistency
	 * Same graph always gets same DefKey regardless of call context
	 */
	BP2AI_API FString GetCanonicalDefKey(
		const FString& GraphPath, 
		const FString& GraphNameHint, 
		FMarkdownPathTracer::EUserGraphType GraphType
	);
	
	  class BP2AI_API FGraphNameNormalizer
        {
        public:
            /**
             * Normalize GraphNameHint to canonical form for DefKey generation
             * Ensures same graph always gets same DefKey regardless of call context
             */
            static FString NormalizeForDefKey(const FString& GraphNameHint, FMarkdownPathTracer::EUserGraphType GraphType);
            
            /**
             * Extract consistent context and item names from hint
             * Returns {ContextName, ItemName}
             */
            static TTuple<FString, FString> SplitContextAndItem(const FString& GraphNameHint);


	  	
        private:
            /**
             * Replace all separators (: . ::) with consistent separator
             */
            static FString NormalizeSeparators(const FString& Input);
            
            /**
             * Remove common prefixes/suffixes that vary by context
             */
            static FString CleanVariableAffixes(const FString& Input);
        };


	BP2AI_API FString GetReferenceTypeSuffix(const FString& Category);

} // namespace MarkdownTracerUtils