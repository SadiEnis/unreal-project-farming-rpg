#include "DayManager.h"

ADayManager::ADayManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADayManager::AdvanceDay()
{
	++CurrentDay;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("New day started: %d"),
		CurrentDay
	);

	OnDayAdvanced.Broadcast(CurrentDay);
}
