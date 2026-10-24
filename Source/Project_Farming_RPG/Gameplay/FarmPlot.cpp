#include "FarmPlot.h"
#include "Time/DayManager.h"

#include "Kismet/GameplayStatics.h"

AFarmPlot::AFarmPlot()
{
    PrimaryActorTick.bCanEverTick = false;
}


void AFarmPlot::BeginPlay()
{
    Super::BeginPlay();
    OnFarmVisualStateChanged();

    DayManager = Cast<ADayManager>(
        UGameplayStatics::GetActorOfClass(
            this,
            ADayManager::StaticClass()
        )
    );

    if (!IsValid(DayManager))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s: DayManager not found!"),
            *GetName()
        );
        return;
    }

    DayManager->OnDayAdvanced.AddDynamic(this, &AFarmPlot::HandleDayAdvanced);
}


void AFarmPlot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(DayManager))
    {
        DayManager->OnDayAdvanced.RemoveDynamic(this, &AFarmPlot::HandleDayAdvanced); // Unsubscribe from the OnDayAdvanced event
    }

    Super::EndPlay(EndPlayReason);
}

void AFarmPlot::Interact_Implementation(AActor *Interactor)
{
    if (SoilState == EFarmSoilState::Untilled)
    {
        if (TryTillSoil())
        {
            UE_LOG(
                LogTemp,
                Display,
                TEXT("FarmPlot %s tilled by %s"),
                *GetName(),
                *GetNameSafe(Interactor));
        }
        return;
    }

    if (CropState == EFarmCropState::Empty)
    {
        if (TryPlantSeed())
        {
            UE_LOG(
                LogTemp,
                Display,
                TEXT("FarmPlot %s planted by %s"),
                *GetName(),
                *GetNameSafe(Interactor));
        }
        return;
    }

    if (CropState == EFarmCropState::Growing)
    {
        if (TryWaterCrop())
        {
            UE_LOG(
                LogTemp,
                Display,
                TEXT("FarmPlot %s watered by %s"),
                *GetName(),
                *GetNameSafe(Interactor));
        }
        return;
    }

    if (CropState == EFarmCropState::Harvestable)
    {
        if (TryHarvestCrop())
        {
            constexpr int32 HarvestQuantity = 1;

            UE_LOG(
                LogTemp,
                Display,
                TEXT("%s harvested %d crop(s) by %s"),
                *GetName(),
                HarvestQuantity,
                *GetNameSafe(Interactor));
        }
        return;
    }
}

FText AFarmPlot::GetInteractionText_Implementation() const
{
    if (SoilState == EFarmSoilState::Untilled)
    {
        return NSLOCTEXT(
            "Farming",
            "TillSoil",
            "Toprağı İşle");
    }

    if (CropState == EFarmCropState::Empty)
    {
        return NSLOCTEXT(
            "Farming",
            "PlantSeed",
            "Tohum Ek");
    }

    if (CropState == EFarmCropState::Growing)
    {
        if (!bIsWatered)
        {
            return NSLOCTEXT(
                "Farming",
                "WaterCrop",
                "Sula");
        }

        return NSLOCTEXT(
            "Farming",
            "CropWatered",
            "Sulandı");
    }

    return NSLOCTEXT(
        "Farming",
        "HarvestCrop",
        "Hasat Et");
}



bool AFarmPlot::CanInteract_Implementation() const
{
    if (SoilState == EFarmSoilState::Untilled)
        return CropState == EFarmCropState::Empty;

    if (SoilState != EFarmSoilState::Tilled)
        return false;

    switch (CropState)
    {
        case EFarmCropState::Empty:
            return true;

        case EFarmCropState::Growing:
            return !bIsWatered;

        case EFarmCropState::Harvestable:
            return true;

        default:
            return false;
    }
}




void AFarmPlot::HandleDayAdvanced(int32 NewDay)
{
    if (CropState != EFarmCropState::Growing)
        return;

    if (!bIsWatered)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("%s: Growth paused on day %d (dry)."),
            *GetName(),
            NewDay
        );
        return;
    }

    ++GrowthProgress;
    bIsWatered = false;

    if (GrowthProgress >= RequiredGrowthDays)
    {
        CropState = EFarmCropState::Harvestable;

        UE_LOG(
            LogTemp,
            Display,
            TEXT("%s: Crop is ready to harvest!"),
            *GetName()
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("%s: Growth progress %d/%d"),
            *GetName(),
            GrowthProgress,
            RequiredGrowthDays
        );
    }

    OnFarmVisualStateChanged();
}


bool AFarmPlot::TryTillSoil()
{
    if (SoilState != EFarmSoilState::Untilled)
        return false;
    if (CropState != EFarmCropState::Empty)
        return false;

    SoilState = EFarmSoilState::Tilled;

    OnFarmVisualStateChanged();
    return true;
}

bool AFarmPlot::TryPlantSeed()
{
    if (SoilState != EFarmSoilState::Tilled)
        return false;
    if (CropState != EFarmCropState::Empty)
        return false;

    CropState = EFarmCropState::Growing;
    GrowthProgress = 0;
    bIsWatered = false;

    OnFarmVisualStateChanged();
    return true;
}

bool AFarmPlot::TryWaterCrop()
{
    if (SoilState != EFarmSoilState::Tilled)
        return false;
    if (CropState != EFarmCropState::Growing)
        return false;
    if (bIsWatered)
        return false;

    bIsWatered = true;
    OnFarmVisualStateChanged();
    return true;
}

bool AFarmPlot::TryHarvestCrop()
{
    if (SoilState != EFarmSoilState::Tilled)
        return false;
    if (CropState != EFarmCropState::Harvestable)
        return false;

    CropState = EFarmCropState::Empty;
    GrowthProgress = 0;
    bIsWatered = false;

    OnFarmVisualStateChanged();
    return true;
}
