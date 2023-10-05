// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YarnSpinnerLibraryData.h"
#include "JsonObjectConverter.h"
#include "Misc/YSLogging.h"


TOptional<FYarnSpinnerLibraryData> FYarnSpinnerLibraryData::FromJsonString(const FString& JsonString)
{
    TOptional<FYarnSpinnerLibraryData> LibData;
    FYarnSpinnerLibraryData Data;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Data, 0, 0))
    {
        YS_WARN("Could not parse .ysls file")
    }
    else
    {
        LibData = Data;
    }
    return LibData;
}


FString FYarnSpinnerLibraryData::ToJsonString()
{
    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString<FYarnSpinnerLibraryData>(*this, JsonString, 0, 0);
    return JsonString;
}
