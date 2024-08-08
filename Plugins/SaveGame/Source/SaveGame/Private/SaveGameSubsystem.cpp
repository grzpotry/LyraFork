// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameSubsystem.h"
#include "LocalSaveGame.h"
#include "Kismet/GameplayStatics.h"

FGameInstanceRecord USaveGameSubsystem::Save()
{
	FGameInstanceRecord GameRecord;

	if (const auto Saveable = CastChecked<ISaveableInterface>(RootSaveable))
	{
		Saveable->Save(GameRecord.Root);
	}

	SaveTo(GameRecord);
	return GameRecord;
}

void USaveGameSubsystem::LoadFromSlotAsync(const FString& SlotName, FAsyncGameLoadedFromSlotDelegate LoadedDelegate)
{
	FAsyncLoadGameFromSlotDelegate Lambda = FAsyncLoadGameFromSlotDelegate::CreateWeakLambda(
		this, [this, LoadedDelegate]
	(const FString& SlotName, const int32 UserIndex, USaveGame* SavedGame)
		{
			if (const auto SaveGame = Cast<ULocalSaveGame>(SavedGame); ensureMsgf(
				SaveGame, TEXT("Failed to load game from slot %s"), *SlotName))
			{
				LoadGameAsync(SaveGame->GameRecord, LoadedDelegate);
			}
		});


	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, GetPlayerSaveIndex(), Lambda);
}

void USaveGameSubsystem::SaveToSlotAsync(const FString& SlotName, FAsyncGameSavedToSlotDelegate SavedDelegate)
{
	FAsyncSaveGameToSlotDelegate Lambda = FAsyncSaveGameToSlotDelegate::CreateWeakLambda(this, [this, SavedDelegate]
	(const FString& SlotName, const int32 UserIndex, bool bSuccess)
		{
			// ReSharper disable once CppExpressionWithoutSideEffects
			SavedDelegate.ExecuteIfBound(SlotName, UserIndex, bSuccess);
		});

	ULocalSaveGame* SaveGameObj = CastChecked<ULocalSaveGame>(
		UGameplayStatics::CreateSaveGameObject(ULocalSaveGame::StaticClass()));

	SaveGameObj->Save(this);


	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObj, SlotName, GetPlayerSaveIndex(), Lambda);
}

void USaveGameSubsystem::RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Saveable)
{
	if (!ensureMsgf(Saveable, TEXT("Tried to register null saveable")))
	{
		return;
	}

	UObject* SaveableObj = Saveable.GetObject();
	UClass* Class = SaveableObj->GetClass();

	bool bIsRoot = ISaveableInterface::Execute_bIsRoot(SaveableObj);

	if (bIsRoot)
	{
		checkf(RootSaveable == nullptr, TEXT("Other root already already registered %s"), *RootSaveable->GetFullName());

		RootSaveable = SaveableObj;
		UE_LOG(LogTemp, Log, TEXT("Registered saveable root %s"), *Class->GetName());

		return;
	}

	if (Saveables.Num() > 0 && ensureMsgf(Saveables.Contains(Class),
	                                      TEXT("Singleton saveable %s is already registered. Saveables: %i"),
	                                      *Class->GetName(), Saveables.Num()))
	{
		return;
	}

	Saveables.Add(Class, SaveableObj);
	UE_LOG(LogTemp, Log, TEXT("Registered saveable [%i] %s"), Saveables.Num(), *Class->GetName());

	if (AActor* Actor = Cast<AActor>(SaveableObj))
	{
		Actor->OnEndPlay.AddDynamic(this, &USaveGameSubsystem::OnActorEndPlay);
	}
}

void USaveGameSubsystem::OnActorEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	const UClass* Class = Actor->GetClass();
	Saveables.Remove(Class);

	const FString ReasonStr = StaticEnum<EEndPlayReason::Type>()->GetValueAsString(Reason);
	UE_LOG(LogTemp, Log, TEXT("Unregistered saveable %s reason: %s"), *Class->GetName(), *ReasonStr);
}

int32 USaveGameSubsystem::GetPlayerSaveIndex() const
{
	// TODO: get user index eg. from game instance
	return 0;
}

void USaveGameSubsystem::SaveTo(FGameInstanceRecord& SaveRecord)
{
	for (auto SaveableInfo : Saveables)
	{
		UClass* Class = SaveableInfo.Key;

		if (const auto Saveable = Cast<ISaveableInterface>(SaveableInfo.Value))
		{
			FSaveDataRecord Data;

			Saveable->Save(Data);
			SaveRecord.SaveRecordsByClass.Emplace(Class, Data);

			UE_LOG(LogTemp, Log, TEXT("Saved object %s"), *Class->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid saveable %s"), *Class->GetName());
		}
	}
}

void USaveGameSubsystem::LoadFrom(FGameInstanceRecord& SaveRecord)
{
	UE_LOG(LogTemp, Log, TEXT("Loading saveables"));

	for (auto SaveableInfo : Saveables)
	{
		if (!SaveableInfo.Value)
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to load object %p, null saveable"), SaveableInfo.Key);
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("Loading object %s"), *SaveableInfo.Value->GetName());

		FSaveDataRecord* Data = SaveRecord.SaveRecordsByClass.Find(SaveableInfo.Key);

		if (Data == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to load %s - null data"), *SaveableInfo.Key->GetName());
			continue;
		}

		if (const auto Saveable = Cast<ISaveableInterface>(SaveableInfo.Value))
		{
			Saveable->Load(*Data);
			UE_LOG(LogTemp, Log, TEXT("Loaded object %s"), *SaveableInfo.Key->GetName());
		}
	}
}

void USaveGameSubsystem::LoadGameAsync(FGameInstanceRecord GameRecord, FAsyncGameLoadedFromSlotDelegate LoadedDelegate)
{
	UE_LOG(LogTemp, Log, TEXT("LoadGameAsync"));
	LoadRootAsync(GameRecord, [this, GameRecord, LoadedDelegate]()
	{
		FGameInstanceRecord _ = GameRecord;

		LoadFrom(_);

		check(IsInGameThread());
		// ReSharper disable once CppExpressionWithoutSideEffects
		LoadedDelegate.ExecuteIfBound();
	});
}

void USaveGameSubsystem::LoadRootAsync(FGameInstanceRecord& SaveRecord, TFunction<void()> Callback)
{
	check(RootSaveable);

	//ensure root is not garbage collected during loading 
	RootSaveable->AddToRoot();

	const auto Saveable = Cast<ISaveableInterface>(RootSaveable);
	bIsRootLoadingFinished = false;

	// first load root saveable which reinitializes game
	Saveable->Load(SaveRecord.Root);

	// wait until saveable root is not fully loaded, only then we can proceed with loading other saveables
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, Callback]()
	{
		while (!bIsRootLoadingFinished)
		{
			if (!ensureMsgf(RootSaveable->IsValidLowLevel(), TEXT("Saveable root is destroyed")))
			{
				break;
			}

			if (ISaveableInterface::Execute_bIsLoadingCompleted(RootSaveable))
			{
				AsyncTask(ENamedThreads::GameThread, [this, Callback]
				{
					if (Callback)
					{
						Callback();
					}

					bIsRootLoadingFinished = true;
				});
			}

			FPlatformProcess::Sleep(0.1);
		}

		bIsRootLoadingFinished = true;
		RootSaveable->RemoveFromRoot();
	});
}
