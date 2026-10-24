#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDayAdvanced,
	int32,
	NewDay
);

UCLASS()
class PROJECT_FARMING_RPG_API ADayManager : public AActor
{
	GENERATED_BODY()

public:
	ADayManager();

	UFUNCTION(BlueprintCallable, Category = "Time")
	void AdvanceDay();

	UPROPERTY(BlueprintAssignable, Category = "Time|Events")
	FOnDayAdvanced OnDayAdvanced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 1;
};
