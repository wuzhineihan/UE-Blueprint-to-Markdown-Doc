/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/SemanticData.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h" // Required for UENUM/USTRUCT macros
#include "SemanticData.generated.h" // For UENUM and USTRUCT

/**
 * Semantic node types for execution steps.
 * This enum helps categorize the primary action or nature of an execution step.
 */
UENUM(BlueprintType)
enum class ESemanticNodeType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),

    // Core execution types
    Event UMETA(DisplayName = "Event"),                      // Represents an event node (e.g., BeginPlay, Custom Event declaration)
    CustomEvent UMETA(DisplayName = "Custom Event Call"),    // Represents a call to a custom event
    FunctionCall UMETA(DisplayName = "Function Call"),       // Represents a call to a regular function
    ParentFunctionCall UMETA(DisplayName = "Parent Function Call"), // Represents a call to a parent's version of a function
    MacroCall UMETA(DisplayName = "Macro Call"),             // Represents an invocation of a macro
    CollapsedGraph UMETA(DisplayName = "Collapsed Graph"),   // Represents an invocation of a collapsed graph
    InterfaceCall UMETA(DisplayName = "Interface Call"),     // Represents a call to an interface function
    CustomEventCall UMETA(DisplayName = "Custom Event Call"), // Represents a call to a custom event
    // Data types & operations
    Variable UMETA(DisplayName = "Variable"),               // Represents a variable get/set operation or a variable literal
    Parameter UMETA(DisplayName = "Parameter"),             // Represents a parameter name in a function/event signature or call
    Literal UMETA(DisplayName = "Literal"),                 // Represents a literal value (string, number, bool, etc.)
    Operator UMETA(DisplayName = "Operator"),               // Represents a mathematical or logical operator
    DataType UMETA(DisplayName = "Data Type"),              // Represents a data type name (e.g., FVector, int32)
    Conversion UMETA(DisplayName = "Conversion"),            // Represents a type conversion operation

    // Special node representations
    Sequence UMETA(DisplayName = "Sequence"),               // Represents a Sequence node
    Branch UMETA(DisplayName = "Branch"),                   // Represents a Branch node (IfThenElse, Switch)
    Loop UMETA(DisplayName = "Loop"),                       // Represents a loop construct (For Each, While)
    VariableSet UMETA(DisplayName = "Variable Set"),         // Represents a Variable Set node
    Timeline UMETA(DisplayName = "Timeline"),               // Represents a Timeline node
    Delay UMETA(DisplayName = "Delay"),                     // Represents a Delay node
    PlayMontage UMETA(DisplayName = "Play Montage"),         // Represents a Play Montage node
    ReturnNode UMETA(DisplayName = "Return Node"),           // Represents a Return node from a function
    
    // Meta/Informational types
    Keyword UMETA(DisplayName = "Keyword"),                 // General keywords (e.g., "Set" for a variable set operation if not using VariableSet type)
    Comment UMETA(DisplayName = "Comment"),                 // Represents a comment or annotation
    Modifier UMETA(DisplayName = "Modifier"),               // Represents modifiers like (Latent), (Symbolic)
    PathEnd UMETA(DisplayName = "Path End"),                 // Annotation for the end of an execution path
    Error UMETA(DisplayName = "Error")                     // Represents an error in the execution path
};
UENUM(BlueprintType)
enum class ESemanticValueType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Literal UMETA(DisplayName = "Literal Value"),
    Variable UMETA(DisplayName = "Variable Reference"), 
    FunctionResult UMETA(DisplayName = "Function Result"),
    Expression UMETA(DisplayName = "Expression"),
    Error UMETA(DisplayName = "Error Value")
};


/**
 * Semantic argument representation for function calls, macro invocations, etc.
 * Replaces formatted argument strings with structured data.
 */
USTRUCT(BlueprintType)
struct BP2AI_API FSemanticArgument
{
    GENERATED_BODY()

    /** The name of the parameter (e.g., "InString", "TimeOfDay"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString Name;

    /** The string representation of the parameter's value (e.g., "'Hello'", "Loop Element", "MyVariable"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString ValueRepresentation;

    /** The semantic type of the value itself (e.g., Literal, Variable, FunctionCall result). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    ESemanticNodeType ValueType = ESemanticNodeType::Unknown;

    /** The data type of the parameter pin (e.g., "string", "float", "FVector"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString DataType;

    FSemanticArgument() = default;
    FSemanticArgument(const FString& InName, const FString& InValueRepresentation, ESemanticNodeType InValueType = ESemanticNodeType::Literal, const FString& InDataType = TEXT(""))
        : Name(InName), ValueRepresentation(InValueRepresentation), ValueType(InValueType), DataType(InDataType) {}
};

/**
 * Core semantic representation of a single step in an execution flow.
 * This structure replaces the previously used formatted FString lines.
 */
USTRUCT(BlueprintType)
struct BP2AI_API FSemanticExecutionStep
{
    GENERATED_BODY()

    /** The primary semantic type of this execution step. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    ESemanticNodeType NodeType = ESemanticNodeType::Unknown;

    /** The clean, unformatted name of the node or item (e.g., "PrintString", "MyEvent", "MyVariable"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString NodeName;

    /** For targeted calls like "MyObject.DoSomething", this would hold "MyObject". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString TargetExpression;

    /** Structured list of arguments for function/macro calls. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    TArray<FSemanticArgument> Arguments;

    /** Anchor ID for creating navigable links, e.g., "#my-function-definition". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString LinkAnchor;

    /** The visual prefix for indentation in tree-like views (e.g., "    |-- "). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString IndentPrefix;

    /** Flag indicating if this step represents a repeated call already detailed elsewhere. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    bool bIsRepeat = false;

    /** Flag indicating if this step represents a latent action. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    bool bIsLatent = false;

    /** For branching nodes, indicates the type of branch taken (e.g., "then_0", "LoopBody", "Completed", "True", "False"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString BranchType;

    FSemanticExecutionStep() = default;
};

/**
 * Represents a resolved value in a structured, semantic way.
 * Used during the semantic tracing phase before final string formatting.
 */

USTRUCT(BlueprintType)
struct BP2AI_API FSemanticValue
{
    GENERATED_BODY()

    /** The string representation of the value (e.g., "5", "'Hello'", "`MyVar`"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString ValueRepresentation;

    /** The semantic type of this value. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    ESemanticNodeType NodeType = ESemanticNodeType::Unknown;

    /** The data type of this value (e.g., "int", "string", "bool"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Semantic Data")
    FString DataType;

    FSemanticValue() = default;
    FSemanticValue(const FString& InValueRepresentation, ESemanticNodeType InNodeType, const FString& InDataType = TEXT(""))
        : ValueRepresentation(InValueRepresentation), NodeType(InNodeType), DataType(InDataType) {}
};