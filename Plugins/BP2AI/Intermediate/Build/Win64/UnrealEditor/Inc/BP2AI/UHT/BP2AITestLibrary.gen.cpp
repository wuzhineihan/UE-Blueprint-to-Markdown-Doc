// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "BP2AI/Public/BP2AITestLibrary.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeBP2AITestLibrary() {}

// Begin Cross Module References
BP2AI_API UClass* Z_Construct_UClass_UBP2AITestLibrary();
BP2AI_API UClass* Z_Construct_UClass_UBP2AITestLibrary_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
UPackage* Z_Construct_UPackage__Script_BP2AI();
// End Cross Module References

// Begin Class UBP2AITestLibrary Function RunTask13Test
struct Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "BP2AI|Testing" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n     * \xe8\xbf\x90\xe8\xa1\x8c Task 1.3 \xe7\x9a\x84\xe6\xb5\x8b\xe8\xaf\x95\n     * \xe5\x8f\xaf\xe4\xbb\xa5\xe5\x9c\xa8\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe7\x9a\x84\xe8\x93\x9d\xe5\x9b\xbe\xe4\xb8\xad\xe8\xb0\x83\xe7\x94\xa8\xef\xbc\x8c\xe6\x88\x96\xe8\x80\x85\xe9\x80\x9a\xe8\xbf\x87\xe6\x8e\xa7\xe5\x88\xb6\xe5\x8f\xb0\xe5\x91\xbd\xe4\xbb\xa4\xe8\xb0\x83\xe7\x94\xa8\n     */" },
#endif
		{ "ModuleRelativePath", "Public/BP2AITestLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xe8\xbf\x90\xe8\xa1\x8c Task 1.3 \xe7\x9a\x84\xe6\xb5\x8b\xe8\xaf\x95\n\xe5\x8f\xaf\xe4\xbb\xa5\xe5\x9c\xa8\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe7\x9a\x84\xe8\x93\x9d\xe5\x9b\xbe\xe4\xb8\xad\xe8\xb0\x83\xe7\x94\xa8\xef\xbc\x8c\xe6\x88\x96\xe8\x80\x85\xe9\x80\x9a\xe8\xbf\x87\xe6\x8e\xa7\xe5\x88\xb6\xe5\x8f\xb0\xe5\x91\xbd\xe4\xbb\xa4\xe8\xb0\x83\xe7\x94\xa8" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UBP2AITestLibrary, nullptr, "RunTask13Test", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test_Statics::Function_MetaDataParams), Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UBP2AITestLibrary::execRunTask13Test)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	UBP2AITestLibrary::RunTask13Test();
	P_NATIVE_END;
}
// End Class UBP2AITestLibrary Function RunTask13Test

// Begin Class UBP2AITestLibrary Function TestExportBlueprintByPath
struct Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics
{
	struct BP2AITestLibrary_eventTestExportBlueprintByPath_Parms
	{
		FString BlueprintPath;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "BP2AI|Testing" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n     * \xe6\xb5\x8b\xe8\xaf\x95\xe5\xaf\xbc\xe5\x87\xba\xe6\x8c\x87\xe5\xae\x9a\xe8\xb7\xaf\xe5\xbe\x84\xe7\x9a\x84\xe8\x93\x9d\xe5\x9b\xbe\n     * @param BlueprintPath \xe8\x93\x9d\xe5\x9b\xbe\xe8\xb5\x84\xe4\xba\xa7\xe8\xb7\xaf\xe5\xbe\x84\xef\xbc\x8c\xe5\xa6\x82 \"/Game/Test/BP_TestExport\"\n     */" },
#endif
		{ "ModuleRelativePath", "Public/BP2AITestLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xe6\xb5\x8b\xe8\xaf\x95\xe5\xaf\xbc\xe5\x87\xba\xe6\x8c\x87\xe5\xae\x9a\xe8\xb7\xaf\xe5\xbe\x84\xe7\x9a\x84\xe8\x93\x9d\xe5\x9b\xbe\n@param BlueprintPath \xe8\x93\x9d\xe5\x9b\xbe\xe8\xb5\x84\xe4\xba\xa7\xe8\xb7\xaf\xe5\xbe\x84\xef\xbc\x8c\xe5\xa6\x82 \"/Game/Test/BP_TestExport\"" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BlueprintPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_BlueprintPath;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_BlueprintPath = { "BlueprintPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(BP2AITestLibrary_eventTestExportBlueprintByPath_Parms, BlueprintPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BlueprintPath_MetaData), NewProp_BlueprintPath_MetaData) };
void Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((BP2AITestLibrary_eventTestExportBlueprintByPath_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(BP2AITestLibrary_eventTestExportBlueprintByPath_Parms), &Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_BlueprintPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UBP2AITestLibrary, nullptr, "TestExportBlueprintByPath", nullptr, nullptr, Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::PropPointers), sizeof(Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::BP2AITestLibrary_eventTestExportBlueprintByPath_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::Function_MetaDataParams), Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::BP2AITestLibrary_eventTestExportBlueprintByPath_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UBP2AITestLibrary::execTestExportBlueprintByPath)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_BlueprintPath);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UBP2AITestLibrary::TestExportBlueprintByPath(Z_Param_BlueprintPath);
	P_NATIVE_END;
}
// End Class UBP2AITestLibrary Function TestExportBlueprintByPath

// Begin Class UBP2AITestLibrary
void UBP2AITestLibrary::StaticRegisterNativesUBP2AITestLibrary()
{
	UClass* Class = UBP2AITestLibrary::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "RunTask13Test", &UBP2AITestLibrary::execRunTask13Test },
		{ "TestExportBlueprintByPath", &UBP2AITestLibrary::execTestExportBlueprintByPath },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UBP2AITestLibrary);
