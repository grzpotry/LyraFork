// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "LocalSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SAVEGAME_API ULocalSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	void Save(USaveGameSubsystem* Game);

	UPROPERTY(SaveGame)
	FGameInstanceRecord GameRecord;
};
