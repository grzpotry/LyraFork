// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveableInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

/** Delegate called from AsyncLoadGameFromSlot. First two parameters are passed in SlotName and UserIndex, third parameter is a bool indicating success (true) or failure (false). */
DECLARE_DYNAMIC_DELEGATE(FAsyncGameLoadedFromSlotDelegate);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FAsyncGameSavedToSlotDelegate, const FString&, SlotName, const int32, PlayerIndex, bool, Status);

/*
 * Stores serialized data for whole game
 */
USTRUCT()
struct FGameInstanceRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<UClass*, FSaveDataRecord> SaveRecordsByClass;

	//TODO: store records by id to support restoring objects other than singletons

	UPROPERTY(SaveGame)
	FSaveDataRecord Root;
};

/**
 * 
 */
UCLASS()
class SAVEGAME_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FGameInstanceRecord Save();

	UFUNCTION(BlueprintCallable)
	void LoadFromSlotAsync(const FString& SlotName, FAsyncGameLoadedFromSlotDelegate LoadedDelegate);

	UFUNCTION(BlueprintCallable)
	void SaveToSlotAsync(const FString& SlotName, FAsyncGameSavedToSlotDelegate SavedDelegate);

	// singleton saveables are identified by it's class, so multiple instances are not allowed yet
	UFUNCTION(BlueprintCallable)
	void RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Saveable);

	UFUNCTION()
	void OnActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	UPROPERTY()
	UObject* RootSaveable = nullptr;

	UPROPERTY()
	TMap<UClass*, UObject*> Saveables;

private:
	int32 GetPlayerSaveIndex() const;
	
	void SaveTo(FGameInstanceRecord& SaveRecord);
	void LoadFrom(FGameInstanceRecord& SaveRecord);

	void LoadGameAsync(FGameInstanceRecord GameRecord, FAsyncGameLoadedFromSlotDelegate LoadedDelegate);
	void LoadRootAsync(FGameInstanceRecord& SaveRecord, TFunction<void()> Callback);

	bool bIsRootLoadingFinished;
};
