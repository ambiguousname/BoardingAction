// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsSubsystem.h"
#include "GravityController.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BOARDINGACTION_API UGravityController : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGravityController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	AActor* parent;
	UPhysicsSubsystem *worldPhysics;
	UPrimitiveComponent* mesh;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
