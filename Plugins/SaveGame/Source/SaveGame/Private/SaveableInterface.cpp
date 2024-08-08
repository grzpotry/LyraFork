// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveableInterface.h"
#include "SaveStatics.h"

// Add default functionality here for any ISaveableInterface functions that are not pure virtual.

void ISaveableInterface::Save(FSaveDataRecord& SaveRecord)
{
	if (UObject* Obj = Cast<UObject>(this))
	{
		FBlob MainBlob;
		USaveStatics::SerializeObject(Obj, MainBlob.Bytes);

		SaveRecord.Blobs.Add(MainBlob);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Saving not implemented"));
	}
}

void ISaveableInterface::Load(FSaveDataRecord Data)
{
	if (UObject* Obj = Cast<UObject>(this))
	{
		USaveStatics::DeserializeObject(Obj, Data.Blobs[0].Bytes);
		Execute_OnLoad(Obj);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Loading not implemented"));
	}
}
