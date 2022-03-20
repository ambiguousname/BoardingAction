// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	virtual bool IsWalkable(const FHitResult& Hit) const;

	virtual void SetPostLandedPhysics(const FHitResult& Hit);

	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations);

	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const;
};
