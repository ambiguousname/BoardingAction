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

FRotator UPhysicsSubsystem::GetRotatorFromGravity(FVector grav) {
	// Primarily, this is about rotating the global down down vector (and everything else) to match the new gravity vector.
	// So, when the gravity changes, look at how you can set the actor's rotation to match the new gravity.

	FVector normalGrav = grav.GetSafeNormal();

	// We want to use the universal down of gravity to figure out how to set the player's rotation. Otherwise, we'll be setting global rotation
	// based on information calculated from local rotation:
	FVector downGravCross = FVector::CrossProduct(FVector::DownVector, normalGrav);

	// If the cross product is zero, this means that normalGrav either goes directly up (globally), directly down (globally), or is the 0 vector.
	if (downGravCross.IsNearlyZero()) {
		// If the normalGrav is the zero vector, don't make any rotations.
		// Otherwise, just pick the forward vector as the angle we want to rotate along.
		if (!normalGrav.IsNearlyZero()) {
			downGravCross = FVector::ForwardVector;
		}
	}

	// In case downGravCross happens to be the zero vector and gets changed:
	FVector actualCross = FVector::CrossProduct(FVector::DownVector, normalGrav);

	//UE_LOG(LogTemp, Warning, TEXT("Vector we're rotating along: %s Cross product: %s Dot Product: %f"), *downGravCross.ToString(), *actualCross.ToString(), FVector::DotProduct(FVector::DownVector, normalGrav));

	float crossAngle = FMath::Atan2(actualCross.Size(), FVector::DotProduct(FVector::DownVector, normalGrav)) * 180 / PI; // We need to convert to degrees.

	// Simplified from the nightmare that was before.
	// We create an axis angle representation of our rotation, use the KismetMathLibrary to do the hard job of converting (https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Conversion_formulae_between_formalisms)
	// And then we apply that rotation to our actor.
	FVector axisAngle = downGravCross.GetSafeNormal();

	FRotator newRot = UKismetMathLibrary::RotatorFromAxisAndAngle(axisAngle, crossAngle);

	return newRot;
}