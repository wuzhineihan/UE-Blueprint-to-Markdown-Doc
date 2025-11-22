// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "BP2AI/Private/Trace/SemanticData.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSemanticData() {}

// Begin Cross Module References
BP2AI_API UEnum* Z_Construct_UEnum_BP2AI_ESemanticNodeType();
BP2AI_API UEnum* Z_Construct_UEnum_BP2AI_ESemanticValueType();
BP2AI_API UScriptStruct* Z_Construct_UScriptStruct_FSemanticArgument();
BP2AI_API UScriptStruct* Z_Construct_UScriptStruct_FSemanticExecutionStep();
BP2AI_API UScriptStruct* Z_Construct_UScriptStruct_FSemanticValue();
UPackage* Z_Construct_UPackage__Script_BP2AI();
// End Cross Module References

// Begin Enum ESemanticNodeType
static FEnumRegistrationInfo Z_Registration_Info_UEnum_ESemanticNodeType;
static UEnum* ESemanticNodeType_StaticEnum()
{
	if (!Z_Registration_Info_UEnum_ESemanticNodeType.OuterSingleton)
	{
		Z_Registration_Info_UEnum_ESemanticNodeType.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_BP2AI_ESemanticNodeType, (UObject*)Z_Construct_UPackage__Script_BP2AI(), TEXT("ESemanticNodeType"));
	}
	return Z_Registration_Info_UEnum_ESemanticNodeType.OuterSingleton;
}
template<> BP2AI_API UEnum* StaticEnum<ESemanticNodeType>()
{
	return ESemanticNodeType_StaticEnum();
}
struct Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Branch.Comment", "// Represents a Sequence node\n" },
		{ "Branch.DisplayName", "Branch" },
		{ "Branch.Name", "ESemanticNodeType::Branch" },
		{ "Branch.ToolTip", "Represents a Sequence node" },
		{ "CollapsedGraph.Comment", "// Represents an invocation of a macro\n" },
		{ "CollapsedGraph.DisplayName", "Collapsed Graph" },
		{ "CollapsedGraph.Name", "ESemanticNodeType::CollapsedGraph" },
		{ "CollapsedGraph.ToolTip", "Represents an invocation of a macro" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Semantic node types for execution steps.\n * This enum helps categorize the primary action or nature of an execution step.\n */" },
#endif
		{ "Comment.Comment", "// General keywords (e.g., \"Set\" for a variable set operation if not using VariableSet type)\n" },
		{ "Comment.DisplayName", "Comment" },
		{ "Comment.Name", "ESemanticNodeType::Comment" },
		{ "Comment.ToolTip", "General keywords (e.g., \"Set\" for a variable set operation if not using VariableSet type)" },
		{ "Conversion.Comment", "// Represents a data type name (e.g., FVector, int32)\n" },
		{ "Conversion.DisplayName", "Conversion" },
		{ "Conversion.Name", "ESemanticNodeType::Conversion" },
		{ "Conversion.ToolTip", "Represents a data type name (e.g., FVector, int32)" },
		{ "CustomEvent.Comment", "// Represents an event node (e.g., BeginPlay, Custom Event declaration)\n" },
		{ "CustomEvent.DisplayName", "Custom Event Call" },
		{ "CustomEvent.Name", "ESemanticNodeType::CustomEvent" },
		{ "CustomEvent.ToolTip", "Represents an event node (e.g., BeginPlay, Custom Event declaration)" },
		{ "CustomEventCall.Comment", "// Represents a call to an interface function\n" },
		{ "CustomEventCall.DisplayName", "Custom Event Call" },
		{ "CustomEventCall.Name", "ESemanticNodeType::CustomEventCall" },
		{ "CustomEventCall.ToolTip", "Represents a call to an interface function" },
		{ "DataType.Comment", "// Represents a mathematical or logical operator\n" },
		{ "DataType.DisplayName", "Data Type" },
		{ "DataType.Name", "ESemanticNodeType::DataType" },
		{ "DataType.ToolTip", "Represents a mathematical or logical operator" },
		{ "Delay.Comment", "// Represents a Timeline node\n" },
		{ "Delay.DisplayName", "Delay" },
		{ "Delay.Name", "ESemanticNodeType::Delay" },
		{ "Delay.ToolTip", "Represents a Timeline node" },
		{ "Error.Comment", "// Annotation for the end of an execution path\n" },
		{ "Error.DisplayName", "Error" },
		{ "Error.Name", "ESemanticNodeType::Error" },
		{ "Error.ToolTip", "Annotation for the end of an execution path" },
		{ "Event.Comment", "// Core execution types\n" },
		{ "Event.DisplayName", "Event" },
		{ "Event.Name", "ESemanticNodeType::Event" },
		{ "Event.ToolTip", "Core execution types" },
		{ "FunctionCall.Comment", "// Represents a call to a custom event\n" },
		{ "FunctionCall.DisplayName", "Function Call" },
		{ "FunctionCall.Name", "ESemanticNodeType::FunctionCall" },
		{ "FunctionCall.ToolTip", "Represents a call to a custom event" },
		{ "InterfaceCall.Comment", "// Represents an invocation of a collapsed graph\n" },
		{ "InterfaceCall.DisplayName", "Interface Call" },
		{ "InterfaceCall.Name", "ESemanticNodeType::InterfaceCall" },
		{ "InterfaceCall.ToolTip", "Represents an invocation of a collapsed graph" },
		{ "Keyword.Comment", "// Meta/Informational types\n" },
		{ "Keyword.DisplayName", "Keyword" },
		{ "Keyword.Name", "ESemanticNodeType::Keyword" },
		{ "Keyword.ToolTip", "Meta/Informational types" },
		{ "Literal.Comment", "// Represents a parameter name in a function/event signature or call\n" },
		{ "Literal.DisplayName", "Literal" },
		{ "Literal.Name", "ESemanticNodeType::Literal" },
		{ "Literal.ToolTip", "Represents a parameter name in a function/event signature or call" },
		{ "Loop.Comment", "// Represents a Branch node (IfThenElse, Switch)\n" },
		{ "Loop.DisplayName", "Loop" },
		{ "Loop.Name", "ESemanticNodeType::Loop" },
		{ "Loop.ToolTip", "Represents a Branch node (IfThenElse, Switch)" },
		{ "MacroCall.Comment", "// Represents a call to a parent's version of a function\n" },
		{ "MacroCall.DisplayName", "Macro Call" },
		{ "MacroCall.Name", "ESemanticNodeType::MacroCall" },
		{ "MacroCall.ToolTip", "Represents a call to a parent's version of a function" },
		{ "Modifier.Comment", "// Represents a comment or annotation\n" },
		{ "Modifier.DisplayName", "Modifier" },
		{ "Modifier.Name", "ESemanticNodeType::Modifier" },
		{ "Modifier.ToolTip", "Represents a comment or annotation" },
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
		{ "Operator.Comment", "// Represents a literal value (string, number, bool, etc.)\n" },
		{ "Operator.DisplayName", "Operator" },
		{ "Operator.Name", "ESemanticNodeType::Operator" },
		{ "Operator.ToolTip", "Represents a literal value (string, number, bool, etc.)" },
		{ "Parameter.Comment", "// Represents a variable get/set operation or a variable literal\n" },
		{ "Parameter.DisplayName", "Parameter" },
		{ "Parameter.Name", "ESemanticNodeType::Parameter" },
		{ "Parameter.ToolTip", "Represents a variable get/set operation or a variable literal" },
		{ "ParentFunctionCall.Comment", "// Represents a call to a regular function\n" },
		{ "ParentFunctionCall.DisplayName", "Parent Function Call" },
		{ "ParentFunctionCall.Name", "ESemanticNodeType::ParentFunctionCall" },
		{ "ParentFunctionCall.ToolTip", "Represents a call to a regular function" },
		{ "PathEnd.Comment", "// Represents modifiers like (Latent), (Symbolic)\n" },
		{ "PathEnd.DisplayName", "Path End" },
		{ "PathEnd.Name", "ESemanticNodeType::PathEnd" },
		{ "PathEnd.ToolTip", "Represents modifiers like (Latent), (Symbolic)" },
		{ "PlayMontage.Comment", "// Represents a Delay node\n" },
		{ "PlayMontage.DisplayName", "Play Montage" },
		{ "PlayMontage.Name", "ESemanticNodeType::PlayMontage" },
		{ "PlayMontage.ToolTip", "Represents a Delay node" },
		{ "ReturnNode.Comment", "// Represents a Play Montage node\n" },
		{ "ReturnNode.DisplayName", "Return Node" },
		{ "ReturnNode.Name", "ESemanticNodeType::ReturnNode" },
		{ "ReturnNode.ToolTip", "Represents a Play Montage node" },
		{ "Sequence.Comment", "// Special node representations\n" },
		{ "Sequence.DisplayName", "Sequence" },
		{ "Sequence.Name", "ESemanticNodeType::Sequence" },
		{ "Sequence.ToolTip", "Special node representations" },
		{ "Timeline.Comment", "// Represents a Variable Set node\n" },
		{ "Timeline.DisplayName", "Timeline" },
		{ "Timeline.Name", "ESemanticNodeType::Timeline" },
		{ "Timeline.ToolTip", "Represents a Variable Set node" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Semantic node types for execution steps.\nThis enum helps categorize the primary action or nature of an execution step." },
#endif
		{ "Unknown.DisplayName", "Unknown" },
		{ "Unknown.Name", "ESemanticNodeType::Unknown" },
		{ "Variable.Comment", "// Represents a call to a custom event\n// Data types & operations\n" },
		{ "Variable.DisplayName", "Variable" },
		{ "Variable.Name", "ESemanticNodeType::Variable" },
		{ "Variable.ToolTip", "Represents a call to a custom event\nData types & operations" },
		{ "VariableSet.Comment", "// Represents a loop construct (For Each, While)\n" },
		{ "VariableSet.DisplayName", "Variable Set" },
		{ "VariableSet.Name", "ESemanticNodeType::VariableSet" },
		{ "VariableSet.ToolTip", "Represents a loop construct (For Each, While)" },
	};
#endif // WITH_METADATA
	static constexpr UECodeGen_Private::FEnumeratorParam Enumerators[] = {
		{ "ESemanticNodeType::Unknown", (int64)ESemanticNodeType::Unknown },
		{ "ESemanticNodeType::Event", (int64)ESemanticNodeType::Event },
		{ "ESemanticNodeType::CustomEvent", (int64)ESemanticNodeType::CustomEvent },
		{ "ESemanticNodeType::FunctionCall", (int64)ESemanticNodeType::FunctionCall },
		{ "ESemanticNodeType::ParentFunctionCall", (int64)ESemanticNodeType::ParentFunctionCall },
		{ "ESemanticNodeType::MacroCall", (int64)ESemanticNodeType::MacroCall },
		{ "ESemanticNodeType::CollapsedGraph", (int64)ESemanticNodeType::CollapsedGraph },
		{ "ESemanticNodeType::InterfaceCall", (int64)ESemanticNodeType::InterfaceCall },
		{ "ESemanticNodeType::CustomEventCall", (int64)ESemanticNodeType::CustomEventCall },
		{ "ESemanticNodeType::Variable", (int64)ESemanticNodeType::Variable },
		{ "ESemanticNodeType::Parameter", (int64)ESemanticNodeType::Parameter },
		{ "ESemanticNodeType::Literal", (int64)ESemanticNodeType::Literal },
		{ "ESemanticNodeType::Operator", (int64)ESemanticNodeType::Operator },
		{ "ESemanticNodeType::DataType", (int64)ESemanticNodeType::DataType },
		{ "ESemanticNodeType::Conversion", (int64)ESemanticNodeType::Conversion },
		{ "ESemanticNodeType::Sequence", (int64)ESemanticNodeType::Sequence },
		{ "ESemanticNodeType::Branch", (int64)ESemanticNodeType::Branch },
		{ "ESemanticNodeType::Loop", (int64)ESemanticNodeType::Loop },
		{ "ESemanticNodeType::VariableSet", (int64)ESemanticNodeType::VariableSet },
		{ "ESemanticNodeType::Timeline", (int64)ESemanticNodeType::Timeline },
		{ "ESemanticNodeType::Delay", (int64)ESemanticNodeType::Delay },
		{ "ESemanticNodeType::PlayMontage", (int64)ESemanticNodeType::PlayMontage },
		{ "ESemanticNodeType::ReturnNode", (int64)ESemanticNodeType::ReturnNode },
		{ "ESemanticNodeType::Keyword", (int64)ESemanticNodeType::Keyword },
		{ "ESemanticNodeType::Comment", (int64)ESemanticNodeType::Comment },
		{ "ESemanticNodeType::Modifier", (int64)ESemanticNodeType::Modifier },
		{ "ESemanticNodeType::PathEnd", (int64)ESemanticNodeType::PathEnd },
		{ "ESemanticNodeType::Error", (int64)ESemanticNodeType::Error },
	};
	static const UECodeGen_Private::FEnumParams EnumParams;
};
const UECodeGen_Private::FEnumParams Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::EnumParams = {
	(UObject*(*)())Z_Construct_UPackage__Script_BP2AI,
	nullptr,
	"ESemanticNodeType",
	"ESemanticNodeType",
	Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::Enumerators,
	RF_Public|RF_Transient|RF_MarkAsNative,
	UE_ARRAY_COUNT(Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::Enumerators),
	EEnumFlags::None,
	(uint8)UEnum::ECppForm::EnumClass,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::Enum_MetaDataParams), Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::Enum_MetaDataParams)
};
UEnum* Z_Construct_UEnum_BP2AI_ESemanticNodeType()
{
	if (!Z_Registration_Info_UEnum_ESemanticNodeType.InnerSingleton)
	{
		UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_ESemanticNodeType.InnerSingleton, Z_Construct_UEnum_BP2AI_ESemanticNodeType_Statics::EnumParams);
	}
	return Z_Registration_Info_UEnum_ESemanticNodeType.InnerSingleton;
}
// End Enum ESemanticNodeType

