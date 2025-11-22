// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "BP2AITestLibrary.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef BP2AI_BP2AITestLibrary_generated_h
#error "BP2AITestLibrary.generated.h already included, missing '#pragma once' in BP2AITestLibrary.h"
#endif
#define BP2AI_BP2AITestLibrary_generated_h

#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execTestExportBlueprintByPath);


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUBP2AITestLibrary(); \
	friend struct Z_Construct_UClass_UBP2AITestLibrary_Statics; \
public: \
	DECLARE_CLASS(UBP2AITestLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/BP2AI"), NO_API) \
	DECLARE_SERIALIZER(UBP2AITestLibrary)


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UBP2AITestLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UBP2AITestLibrary(UBP2AITestLibrary&&); \
	UBP2AITestLibrary(const UBP2AITestLibrary&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UBP2AITestLibrary); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UBP2AITestLibrary); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UBP2AITestLibrary) \
	NO_API virtual ~UBP2AITestLibrary();


#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_17_PROLOG
#define FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_INCLASS_NO_PURE_DECLS \
	FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_20_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> BP2AI_API UClass* StaticClass<class UBP2AITestLibrary>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
