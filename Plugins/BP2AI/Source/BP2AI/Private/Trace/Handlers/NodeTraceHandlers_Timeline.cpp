/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Timeline.cpp


#include "NodeTraceHandlers_Timeline.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"

//----------------------------------------------------------------//
// Timeline Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Timeline::HandleTimeline(
	TSharedPtr<const FBlueprintNode> Node,
	TSharedPtr<const FBlueprintPin> OutputPin,
	FMarkdownDataTracer* Tracer,
	const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
	int32 Depth,
	TSet<FString>& VisitedPins,
	const FBlueprintDataExtractor& DataExtractor,
	TSharedPtr<const FBlueprintNode> CallingNode,
	const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
	bool bSymbolicTrace,
	const FString& CurrentBlueprintContext
	)
{
	check(Node.IsValid() && Node->NodeType == TEXT("Timeline"));
	check(Tracer);
	check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleTimeline: Processing node %s for output pin %s"), *Node->Guid, *OutputPin->Name);

	// Get the timeline name from properties
	const FString* TimelineNamePtr = Node->RawProperties.Find(TEXT("TimelineName"));
	const FString TimelineName = TimelineNamePtr ? **TimelineNamePtr : TEXT("Timeline");

	// Format symbolically as TimelineName.PinName
	FString Result = FMarkdownSpan::TimelineName(FString::Printf(TEXT("%s"), *TimelineName))
					 + TEXT(".")
					 + FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name));

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleTimeline: Returning %s"), *Result);
	return Result;
}

//----------------------------------------------------------------//