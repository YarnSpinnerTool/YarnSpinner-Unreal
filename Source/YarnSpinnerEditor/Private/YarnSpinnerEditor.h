#pragma once

//#include "UnrealEd.h"
//#include "SlateBasics.h"
//#include "SlateExtras.h"
//#include "Editor/LevelEditor/Public/LevelEditor.h"
//#include "Editor/PropertyEditor/Public/PropertyEditing.h"
#include "CoreMinimal.h"
#include "IAssetTypeActions.h"
#include "IYarnSpinnerModuleInterface.h"


YARNSPINNEREDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogYarnSpinnerEditor, Log, All);

class FYarnSpinnerEditor : public IYarnSpinnerModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual void AddModuleListeners() override;

    static inline FYarnSpinnerEditor& Get()
    {
        return FModuleManager::LoadModuleChecked<FYarnSpinnerEditor>("YarnSpinnerEditor");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("YarnSpinnerEditor");
    }
    
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
};