// Begin Enum ESemanticValueType
static FEnumRegistrationInfo Z_Registration_Info_UEnum_ESemanticValueType;
static UEnum* ESemanticValueType_StaticEnum()
{
	if (!Z_Registration_Info_UEnum_ESemanticValueType.OuterSingleton)
	{
		Z_Registration_Info_UEnum_ESemanticValueType.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_BP2AI_ESemanticValueType, (UObject*)Z_Construct_UPackage__Script_BP2AI(), TEXT("ESemanticValueType"));
	}
	return Z_Registration_Info_UEnum_ESemanticValueType.OuterSingleton;
}
template<> BP2AI_API UEnum* StaticEnum<ESemanticValueType>()
{
	return ESemanticValueType_StaticEnum();
}
struct Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Error.DisplayName", "Error Value" },
		{ "Error.Name", "ESemanticValueType::Error" },
		{ "Expression.DisplayName", "Expression" },
		{ "Expression.Name", "ESemanticValueType::Expression" },
		{ "FunctionResult.DisplayName", "Function Result" },
		{ "FunctionResult.Name", "ESemanticValueType::FunctionResult" },
		{ "Literal.DisplayName", "Literal Value" },
		{ "Literal.Name", "ESemanticValueType::Literal" },
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
		{ "Unknown.DisplayName", "Unknown" },
		{ "Unknown.Name", "ESemanticValueType::Unknown" },
		{ "Variable.DisplayName", "Variable Reference" },
		{ "Variable.Name", "ESemanticValueType::Variable" },
	};
