// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalSaveGame.h"

void ULocalSaveGame::Save(USaveGameSubsystem* Game)
{
	GameRecord = Game->Save();
}
