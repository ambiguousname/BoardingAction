// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnGravityController.h"

// Sets default values for this component's properties
UPawnGravityController::UPawnGravityController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPawnGravityController::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPawnGravityController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	/*FVector actorUp = parent->GetActorUpVector();
	FRotator actorRot = parent->GetActorRotation();
	FRotator newRot = FMath::Lerp(actorRot, actorUp.ToOrientationRotator() - worldPhysics->GetGravity().ToOrientationRotator(), 0.01f);
	parent->AddActorLocalRotation(newRot);*/
	// ...
}

