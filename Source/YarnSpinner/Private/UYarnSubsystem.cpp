// Fill out your copyright notice in the Description page of Project Settings.


#include "UYarnSubsystem.h"

#include "Misc/YSLogging.h"


void UUYarnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);
}


void UUYarnSubsystem::Deinitialize()
{
    YS_LOG_FUNCSIG
    Super::Deinitialize();
}

