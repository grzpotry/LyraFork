// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveStatics.h"
#include "SaveableInterface.h"
#include "SaveGameSubsystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void USaveStatics::Save(UObject* Object, FSaveDataRecord& SaveRecord)
{
	SaveRecord.Blobs.Reset();

	FBlob MainBlob;
	SerializeObject(Object, MainBlob.Bytes);
	SaveRecord.Blobs.Add(MainBlob);
}

void USaveStatics::RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Interface, const UWorld* World)
{
	const UGameInstance* GameInstance = World->GetGameInstance();
	RegisterSingletonSaveable(Interface, GameInstance->GetSubsystem<USaveGameSubsystem>());
}

void USaveStatics::RegisterSingletonSaveable(TScriptInterface<ISaveableInterface> Interface,
                                             USaveGameSubsystem* SaveSystem)
{
	UObject* Saveable = Interface.GetObject();

	if (ensureMsgf(Saveable, TEXT("Tried to register invalid saveable")))
	{
		SaveSystem->RegisterSingletonSaveable(Saveable);
	}
}

void USaveStatics::SerializeObject(UObject* Object, TArray<uint8>& OutData)
{
	FMemoryWriter MemoryWriter(OutData, true);
	SerializeObject(Object, MemoryWriter);

	UE_LOG(LogTemp, Log, TEXT("Saved UObject %s"), *Object->GetName());
}

void USaveStatics::DeserializeObject(UObject* Object, TArray<uint8> OutData)
{
	FMemoryReader MemoryReader(OutData, true);
	SerializeObject(Object, MemoryReader);

	UE_LOG(LogTemp, Log, TEXT("Loaded UObject %s"), *Object->GetName());
}

void USaveStatics::SerializeObject(UObject* Object, FArchive& Archive)
{
	FObjectAndNameAsStringProxyArchive Ar(Archive, true);
	Ar.ArIsSaveGame = true;
	Object->Serialize(Ar);

	Archive.Close();
	Ar.Reset();
	Archive.Reset();
}
