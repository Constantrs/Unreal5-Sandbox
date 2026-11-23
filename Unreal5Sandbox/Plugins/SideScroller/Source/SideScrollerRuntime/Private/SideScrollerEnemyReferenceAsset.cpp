// Fill out your copyright notice in the Description page of Project Settings.


#include "SideScrollerEnemyReferenceAsset.h"

bool USideScrollerEnemyReferenceAsset::ValidateCatalog(FString& OutError) const
{
	OutError.Reset();
	TSet<int32> Ids;
	for (const FSideScrollerEnemyEntry& Entry : Enemies)
	{
		if (Ids.Contains(Entry.Id) || !Entry.MeshAssetId.IsValid() || Entry.EnemyClass.IsNull())
		{
			OutError = FString::Printf(TEXT("Enemy Id=%d: duplicate Id, invalid MeshAssetId or empty EnemyClass."), Entry.Id);
			return false;
		}
		Ids.Add(Entry.Id);
	}
	return true;
}

const FSideScrollerEnemyEntry* USideScrollerEnemyReferenceAsset::FindEnemy(int32 Id) const
{
	return Enemies.FindByPredicate([Id](const FSideScrollerEnemyEntry& Entry) { return Entry.Id == Id; });
}