#endif // WITH_METADATA
	static constexpr UECodeGen_Private::FEnumeratorParam Enumerators[] = {
		{ "ESemanticValueType::Unknown", (int64)ESemanticValueType::Unknown },
		{ "ESemanticValueType::Literal", (int64)ESemanticValueType::Literal },
		{ "ESemanticValueType::Variable", (int64)ESemanticValueType::Variable },
		{ "ESemanticValueType::FunctionResult", (int64)ESemanticValueType::FunctionResult },
		{ "ESemanticValueType::Expression", (int64)ESemanticValueType::Expression },
		{ "ESemanticValueType::Error", (int64)ESemanticValueType::Error },
	};
	static const UECodeGen_Private::FEnumParams EnumParams;
};
const UECodeGen_Private::FEnumParams Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::EnumParams = {
	(UObject*(*)())Z_Construct_UPackage__Script_BP2AI,
	nullptr,
	"ESemanticValueType",
	"ESemanticValueType",
	Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::Enumerators,
	RF_Public|RF_Transient|RF_MarkAsNative,
	UE_ARRAY_COUNT(Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::Enumerators),
	EEnumFlags::None,
	(uint8)UEnum::ECppForm::EnumClass,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::Enum_MetaDataParams), Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::Enum_MetaDataParams)
};
UEnum* Z_Construct_UEnum_BP2AI_ESemanticValueType()
{
	if (!Z_Registration_Info_UEnum_ESemanticValueType.InnerSingleton)
	{
		UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_ESemanticValueType.InnerSingleton, Z_Construct_UEnum_BP2AI_ESemanticValueType_Statics::EnumParams);
	}
	return Z_Registration_Info_UEnum_ESemanticValueType.InnerSingleton;
}
// End Enum ESemanticValueType

