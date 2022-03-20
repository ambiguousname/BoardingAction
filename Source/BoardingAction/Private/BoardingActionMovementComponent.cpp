// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardingActionMovementComponent.h"
#include "GravityController.h"

UBoardingActionMovementComponent::UBoardingActionMovementComponent(const FObjectInitializer& ObjectInitializer) {
	return;
}

void UBoardingActionMovementComponent::BeginPlay() {
	UE_LOG(LogTemp, Warning, TEXT("TEST"));
}

bool UBoardingActionMovementComponent::IsWalkable(const FHitResult& Hit) const {
	float z = Super::GetWalkableFloorZ();
	//UE_LOG(LogTemp, Warning, TEXT("%f %f"), Hit.ImpactNormal.Z, z);

	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		z = SlopeOverride.ModifyWalkableFloorZ(z);
		//UE_LOG(LogTemp, Warning, TEXT("%f"), z);
	}
	return true;
}

void UBoardingActionMovementComponent::SetPostLandedPhysics(const FHitResult& Hit) {
	UE_LOG(LogTemp, Warning, TEXT("LANDED"));
	Super::SetPostLandedPhysics(Hit);
}

void UBoardingActionMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) {
	UE_LOG(LogTemp, Warning, TEXT("PROCESS"))
	return Super::ProcessLanded(Hit, remainingTime, Iterations);
}


// Based on CharacterMovementComponent.cpp
bool UBoardingActionMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const {
	UWorld* world = GetWorld();
	UPhysicsSubsystem* worldPhysics = world->GetSubsystem<UPhysicsSubsystem>();

	FVector gravity = worldPhysics->GetGravity();
	float aligned = 1 - FVector::DotProduct(gravity, Hit.Normal);

	FHitResult newHit = FHitResult(Hit);
	newHit.Normal.Z = aligned;
	bool r = Super::IsValidLandingSpot(CapsuleLocation, newHit);
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), (newHit.Normal.Z < KINDA_SMALL_NUMBER ? TEXT("TRUE") : TEXT("FALSE")), (r ? TEXT("TRUE") : TEXT("FALSE")))
	return true;
}