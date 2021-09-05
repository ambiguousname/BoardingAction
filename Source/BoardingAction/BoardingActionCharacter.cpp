// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardingActionCharacter.h"
#include "BoardingActionProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ABoardingActionCharacter

ABoardingActionCharacter::ABoardingActionCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(RootComponent);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	previousGravity = FVector{0, 0, -9.8f};
}

void ABoardingActionCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	rotGravity = (-GetActorUpVector()).ToOrientationRotator();
	rotGravityPercent = 1;
	prevGravityPercent = 1;

	UWorld* world = GetWorld();
	worldPhysics = world->GetSubsystem<UPhysicsSubsystem>();

	// Make sure we can add a tick:
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		Mesh1P->SetHiddenInGame(false, true);
	}
}

void ABoardingActionCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	UCharacterMovementComponent* mover = FindComponentByClass<UCharacterMovementComponent>();
	mover->AddImpulse(worldPhysics->GetGravity(), true);

	// Primarily, this is about rotating the actor's down vector (and everything else) to match the new gravity vector.
	// So, when the gravity changes, look at how you can rotate the down vector to match the new gravity.
	
	if (previousGravity != worldPhysics->GetGravity()) {
		// Stuff for following the 180 degree rule. Not that we need it right now, because everything is actually working.
		// Even simpler solution for following the 180 degree rule than this. If the DotProduct of the vector representing
		// where the player is going to be rotated and the player's forward vector is < 0, multiply the vector by -actorForwardVector.
		/*FVector plane = FVector::CrossProduct(worldPhysics->GetGravity(), GetActorRightVector());
		plane.Normalize();
		if (FVector::DotProduct(plane, lookRot) < 0) {
			lookRot *= -plane;
		}*/
		rotGravity = worldPhysics->GetGravity().ToOrientationRotator() - previousGravity.ToOrientationRotator();
		//The transition should be gradual, so we increment in terms of the percentage of the rotation.
		rotGravityPercent = 0;
		prevGravityPercent = 0;
	}

	if (rotGravityPercent < 1) {
		rotGravityPercent += DeltaTime;
		if (rotGravityPercent > 1) {
			rotGravityPercent = 1;
		}
		UE_LOG(LogTemp, Warning, TEXT("%f"), rotGravityPercent);

		AddActorLocalRotation(rotGravity * rotGravityPercent - rotGravity * prevGravityPercent);

		prevGravityPercent = rotGravityPercent;
	} else if (rotGravityPercent >= 1 && prevGravityPercent >= 1){
		/*FRotator remainingRot = worldPhysics->GetGravity().ToOrientationRotator() - (-GetActorUpVector()).ToOrientationRotator();
		AddActorLocalRotation(remainingRot);
		prevGravityPercent = 0;*/
	}

	previousGravity = worldPhysics->GetGravity();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABoardingActionCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABoardingActionCharacter::OnFire);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &ABoardingActionCharacter::OnRightClick);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABoardingActionCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ABoardingActionCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABoardingActionCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ABoardingActionCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABoardingActionCharacter::LookUp);
}

void ABoardingActionCharacter::OnFire()
{
	// try and fire a projectile
	/*if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<ABoardingActionProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}*/
}

void ABoardingActionCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ABoardingActionCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ABoardingActionCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void ABoardingActionCharacter::OnRightClick() {
	if (worldPhysics->GetGravity().Z == -9.8f) {
		worldPhysics->SetGravity(0, 0, 9.8f);
	}
	else {
		worldPhysics->SetGravity(0, 0, -9.8f);
	}
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ABoardingActionCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ABoardingActionCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABoardingActionCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABoardingActionCharacter::Turn(float Val)
{
	// TODO: Make this instead a rotation around the player's up vector.
	FirstPersonCameraComponent->AddLocalRotation(FRotator{ 0, Val, 0 });
}

void ABoardingActionCharacter::LookUp(float Val)
{
	FirstPersonCameraComponent->AddLocalRotation(FRotator{-Val, 0, 0});
}

bool ABoardingActionCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABoardingActionCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ABoardingActionCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ABoardingActionCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}
