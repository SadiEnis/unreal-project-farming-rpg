#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TimerManager.h"

#include "InteractionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInteractionTargetChanged,
    AActor*,
    NewTarget
); // Called when the interaction target changes


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInteractionExecuted,
    AActor*,
    InteractedActor
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInteractionTextChanged,
    FText,
    NewText
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInteractionAvailabilityChanged,
    bool,
    bCanInteract
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_FARMING_RPG_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float DetectionRadius = 250.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY(EditAnywhere, Category = "Interaction|Detection", meta = (ClampMin = "0.002"))
	float SearchInterval = 0.1f;

	FTimerHandle TargetSearchTimer;
	FText CachedInteractionText;
	bool bCachedCanInteract = false;
	
	bool HasClearLineOfSight(AActor* Candidate) const;
	AActor* FindBestInteractionTarget() const;
	void RefreshTargetState(AActor* NewTarget);
	void UpdateTarget();

public:	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionTargetChanged OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionExecuted OnInteractionExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionTextChanged OnInteractionTextChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionAvailabilityChanged OnInteractionAvailabilityChanged;


	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
};