// Begin ScriptStruct FSemanticArgument
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_SemanticArgument;
class UScriptStruct* FSemanticArgument::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticArgument.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_SemanticArgument.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FSemanticArgument, (UObject*)Z_Construct_UPackage__Script_BP2AI(), TEXT("SemanticArgument"));
	}
	return Z_Registration_Info_UScriptStruct_SemanticArgument.OuterSingleton;
}
template<> BP2AI_API UScriptStruct* StaticStruct<FSemanticArgument>()
{
	return FSemanticArgument::StaticStruct();
}
struct Z_Construct_UScriptStruct_FSemanticArgument_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Semantic argument representation for function calls, macro invocations, etc.\n * Replaces formatted argument strings with structured data.\n */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Semantic argument representation for function calls, macro invocations, etc.\nReplaces formatted argument strings with structured data." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Name_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The name of the parameter (e.g., \"InString\", \"TimeOfDay\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The name of the parameter (e.g., \"InString\", \"TimeOfDay\")." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ValueRepresentation_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The string representation of the parameter's value (e.g., \"'Hello'\", \"Loop Element\", \"MyVariable\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The string representation of the parameter's value (e.g., \"'Hello'\", \"Loop Element\", \"MyVariable\")." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ValueType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The semantic type of the value itself (e.g., Literal, Variable, FunctionCall result). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The semantic type of the value itself (e.g., Literal, Variable, FunctionCall result)." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DataType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The data type of the parameter pin (e.g., \"string\", \"float\", \"FVector\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The data type of the parameter pin (e.g., \"string\", \"float\", \"FVector\")." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_Name;
	static const UECodeGen_Private::FStrPropertyParams NewProp_ValueRepresentation;
	static const UECodeGen_Private::FBytePropertyParams NewProp_ValueType_Underlying;
	static const UECodeGen_Private::FEnumPropertyParams NewProp_ValueType;
	static const UECodeGen_Private::FStrPropertyParams NewProp_DataType;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FSemanticArgument>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_Name = { "Name", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticArgument, Name), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Name_MetaData), NewProp_Name_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueRepresentation = { "ValueRepresentation", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticArgument, ValueRepresentation), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ValueRepresentation_MetaData), NewProp_ValueRepresentation_MetaData) };
