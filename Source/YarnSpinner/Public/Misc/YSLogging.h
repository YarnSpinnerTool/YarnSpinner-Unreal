#pragma once

#include "CoreMinimal.h"
#include "Misc/GeneratedTypeName.h"

YARNSPINNER_API DECLARE_LOG_CATEGORY_EXTERN(LogYarnSpinner, Log, All);

// Disable for shipping builds
#if NO_LOGGING
    YARNSPINNEREDITOR_API DECLARE_LOG_CATEGORY_EXTERN(YSLogClean, Log, All);
#else
    // Direct implementation of the DECLARE_LOG_CATEGORY_EXTERN macro
    YARNSPINNER_API extern struct FLogCategoryYSLogClean : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> { FORCEINLINE FLogCategoryYSLogClean() : FLogCategory(TEXT("")) {} } YSLogClean;
#endif

#define YS_LOG_CLEAN(Format, ...) UE_LOG(YSLogClean, Log, TEXT(Format), ##__VA_ARGS__)

YARNSPINNER_API DECLARE_LOG_CATEGORY_EXTERN(YSLogFuncSig, Log, All);

#if defined(_MSC_VER) && !defined(__clang__)
	#define SIG __FUNCSIG__
#else
	#define SIG __PRETTY_FUNCTION__
#endif

#define YS_LOG_FUNCSIG UE_LOG(YSLogFuncSig, Log, TEXT("%s"), *FString(SIG))

#define YS_LOG(Format, ...) \
{ \
	UE_LOG(LogYarnSpinner, Log, TEXT(Format), ##__VA_ARGS__) \
}

#define YS_LOG_FUNC(Format, ...) \
{ \
	const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogYarnSpinner, Log, TEXT("%s: %s"), *FString(SIG), *_Msg) \
}

#define YS_WARN(Format, ...) \
{ \
	UE_LOG(LogYarnSpinner, Warning, TEXT(Format), ##__VA_ARGS__) \
}

#define YS_WARN_FUNC(Format, ...) \
{ \
	const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogYarnSpinner, Warning, TEXT("%s: %s"), *FString(SIG), *_Msg) \
}

#define YS_ERR(Format, ...) \
{ \
	UE_LOG(LogYarnSpinner, Error, TEXT(Format), ##__VA_ARGS__) \
}

#define YS_ERR_FUNC(Format, ...) \
{ \
	const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogYarnSpinner, Error, TEXT("%s: %s"), *FString(SIG), *_Msg) \
}
