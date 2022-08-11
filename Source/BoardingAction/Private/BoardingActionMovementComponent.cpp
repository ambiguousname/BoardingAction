// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardingActionMovementComponent.h"
#include "GravityController.h"

#define LOG_FAST(text) { UE_LOG(LogTemp, Warning, TEXT(text))}

UBoardingActionMovementComponent::UBoardingActionMovementComponent(const FObjectInitializer& ObjectInitializer) {
	return;
}

void UBoardingActionMovementComponent::BeginPlay() {
	//UE_LOG(LogTemp, Warning, TEXT("TEST"));
}

void UBoardingActionMovementComponent::PostLoad() {
	Super::PostLoad();
	CharacterOwner = Cast<ACharacter>(PawnOwner);
	UE_LOG(LogTemp, Warning, TEXT("%i"), CharacterOwner->GetLocalRole());
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

bool UBoardingActionMovementComponent::IsMovingOnGround() {
	UE_LOG(LogTemp, Warning, TEXT("Moving"));
	return true;
}

bool UBoardingActionMovementComponent::IsFalling() {
	UE_LOG(LogTemp, Warning, TEXT("Falling"));
	return false;
}

void UBoardingActionMovementComponent::SetDefaultMovementMode() {
	UE_LOG(LogTemp, Warning, TEXT("Defaulting"));
	SetMovementMode(DefaultLandMovementMode);
}

void UBoardingActionMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {
	UE_LOG(LogTemp, Warning, TEXT("Switched"));
}

void UBoardingActionMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	//UE_LOG(LogTemp, Warning, TEXT("%i"), (MovementMode.GetValue()));
	//Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	return;
}



void UBoardingActionMovementComponent::SetPostLandedPhysics(const FHitResult& Hit) {
	//UE_LOG(LogTemp, Warning, TEXT("LANDED"));
	Super::SetPostLandedPhysics(Hit);
}

void UBoardingActionMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) {
	//UE_LOG(LogTemp, Warning, TEXT("PROCESS"))
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
	return r;
}

void UBoardingActionMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult) const {
	Super::ComputeFloorDist(CapsuleLocation, LineDistance, SweepDistance, OutFloorResult, SweepRadius, DownwardSweepResult);
}