const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueType_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FEnumPropertyParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueType = { "ValueType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticArgument, ValueType), Z_Construct_UEnum_BP2AI_ESemanticNodeType, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ValueType_MetaData), NewProp_ValueType_MetaData) }; // 294556372
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_DataType = { "DataType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticArgument, DataType), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DataType_MetaData), NewProp_DataType_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FSemanticArgument_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_Name,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueRepresentation,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueType_Underlying,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_ValueType,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewProp_DataType,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticArgument_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FSemanticArgument_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_BP2AI,
	nullptr,
	&NewStructOps,
	"SemanticArgument",
	Z_Construct_UScriptStruct_FSemanticArgument_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticArgument_Statics::PropPointers),
	sizeof(FSemanticArgument),
	alignof(FSemanticArgument),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000201),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticArgument_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FSemanticArgument_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FSemanticArgument()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticArgument.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_SemanticArgument.InnerSingleton, Z_Construct_UScriptStruct_FSemanticArgument_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_SemanticArgument.InnerSingleton;
}
// End ScriptStruct FSemanticArgument

// Begin ScriptStruct FSemanticExecutionStep
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_SemanticExecutionStep;
class UScriptStruct* FSemanticExecutionStep::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticExecutionStep.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_SemanticExecutionStep.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FSemanticExecutionStep, (UObject*)Z_Construct_UPackage__Script_BP2AI(), TEXT("SemanticExecutionStep"));
	}
	return Z_Registration_Info_UScriptStruct_SemanticExecutionStep.OuterSingleton;
}
template<> BP2AI_API UScriptStruct* StaticStruct<FSemanticExecutionStep>()
{
	return FSemanticExecutionStep::StaticStruct();
}
struct Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Core semantic representation of a single step in an execution flow.\n * This structure replaces the previously used formatted FString lines.\n */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Core semantic representation of a single step in an execution flow.\nThis structure replaces the previously used formatted FString lines." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The primary semantic type of this execution step. */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The primary semantic type of this execution step." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeName_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The clean, unformatted name of the node or item (e.g., \"PrintString\", \"MyEvent\", \"MyVariable\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The clean, unformatted name of the node or item (e.g., \"PrintString\", \"MyEvent\", \"MyVariable\")." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_TargetExpression_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** For targeted calls like \"MyObject.DoSomething\", this would hold \"MyObject\". */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "For targeted calls like \"MyObject.DoSomething\", this would hold \"MyObject\"." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Arguments_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Structured list of arguments for function/macro calls. */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Structured list of arguments for function/macro calls." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_LinkAnchor_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Anchor ID for creating navigable links, e.g., \"#my-function-definition\". */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Anchor ID for creating navigable links, e.g., \"#my-function-definition\"." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IndentPrefix_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The visual prefix for indentation in tree-like views (e.g., \"    |-- \"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The visual prefix for indentation in tree-like views (e.g., \"    |-- \")." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bIsRepeat_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Flag indicating if this step represents a repeated call already detailed elsewhere. */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Flag indicating if this step represents a repeated call already detailed elsewhere." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bIsLatent_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Flag indicating if this step represents a latent action. */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Flag indicating if this step represents a latent action." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BranchType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** For branching nodes, indicates the type of branch taken (e.g., \"then_0\", \"LoopBody\", \"Completed\", \"True\", \"False\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "For branching nodes, indicates the type of branch taken (e.g., \"then_0\", \"LoopBody\", \"Completed\", \"True\", \"False\")." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FBytePropertyParams NewProp_NodeType_Underlying;
	static const UECodeGen_Private::FEnumPropertyParams NewProp_NodeType;
	static const UECodeGen_Private::FStrPropertyParams NewProp_NodeName;
	static const UECodeGen_Private::FStrPropertyParams NewProp_TargetExpression;
	static const UECodeGen_Private::FStructPropertyParams NewProp_Arguments_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_Arguments;
	static const UECodeGen_Private::FStrPropertyParams NewProp_LinkAnchor;
	static const UECodeGen_Private::FStrPropertyParams NewProp_IndentPrefix;
	static void NewProp_bIsRepeat_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bIsRepeat;
	static void NewProp_bIsLatent_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bIsLatent;
	static const UECodeGen_Private::FStrPropertyParams NewProp_BranchType;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FSemanticExecutionStep>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeType_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FEnumPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeType = { "NodeType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, NodeType), Z_Construct_UEnum_BP2AI_ESemanticNodeType, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeType_MetaData), NewProp_NodeType_MetaData) }; // 294556372
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeName = { "NodeName", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, NodeName), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeName_MetaData), NewProp_NodeName_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_TargetExpression = { "TargetExpression", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, TargetExpression), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_TargetExpression_MetaData), NewProp_TargetExpression_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_Arguments_Inner = { "Arguments", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UScriptStruct_FSemanticArgument, METADATA_PARAMS(0, nullptr) }; // 282211552
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_Arguments = { "Arguments", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, Arguments), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Arguments_MetaData), NewProp_Arguments_MetaData) }; // 282211552
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_LinkAnchor = { "LinkAnchor", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, LinkAnchor), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_LinkAnchor_MetaData), NewProp_LinkAnchor_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_IndentPrefix = { "IndentPrefix", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, IndentPrefix), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IndentPrefix_MetaData), NewProp_IndentPrefix_MetaData) };
void Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsRepeat_SetBit(void* Obj)
{
	((FSemanticExecutionStep*)Obj)->bIsRepeat = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsRepeat = { "bIsRepeat", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FSemanticExecutionStep), &Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsRepeat_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bIsRepeat_MetaData), NewProp_bIsRepeat_MetaData) };
void Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsLatent_SetBit(void* Obj)
{
	((FSemanticExecutionStep*)Obj)->bIsLatent = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsLatent = { "bIsLatent", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FSemanticExecutionStep), &Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsLatent_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bIsLatent_MetaData), NewProp_bIsLatent_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_BranchType = { "BranchType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticExecutionStep, BranchType), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BranchType_MetaData), NewProp_BranchType_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeType_Underlying,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeType,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_NodeName,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_TargetExpression,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_Arguments_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_Arguments,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_LinkAnchor,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_IndentPrefix,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsRepeat,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_bIsLatent,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewProp_BranchType,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_BP2AI,
	nullptr,
	&NewStructOps,
	"SemanticExecutionStep",
	Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::PropPointers),
	sizeof(FSemanticExecutionStep),
	alignof(FSemanticExecutionStep),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000201),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FSemanticExecutionStep()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticExecutionStep.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_SemanticExecutionStep.InnerSingleton, Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_SemanticExecutionStep.InnerSingleton;
}
// End ScriptStruct FSemanticExecutionStep

// Begin ScriptStruct FSemanticValue
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_SemanticValue;
class UScriptStruct* FSemanticValue::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticValue.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_SemanticValue.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FSemanticValue, (UObject*)Z_Construct_UPackage__Script_BP2AI(), TEXT("SemanticValue"));
	}
	return Z_Registration_Info_UScriptStruct_SemanticValue.OuterSingleton;
}
template<> BP2AI_API UScriptStruct* StaticStruct<FSemanticValue>()
{
	return FSemanticValue::StaticStruct();
}
struct Z_Construct_UScriptStruct_FSemanticValue_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Represents a resolved value in a structured, semantic way.\n * Used during the semantic tracing phase before final string formatting.\n */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Represents a resolved value in a structured, semantic way.\nUsed during the semantic tracing phase before final string formatting." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ValueRepresentation_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The string representation of the value (e.g., \"5\", \"'Hello'\", \"`MyVar`\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The string representation of the value (e.g., \"5\", \"'Hello'\", \"`MyVar`\")." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NodeType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The semantic type of this value. */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The semantic type of this value." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DataType_MetaData[] = {
		{ "Category", "Semantic Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The data type of this value (e.g., \"int\", \"string\", \"bool\"). */" },
