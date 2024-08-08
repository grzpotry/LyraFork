// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveStatics.generated.h"

/**
 * 
 */
UCLASS()
class SAVEGAME_API USaveStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Evehor/SaveLoad")
	static void SerializeObject(UObject* Object, UPARAM(ref) TArray<uint8>& OutData);

	UFUNCTION(BlueprintCallable, Category = "Evehor/SaveLoad")
	static void DeserializeObject(UObject * Object, TArray<uint8> OutData);

	UFUNCTION(BlueprintCallable, Category = "Evehor/SaveLoad")
	static void Save(UObject* Object, UPARAM(ref) FSaveDataRecord& SaveRecord);

	static void RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Interface, const UWorld* World);
	static void RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Interface, USaveGameSubsystem* SaveSystem);

private:
	static void SerializeObject(UObject* Object, FArchive& Archive);
};
