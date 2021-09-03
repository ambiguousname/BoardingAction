// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class BOARDINGACTION_API UPhysicsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	void SetGravity(float x, float y, float z);
	FVector GetGravity();
protected:
	FVector gravity;
};
