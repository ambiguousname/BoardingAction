// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityController.h"

// Sets default values for this component's properties
UGravityController::UGravityController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGravityController::BeginPlay()
{
	Super::BeginPlay();
	parent = GetOwner();
	UWorld* world = GetWorld();
	worldPhysics = world->GetSubsystem<UPhysicsSubsystem>();
	mesh = parent->FindComponentByClass<UPrimitiveComponent>();

	// ...
	
}


// Called every frame
void UGravityController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	mesh->AddImpulse(worldPhysics->GetGravity(), NAME_None, true);

	// ...
}

