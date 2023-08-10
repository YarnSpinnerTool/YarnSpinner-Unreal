// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnFunctionLibrary.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/WidgetComponent.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/YSLogging.h"



// Sets default values
AYarnFunctionLibrary::AYarnFunctionLibrary()
{
    YS_LOG_FUNCSIG
    
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Attempt to create subobjects from Blueprint-defined Blueprint Function Libraries

    FARFilter Filter;
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
    Filter.PackageNames.Add(TEXT("/Game"));
    TArray<FAssetData> AssetData;
    FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
    if (AssetData.Num() == 0)
    {
        YS_WARN_FUNC("Asset search found nothing")
    }
    for (auto Asset : AssetData)
    {
        // CreateDefaultSubobject<UWidgetComponent>()
        
        YS_LOG_FUNC("Found asset: %s", *Asset.AssetName.ToString())
        // Asset.GetClass()->CreateDefaultSubobject<UBlueprintFunctionLibrary>()
        // Asset.GetClass()->
        // NewObject<UBlueprintFunctionLibrary>(Asset.GetClass());
        // CreateDefaultSubobject<UBlueprintFunctionLibrary>(Asset.AssetName, false);
        // Create

        // NewObject
        
    }
}



// Called when the game starts or when spawned
void AYarnFunctionLibrary::BeginPlay()
{
    Super::BeginPlay();
    
}


// Called every frame
void AYarnFunctionLibrary::Tick(float DeltaTime)
{
    YS_LOG_FUNCSIG
    Super::Tick(DeltaTime);

    auto Ar = FOutputDeviceNull();
    CallFunctionByNameWithArguments(TEXT("MyQuickActorFunction 11"), Ar, nullptr, true);
}

