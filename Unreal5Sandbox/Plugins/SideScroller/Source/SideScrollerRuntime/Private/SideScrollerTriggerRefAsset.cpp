#include "SideScrollerTriggerRefAsset.h"

bool USideScrollerTriggerRefAsset::ValidateCatalog(FString& OutError) const
{
	OutError.Reset();
	TSet<int32> Ids;
	for (const FSideScrollerTriggerEntry& Entry : Triggers)
	{
		if (Ids.Contains(Entry.Id) || Entry.TriggerClass.IsNull())
		{
			OutError = FString::Printf(TEXT("Trigger Id=%d: duplicate Id or empty TriggerClass."), Entry.Id);
			return false;
		}
		Ids.Add(Entry.Id);
	}
	return true;
}

const FSideScrollerTriggerEntry* USideScrollerTriggerRefAsset::FindTrigger(int32 Id) const
{
	return Triggers.FindByPredicate([Id](const FSideScrollerTriggerEntry& Entry) { return Entry.Id == Id; });
}
