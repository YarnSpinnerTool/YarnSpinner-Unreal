// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Misc/GeneratedTypeName.h"


class FYarnSpinnerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    
    virtual bool SupportsDynamicReloading() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FYarnSpinnerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FYarnSpinnerModule>("YarnSpinner");
	}
	
	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("YarnSpinner");
	}
	
private:
	/** Handles for dlls */
	void*	YarnSpinnerCoreLibraryHandle;
	void*	ProtobufLibraryHandle;
};


DECLARE_LOG_CATEGORY_EXTERN(LogYarnSpinner, Log, All);

// Disable for shipping builds
#if NO_LOGGING
    YARNSPINNEREDITOR_API DECLARE_LOG_CATEGORY_EXTERN(YSLogClean, Log, All);
#else
    // Direct implementation of the DECLARE_LOG_CATEGORY_EXTERN macro
    YARNSPINNER_API extern struct FLogCategoryYSLogClean : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> { FORCEINLINE FLogCategoryYSLogClean() : FLogCategory(TEXT("")) {} } YSLogClean;
#endif

#define YS_LOG_CLEAN(Format, ...) UE_LOG(YSLogClean, Log, TEXT(Format), ##__VA_ARGS__)

DECLARE_LOG_CATEGORY_EXTERN(YSLogFuncSig, Log, All);

#if defined(_MSC_VER) && !defined(__clang__)
	#define SIG __FUNCSIG__
#else
	#define SIG __PRETTY_FUNCTION__
#endif

#define YS_LOG_FUNCSIG UE_LOG(YSLogFuncSig, Log, TEXT("%s"), *FString(SIG))


