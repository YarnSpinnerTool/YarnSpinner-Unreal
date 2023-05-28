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


