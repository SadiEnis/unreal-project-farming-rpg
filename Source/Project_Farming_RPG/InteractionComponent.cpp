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

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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

	if (!Target->GetClass()->ImplementsInterface(
			UInteractable::StaticClass()))
	{
		return;
	}

	IInteractable::Execute_Interact(Target, GetOwner());
	OnInteractionExecuted.Broadcast(Target);
}

void UInteractionComponent::UpdateTarget()
{
	AActor *OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
		return;

	APawn *OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerPawn)
		return;

	APlayerController *PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!IsValid(PlayerController))
		return;

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor *> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);

	TArray<AActor *> OverlappingActors;

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		OwnerActor->GetActorLocation(),
		DetectionRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlappingActors);

	int32 ValidCandidateCount = 0;
	AActor *BestCandidate = nullptr;
	float BestScore = -1.0f;

	for (AActor *Candidate : OverlappingActors)
	{
		if (!IsValid(Candidate))
			continue;

		const bool bIsInteractable =
			Candidate->GetClass()->ImplementsInterface(UInteractable::StaticClass());

		if (!bIsInteractable)
			continue;

		const FVector ToTarget =
			Candidate->GetActorLocation() - ViewLocation;

		const float ViewScore = FVector::DotProduct(
			ViewRotation.Vector(),
			ToTarget.GetSafeNormal());

		// Ignore targets behind or far outside our view.
		if (ViewScore < 0.15f)
			continue;

		if (!HasClearLineOfSight(Candidate))
			continue;

		const float Distance = FVector::Dist(
			OwnerActor->GetActorLocation(),
			Candidate->GetActorLocation());

		const float DistanceScore = 1.0f - FMath::Clamp(
											   Distance / DetectionRadius,
											   0.0f,
											   1.0f);

		constexpr float DistanceWeight = 0.4f;
		constexpr float ViewWeight = 0.6f;

		const float Score =
			DistanceScore * DistanceWeight +
			ViewScore * ViewWeight;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Candidate: %s | Score: %.2f"),
			*GetNameSafe(Candidate),
			Score);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestCandidate = Candidate;
		}
	}

	FText NewInteractionText = FText::GetEmpty();

	if (IsValid(BestCandidate))
	{
		NewInteractionText =
			IInteractable::Execute_GetInteractionText(
				BestCandidate);
	}

	if (!CachedInteractionText.EqualTo(NewInteractionText))
	{
		CachedInteractionText = NewInteractionText;

		OnInteractionTextChanged.Broadcast(
			CachedInteractionText);
	}

	if (CurrentTarget.Get() != BestCandidate)
	{
		CurrentTarget = BestCandidate;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Target Changed: %s"),
			*GetNameSafe(BestCandidate));

		OnTargetChanged.Broadcast(BestCandidate);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BEST CANDIDATE: %s | Score: %.2f"),
		*GetNameSafe(BestCandidate),
		BestScore);
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