#include "InteractionComponent.h"
#include "Interactable.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld *World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TargetSearchTimer,
			this,
			&UInteractionComponent::UpdateTarget,
			SearchInterval,
			true);
	}
	UpdateTarget();
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld *World = GetWorld())
		World->GetTimerManager().ClearTimer(TargetSearchTimer);

	CurrentTarget.Reset();
	Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::TryInteract()
{
	UpdateTarget();

	AActor *Target = CurrentTarget.Get();

	if (!IsValid(Target))
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("No valid interaction target."));
		return;
	}

	if (!Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
		return;
	if (!IInteractable::Execute_CanInteract(Target))
		return;

	IInteractable::Execute_Interact(Target, GetOwner());
	OnInteractionExecuted.Broadcast(Target);
}


void UInteractionComponent::UpdateTarget()
{
    AActor* BestCandidate =
        FindBestInteractionTarget();

    RefreshTargetState(BestCandidate);
}

AActor* UInteractionComponent::FindBestInteractionTarget() const
{
    AActor* OwnerActor = GetOwner();

    if (!IsValid(OwnerActor))
        return nullptr;

    if (DetectionRadius <= KINDA_SMALL_NUMBER)
        return nullptr;

    APawn* OwnerPawn = Cast<APawn>(OwnerActor);
    if (!OwnerPawn)
        return nullptr;

    APlayerController* PlayerController =
        Cast<APlayerController>(OwnerPawn->GetController());

    if (!IsValid(PlayerController))
        return nullptr;

    FVector ViewLocation;
    FRotator ViewRotation;

    PlayerController->GetPlayerViewPoint(
        ViewLocation,
        ViewRotation
    );

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

    ObjectTypes.Add(
        UEngineTypes::ConvertToObjectType(ECC_WorldStatic)
    );

    ObjectTypes.Add(
        UEngineTypes::ConvertToObjectType(ECC_WorldDynamic)
    );

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerActor);

    TArray<AActor*> OverlappingActors;

    UKismetSystemLibrary::SphereOverlapActors(
        this,
        OwnerActor->GetActorLocation(),
        DetectionRadius,
        ObjectTypes,
        nullptr,
        ActorsToIgnore,
        OverlappingActors
    );

    AActor* BestCandidate = nullptr;
    float BestScore = -1.0f;

    for (AActor* Candidate : OverlappingActors)
    {
        if (!IsValid(Candidate))
            continue;

        if (!Candidate->GetClass()->ImplementsInterface(
            UInteractable::StaticClass()))
        {
            continue;
        }

        const FVector ToTarget =
            Candidate->GetActorLocation() - ViewLocation;

        const float ViewScore = FVector::DotProduct(
            ViewRotation.Vector(),
            ToTarget.GetSafeNormal()
        );

        // Ignore targets outside the allowed view direction.
        if (ViewScore < 0.15f)
            continue;

        if (!HasClearLineOfSight(Candidate))
            continue;

        const float Distance = FVector::Dist(
            OwnerActor->GetActorLocation(),
            Candidate->GetActorLocation()
        );

        const float DistanceScore = 1.0f - FMath::Clamp(
            Distance / DetectionRadius,
            0.0f,
            1.0f
        );

        constexpr float DistanceWeight = 0.4f;
        constexpr float ViewWeight = 0.6f;

        const float Score =
            DistanceScore * DistanceWeight +
            ViewScore * ViewWeight;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestCandidate = Candidate;
        }
    }

    return BestCandidate;
}


void UInteractionComponent::RefreshTargetState(
    AActor* NewTarget)
{
    if (!IsValid(NewTarget) ||
        !NewTarget->GetClass()->ImplementsInterface(
            UInteractable::StaticClass()))
    {
        NewTarget = nullptr;
    }

    const bool bTargetChanged =
        CurrentTarget.Get() != NewTarget;

    if (bTargetChanged)
    {
        CurrentTarget = NewTarget;

        UE_LOG(
            LogTemp,
            Log,
            TEXT("Target Changed: %s"),
            *GetNameSafe(NewTarget)
        );

        OnTargetChanged.Broadcast(NewTarget);
    }

    FText NewInteractionText = FText::GetEmpty();

    if (IsValid(NewTarget))
    {
        NewInteractionText =
            IInteractable::Execute_GetInteractionText(
                NewTarget
            );
    }

    if (bTargetChanged ||
        !CachedInteractionText.EqualTo(NewInteractionText))
    {
        CachedInteractionText = NewInteractionText;

        OnInteractionTextChanged.Broadcast(
            CachedInteractionText
        );
    }

    const bool bCanInteract =
        IsValid(NewTarget) &&
        IInteractable::Execute_CanInteract(NewTarget);

    if (bTargetChanged ||
        bCanInteract != bCachedCanInteract)
    {
        bCachedCanInteract = bCanInteract;

        OnInteractionAvailabilityChanged.Broadcast(
            bCanInteract
        );
    }
}


bool UInteractionComponent::HasClearLineOfSight(AActor *Candidate) const
{
	APawn *OwnerPawn = Cast<APawn>(GetOwner());
	UWorld *World = GetWorld();
	if (!IsValid(OwnerPawn) || !IsValid(World) || !IsValid(Candidate))
		return false;

	FVector TargetOrigin;
	FVector TargetExtent;

	Candidate->GetActorBounds(false, TargetOrigin, TargetExtent);

	const FVector TraceStart = OwnerPawn->GetActorLocation() + FVector(0.0f, 0.0f, 60.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	FHitResult HitResult;

	const bool bBlockingHit =
		World->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TargetOrigin,
			ECC_Visibility,
			QueryParams);

	return !bBlockingHit || HitResult.GetActor() == Candidate; // If we hit something, it must be the candidate to have a clear line of sight.
}