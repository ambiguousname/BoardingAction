// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardingActionMovementComponent.h"

UBoardingActionMovementComponent::UBoardingActionMovementComponent(const FObjectInitializer& ObjectInitializer) {
	return;
}

void UBoardingActionMovementComponent::BeginPlay() {
	UE_LOG(LogTemp, Warning, TEXT("TEST"));
}