// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "BoardingActionMovementComponent.generated.h"

/**
 * This will be used specifically for characters that need to wall walk.
 */
UCLASS()
class BOARDINGACTION_API UBoardingActionMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UBoardingActionMovementComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay();

	virtual void PostLoad();

	virtual bool IsWalkable(const FHitResult& Hit) const;

	virtual bool IsMovingOnGround();

	virtual bool IsFalling();

	virtual void SetDefaultMovementMode();

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	virtual void SetPostLandedPhysics(const FHitResult& Hit);

	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations);

	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const;

	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult) const;

private:
	ACharacter* CharacterOwner;
};
