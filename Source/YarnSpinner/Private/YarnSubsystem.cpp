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


void UYarnSubsystem::SetValue(std::string name, bool value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


void UYarnSubsystem::SetValue(std::string name, float value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


void UYarnSubsystem::SetValue(std::string name, std::string value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


bool UYarnSubsystem::HasValue(std::string name)
{
    return Variables.Contains(FString(UTF8_TO_TCHAR(name.c_str())));
}


Yarn::Value UYarnSubsystem::GetValue(std::string name)
{
    return Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str())));
}


void UYarnSubsystem::ClearValue(std::string name)
{
    Variables.Remove(FString(UTF8_TO_TCHAR(name.c_str())));
}