#endif
		{ "ModuleRelativePath", "Private/Trace/SemanticData.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The data type of this value (e.g., \"int\", \"string\", \"bool\")." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ValueRepresentation;
	static const UECodeGen_Private::FBytePropertyParams NewProp_NodeType_Underlying;
	static const UECodeGen_Private::FEnumPropertyParams NewProp_NodeType;
	static const UECodeGen_Private::FStrPropertyParams NewProp_DataType;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FSemanticValue>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_ValueRepresentation = { "ValueRepresentation", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticValue, ValueRepresentation), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ValueRepresentation_MetaData), NewProp_ValueRepresentation_MetaData) };
const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_NodeType_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FEnumPropertyParams Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_NodeType = { "NodeType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticValue, NodeType), Z_Construct_UEnum_BP2AI_ESemanticNodeType, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NodeType_MetaData), NewProp_NodeType_MetaData) }; // 294556372
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_DataType = { "DataType", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FSemanticValue, DataType), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DataType_MetaData), NewProp_DataType_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FSemanticValue_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_ValueRepresentation,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_NodeType_Underlying,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_NodeType,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSemanticValue_Statics::NewProp_DataType,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticValue_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FSemanticValue_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_BP2AI,
	nullptr,
	&NewStructOps,
	"SemanticValue",
	Z_Construct_UScriptStruct_FSemanticValue_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticValue_Statics::PropPointers),
	sizeof(FSemanticValue),
	alignof(FSemanticValue),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000201),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FSemanticValue_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FSemanticValue_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FSemanticValue()
{
	if (!Z_Registration_Info_UScriptStruct_SemanticValue.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_SemanticValue.InnerSingleton, Z_Construct_UScriptStruct_FSemanticValue_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_SemanticValue.InnerSingleton;
}
// End ScriptStruct FSemanticValue

// Begin Registration
struct Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_Statics
{
	static constexpr FEnumRegisterCompiledInInfo EnumInfo[] = {
		{ ESemanticNodeType_StaticEnum, TEXT("ESemanticNodeType"), &Z_Registration_Info_UEnum_ESemanticNodeType, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 294556372U) },
		{ ESemanticValueType_StaticEnum, TEXT("ESemanticValueType"), &Z_Registration_Info_UEnum_ESemanticValueType, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 837173170U) },
	};
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FSemanticArgument::StaticStruct, Z_Construct_UScriptStruct_FSemanticArgument_Statics::NewStructOps, TEXT("SemanticArgument"), &Z_Registration_Info_UScriptStruct_SemanticArgument, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FSemanticArgument), 282211552U) },
		{ FSemanticExecutionStep::StaticStruct, Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics::NewStructOps, TEXT("SemanticExecutionStep"), &Z_Registration_Info_UScriptStruct_SemanticExecutionStep, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FSemanticExecutionStep), 364221205U) },
		{ FSemanticValue::StaticStruct, Z_Construct_UScriptStruct_FSemanticValue_Statics::NewStructOps, TEXT("SemanticValue"), &Z_Registration_Info_UScriptStruct_SemanticValue, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FSemanticValue), 3303487526U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_2482848309(TEXT("/Script/BP2AI"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_Statics::ScriptStructInfo),
	Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_Statics::EnumInfo));
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
