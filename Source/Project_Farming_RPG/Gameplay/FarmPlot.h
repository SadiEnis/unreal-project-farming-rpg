#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "../Interactable.h"

#include "FarmPlot.generated.h"

class ADayManager;

UENUM(BlueprintType)
enum class EFarmSoilState : uint8
{
	Untilled,
	Tilled
};

UENUM(BlueprintType)
enum class EFarmCropState : uint8
{
	Empty,
	Growing,
	Harvestable
};

UCLASS()
class PROJECT_FARMING_RPG_API AFarmPlot : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AFarmPlot();

	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual FText GetInteractionText_Implementation() const override;

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void HandleDayAdvanced(int32 NewDay);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming|Soil")
    EFarmSoilState SoilState = EFarmSoilState::Untilled;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming|Crop")
    EFarmCropState CropState = EFarmCropState::Empty;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming|Soil")
    bool bIsWatered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming|Growth")
    int32 GrowthProgress = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Farming|Growth", meta = (ClampMin = "1"))
    int32 RequiredGrowthDays = 3;

private:
	bool TryTillSoil();
    bool TryPlantSeed(); // private because we don't want to allow planting seeds without checking if the soil is tilled and empty
    bool TryWaterCrop();
    bool TryHarvestCrop();

    UPROPERTY(Transient)
    TObjectPtr<ADayManager> DayManager = nullptr;
};
