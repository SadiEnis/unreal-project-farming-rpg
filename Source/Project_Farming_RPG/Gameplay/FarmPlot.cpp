#include "FarmPlot.h"

AFarmPlot::AFarmPlot()
{
    PrimaryActorTick.bCanEverTick = false;
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

bool AFarmPlot::TryTillSoil()
{
    if (SoilState != EFarmSoilState::Untilled)
        return false;
    if (CropState != EFarmCropState::Empty)
        return false;

    SoilState = EFarmSoilState::Tilled;
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

    return true;
}

bool AFarmPlot::TryWaterCrop()
{
    if (SoilState != EFarmSoilState::Tilled)
    {
        return false;
    }

    if (CropState != EFarmCropState::Growing)
    {
        return false;
    }

    if (bIsWatered)
    {
        return false;
    }

    bIsWatered = true;
    return true;
}
