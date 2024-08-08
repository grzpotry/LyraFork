// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"

// wrapper for bytes stream
USTRUCT(BlueprintType)
struct FBlob
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<uint8> Bytes;
};

// container for binary data of single UObject
USTRUCT(BlueprintType)
struct FSaveDataRecord
{
	GENERATED_BODY()

	// most can be serialized in single blob, but in some cases we may need to restore some subobjects so we can store them in separate blobs
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<FBlob> Blobs;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SAVEGAME_API ISaveableInterface
{
	GENERATED_BODY()

public:
	// This function must be implemented in Blueprints
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLoad();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	bool bIsLoadingCompleted();

	// root is loaded on the beginning, responsible for first reinitialization, when completed other saveables are loaded. Exactly one root is expected to exist.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool bIsRoot();

	//extract to separate interface?
	virtual void Save(FSaveDataRecord& SaveRecord);
	virtual void Load(FSaveDataRecord SaveRecord);
};
