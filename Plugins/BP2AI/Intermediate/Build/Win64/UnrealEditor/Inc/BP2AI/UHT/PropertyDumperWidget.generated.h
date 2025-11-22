// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Widgets/PropertyDumperWidget.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef BP2AI_PropertyDumperWidget_generated_h
#error "PropertyDumperWidget.generated.h already included, missing '#pragma once' in PropertyDumperWidget.h"
#endif
#define BP2AI_PropertyDumperWidget_generated_h

#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execOnRunCurrentPhaseTestClicked); \
	DECLARE_FUNCTION(execOnAnalyzeBPFunctionsButtonClicked); \
	DECLARE_FUNCTION(execOnDumpSelectedNodePinsClicked); \
	DECLARE_FUNCTION(execOnDumpPropertiesClicked);


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUPropertyDumperWidget(); \
	friend struct Z_Construct_UClass_UPropertyDumperWidget_Statics; \
public: \
	DECLARE_CLASS(UPropertyDumperWidget, UEditorUtilityWidget, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/BP2AI"), NO_API) \
	DECLARE_SERIALIZER(UPropertyDumperWidget)


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UPropertyDumperWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UPropertyDumperWidget(UPropertyDumperWidget&&); \
	UPropertyDumperWidget(const UPropertyDumperWidget&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UPropertyDumperWidget); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UPropertyDumperWidget); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UPropertyDumperWidget) \
	NO_API virtual ~UPropertyDumperWidget();


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_31_PROLOG
#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_INCLASS_NO_PURE_DECLS \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h_34_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> BP2AI_API UClass* StaticClass<class UPropertyDumperWidget>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_Widgets_PropertyDumperWidget_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
