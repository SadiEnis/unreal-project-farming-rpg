#include "FarmPlot.h"

AFarmPlot::AFarmPlot()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFarmPlot::Interact_Implementation(AActor* Interactor)
{
	if (TryTillSoil())
	{
        UE_LOG(
            LogTemp,
            Display,
            TEXT("FarmPlot %s tilled by %s"),
            *GetName(),
            *GetNameSafe(Interactor)
        );
	}
}


FText AFarmPlot::GetInteractionText_Implementation() const
{
    if (SoilState == EFarmSoilState::Untilled)
    {
        return NSLOCTEXT(
            "Farming",
            "TillSoil",
            "Toprağı İşle"
        );
    }

    return NSLOCTEXT(
        "Farming",
        "SoilReady",
        "Toprak Hazır"
    );
}


bool AFarmPlot::TryTillSoil()
{
	if (SoilState != EFarmSoilState::Untilled) return false;
	if (CropState != EFarmCropState::Empty) return false;

	SoilState = EFarmSoilState::Tilled;
	return true;
}