UClass* Z_Construct_UClass_UBP2AITestLibrary_NoRegister()
{
	return UBP2AITestLibrary::StaticClass();
}
struct Z_Construct_UClass_UBP2AITestLibrary_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Blueprint Function Library for BP2AI Testing\n * \xe6\x8f\x90\xe4\xbe\x9b\xe5\x8f\xaf\xe4\xbb\xa5\xe5\x9c\xa8\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe4\xb8\xad\xe7\x9b\xb4\xe6\x8e\xa5\xe8\xb0\x83\xe7\x94\xa8\xe7\x9a\x84\xe6\xb5\x8b\xe8\xaf\x95\xe5\x87\xbd\xe6\x95\xb0\n */" },
#endif
		{ "IncludePath", "BP2AITestLibrary.h" },
		{ "ModuleRelativePath", "Public/BP2AITestLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Blueprint Function Library for BP2AI Testing\n\xe6\x8f\x90\xe4\xbe\x9b\xe5\x8f\xaf\xe4\xbb\xa5\xe5\x9c\xa8\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe4\xb8\xad\xe7\x9b\xb4\xe6\x8e\xa5\xe8\xb0\x83\xe7\x94\xa8\xe7\x9a\x84\xe6\xb5\x8b\xe8\xaf\x95\xe5\x87\xbd\xe6\x95\xb0" },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UBP2AITestLibrary_RunTask13Test, "RunTask13Test" }, // 1990495801
		{ &Z_Construct_UFunction_UBP2AITestLibrary_TestExportBlueprintByPath, "TestExportBlueprintByPath" }, // 2354939526
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UBP2AITestLibrary>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UBP2AITestLibrary_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
	(UObject* (*)())Z_Construct_UPackage__Script_BP2AI,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UBP2AITestLibrary_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UBP2AITestLibrary_Statics::ClassParams = {
	&UBP2AITestLibrary::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	0,
	0,
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UBP2AITestLibrary_Statics::Class_MetaDataParams), Z_Construct_UClass_UBP2AITestLibrary_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UBP2AITestLibrary()
{
	if (!Z_Registration_Info_UClass_UBP2AITestLibrary.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UBP2AITestLibrary.OuterSingleton, Z_Construct_UClass_UBP2AITestLibrary_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UBP2AITestLibrary.OuterSingleton;
}
template<> BP2AI_API UClass* StaticClass<UBP2AITestLibrary>()
{
	return UBP2AITestLibrary::StaticClass();
}
UBP2AITestLibrary::UBP2AITestLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UBP2AITestLibrary);
UBP2AITestLibrary::~UBP2AITestLibrary() {}
// End Class UBP2AITestLibrary

// Begin Registration
struct Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UBP2AITestLibrary, UBP2AITestLibrary::StaticClass, TEXT("UBP2AITestLibrary"), &Z_Registration_Info_UClass_UBP2AITestLibrary, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UBP2AITestLibrary), 4056800269U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_222645592(TEXT("/Script/BP2AI"),
	Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_CPPPlayGround_Plugins_BP2AI_Source_BP2AI_Public_BP2AITestLibrary_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
