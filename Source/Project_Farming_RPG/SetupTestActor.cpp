// Fill out your copyright notice in the Description page of Project Settings.


#include "SetupTestActor.h"

// Sets default values
ASetupTestActor::ASetupTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASetupTestActor::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("SetupTestActor BeginPlay() called! Test Value: %d"), TestValue);
}

// Called every frame
void ASetupTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

