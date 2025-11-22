// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Trace/SemanticData.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef BP2AI_SemanticData_generated_h
#error "SemanticData.generated.h already included, missing '#pragma once' in SemanticData.h"
#endif
#define BP2AI_SemanticData_generated_h

#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_76_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FSemanticArgument_Statics; \
	static class UScriptStruct* StaticStruct();


template<> BP2AI_API UScriptStruct* StaticStruct<struct FSemanticArgument>();

#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_106_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FSemanticExecutionStep_Statics; \
	static class UScriptStruct* StaticStruct();


template<> BP2AI_API UScriptStruct* StaticStruct<struct FSemanticExecutionStep>();

#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h_155_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FSemanticValue_Statics; \
	static class UScriptStruct* StaticStruct();


template<> BP2AI_API UScriptStruct* StaticStruct<struct FSemanticValue>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Private_Trace_SemanticData_h


#define FOREACH_ENUM_ESEMANTICNODETYPE(op) \
	op(ESemanticNodeType::Unknown) \
	op(ESemanticNodeType::Event) \
	op(ESemanticNodeType::CustomEvent) \
	op(ESemanticNodeType::FunctionCall) \
	op(ESemanticNodeType::ParentFunctionCall) \
	op(ESemanticNodeType::MacroCall) \
	op(ESemanticNodeType::CollapsedGraph) \
	op(ESemanticNodeType::InterfaceCall) \
	op(ESemanticNodeType::CustomEventCall) \
	op(ESemanticNodeType::Variable) \
	op(ESemanticNodeType::Parameter) \
	op(ESemanticNodeType::Literal) \
	op(ESemanticNodeType::Operator) \
	op(ESemanticNodeType::DataType) \
	op(ESemanticNodeType::Conversion) \
	op(ESemanticNodeType::Sequence) \
	op(ESemanticNodeType::Branch) \
	op(ESemanticNodeType::Loop) \
	op(ESemanticNodeType::VariableSet) \
	op(ESemanticNodeType::Timeline) \
	op(ESemanticNodeType::Delay) \
	op(ESemanticNodeType::PlayMontage) \
	op(ESemanticNodeType::ReturnNode) \
	op(ESemanticNodeType::Keyword) \
	op(ESemanticNodeType::Comment) \
	op(ESemanticNodeType::Modifier) \
	op(ESemanticNodeType::PathEnd) \
	op(ESemanticNodeType::Error) 

enum class ESemanticNodeType : uint8;
template<> struct TIsUEnumClass<ESemanticNodeType> { enum { Value = true }; };
template<> BP2AI_API UEnum* StaticEnum<ESemanticNodeType>();

#define FOREACH_ENUM_ESEMANTICVALUETYPE(op) \
	op(ESemanticValueType::Unknown) \
	op(ESemanticValueType::Literal) \
	op(ESemanticValueType::Variable) \
	op(ESemanticValueType::FunctionResult) \
	op(ESemanticValueType::Expression) \
	op(ESemanticValueType::Error) 

enum class ESemanticValueType : uint8;
template<> struct TIsUEnumClass<ESemanticValueType> { enum { Value = true }; };
template<> BP2AI_API UEnum* StaticEnum<ESemanticValueType>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
