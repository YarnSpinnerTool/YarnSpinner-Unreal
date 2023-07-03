// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnSubsystem.h"

#include "DisplayLine.h"
#include "Misc/YSLogging.h"


struct FDisplayLine;


void UYarnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);
}


void UYarnSubsystem::Deinitialize()
{
    YS_LOG_FUNCSIG
    Super::Deinitialize();
}


// TODO: should maybe stick to FText, supposedly better for localization?
FString UYarnSubsystem::GetLocText(const UYarnProject* YarnProject, const FName& Language, const FName& LineID)
{
    if (!YarnProject)
        return "";

    // Test if the yarn project even contains this line
    if (!YarnProject->Lines.Contains(LineID))
        return "";

    const FName ProjectLocID = FName(YarnProject->GetName() + "_" + Language.ToString());

    if (!LocTextDataTables.Contains(ProjectLocID))
    {
        LocTextDataTables.Emplace(ProjectLocID, YarnProject->GetLocTextDataTable(Language));
    }
    if (!LocTextDataTables[ProjectLocID])
    {
        LocTextDataTables[ProjectLocID] = YarnProject->GetLocTextDataTable(Language);
    }
    if (LocTextDataTables[ProjectLocID])
    if (LocTextDataTables[ProjectLocID] && LocTextDataTables[ProjectLocID]->GetRowMap().Contains(LineID))
    {
        return LocTextDataTables[ProjectLocID]->FindRow<FDisplayLine>(LineID, "", true)->Text.ToString();
    }
    
    // TODO: lookup default language instead before returning the base line data
    return YarnProject->Lines[LineID];
}

