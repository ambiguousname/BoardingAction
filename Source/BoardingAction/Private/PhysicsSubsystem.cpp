// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsSubsystem.h"

void UPhysicsSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	gravity = FVector{0, 0, -9.8f};
}

void UPhysicsSubsystem::SetGravity(float x, float y, float z) {
	gravity = FVector{x, y, z};
}

FVector UPhysicsSubsystem::GetGravity() {
	return gravity;
}