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
	BaseTurnRate = 1;
	BaseLookUpRate = 1;

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
	oldRotation = GetActorRotation();

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
	FVector gravVector = worldPhysics->GetGravity();
	UCharacterMovementComponent* mover = FindComponentByClass<UCharacterMovementComponent>();
	mover->AddImpulse(gravVector, true);

	// Primarily, this is about rotating the actor's down vector (and everything else) to match the new gravity vector.
	// So, when the gravity changes, look at how you can rotate the down vector to match the new gravity.
	
	// Get the cross product between the actor's current down vector and the current gravity vector.
	// Normalize the result of that cross product.
	// You now have a plane whose normal vector you can now rotate your down vector around towards the new gravity vector.
	// If the cross product results in the zero vector, you can rotate along any axis where the components for the vector are zero.
	// To get the angle to rotate, use these formulas:
	// cos(theta) * |A| * |B| = dot(A, B).
	// sin(theta) * |A| * |B| = |cross(A, B)|. (Note to self: It's only just the cross product if you're working in 2D).
	// and we want to get theta in terms of atan2 (since atan2 is in terms of the angle from the positive x axis).
	// sin(theta)/cos(theta) = cross(A, B)/dot(A, B).
	// tan(theta) = cross(A, B)/dot(A, B).
	// theta = atan2(cross(A, B), dot(A, B)).
	// Note, this solution still requires me to use ToOrientationRotator(), which I want to avoid. So instead...

	// Let's just create a rotation matrix.
	// As this slowly unravels, my linear algebra knowledge is coming back to me. That stack overflow solution was actually terrible,
	// so instead... it's time to actually research rotation matrices on Wikipedia: https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions
	// Step 1: Get the cross product of 
	// Step 3: Use these formulas: https://en.wikipedia.org/wiki/Euler_angles#Tait%E2%80%93Bryan_angles_2 to calculate the pitch, yaw, and roll.
	// Make sure to use atan2 instead of inverse sin or cosine.
	// Remember, rotation on the z-axis (psi) is yaw, y-axis (theta) is pitch, x-axis (phi) is roll.
	// All of these equations assume that the magnitude of each of these vectors is 1, so MAKE SURE THAT THE VECTORS ARE NORMALIZED.
	// Using x, y, and z as the vectors representing our forward, right, and up vectors respectively, we can use this diagram (https://en.wikipedia.org/wiki/Euler_angles#/media/File:Projections_of_Tait-Bryan_angles.svg)
	// For calculating in terms of atan2.
	// We can also assume that we'll first be rotating by the z-axis, then the y-axis, then the x-axis (this assumes intrinsic rotation, so apply these to the local axes after each rotation).
	// sin(theta) = -x_3 (since theta is negative).
	// cos(theta) = sqrt(1 - x_3^2) (From the pythagorean theorem)
	// theta = atan2(-x_3, sqrt(1 - x_3^2))
	// psi = atan2(x_2, x_1). You can see the diagram above for a visual representation of this.
	// phi = atan2(y_3, z_3). This is because y and z are perpendicular, so the opposite of the triangle formed by z is the adjacent for the triangle formed by y.
	// I hope this works. If not, more complicated equations are to come!
	// If you want to see how you come to this by yourself, go into a 3d modelling program (like blender), and make two object axes objects. Try rotating 
	// one around the local axes by z first, then y, then x. Try rotating all axes but one on 0 or 90 degrees, and see what triangles form as you rotate each angle.
	// I am stupid, there's a FORMULA FOR TELLING YOU HOW TO GET ARCTAN FROM ZYX. Good to know that I got *nearly* everything right (I messed up in calculating phi).
	
	if (previousGravity != gravVector) {
		// Stuff for following the 180 degree rule. Not that we need it right now, because everything is actually working.
		// Even simpler solution for following the 180 degree rule than this. If the DotProduct of the vector representing
		// where the player is going to be rotated and the player's forward vector is < 0, multiply the vector by -actorForwardVector.
		/*FVector plane = FVector::CrossProduct(worldPhysics->GetGravity(), GetActorRightVector());
		plane.Normalize();
		if (FVector::DotProduct(plane, lookRot) < 0) {
			lookRot *= -plane;
		}*/
		// Get the current vectors:
		FVector upVector = GetActorUpVector();
		FVector rightVector = GetActorRightVector();
		FVector forwardVector = GetActorForwardVector();

		FVector normalGrav = gravVector.GetSafeNormal();

		// We're going to use the cross product method I detailed above to get the new vector positions,
		// since rotation matrices are a nightmare, and StackOverflow was not particularly helpful.
		// My plan for making this better involves improvements elsewhere. I might come back to this later.
		FVector downGravCross = FVector::CrossProduct(-upVector, normalGrav);

		UE_LOG(LogTemp, Warning, TEXT("Current down: %s Current grav: %s"), *(-upVector).ToString(), *normalGrav.ToString());

		if (downGravCross.IsNearlyZero()) {
			for (int i = 0; i < 3; i++) {
				if (normalGrav[i] - upVector[i] == 0) {
					downGravCross = FVector::ZeroVector;
					downGravCross[i] = 1;
					break;
				}
			}
		}

		// TODO: This could probably be more efficient if I used extrinsic rotations instead of intrinsic ones. But why not try this first?

		// In case downGravCross happens to be the zero vector and gets changed:
		FVector actualCross = FVector::CrossProduct(-upVector, normalGrav);

		UE_LOG(LogTemp, Warning, TEXT("Vector we're rotating along: %s Cross product: %s"), *downGravCross.ToString(), *actualCross.ToString());

		float crossAngle = FMath::Atan2(actualCross.Size(), FVector::DotProduct(-upVector, normalGrav)) * 180 / PI; // We need to convert to degrees.

		UE_LOG(LogTemp, Warning, TEXT("Cross Angle: %f"), crossAngle);

		// Now we get the new vectors:
		FVector newUp = upVector.RotateAngleAxis(crossAngle, downGravCross); //z
		FVector newRight = rightVector.RotateAngleAxis(crossAngle, downGravCross); //y
		FVector newForward = forwardVector.RotateAngleAxis(crossAngle, downGravCross); //x

		UE_LOG(LogTemp, Warning, TEXT("Old Up: %s New Up: %s"), *upVector.ToString(), *newUp.ToString());

		// And now we calculate each of the rotations:
		float pitch = FMath::Atan2(-newForward.Z, FMath::Sqrt(1 - FMath::Pow(newForward.Z, 2))) * 180/PI;

		float yaw = FMath::Atan2(newForward.Y, newForward.X) * 180/PI;

		float roll = FMath::Atan2(newRight.Z, newUp.Z) * 180/PI;

		FRotator newRotation = FRotator{ pitch, yaw, roll };

		// I hope this works.
		// YESSSSSSSS.
		// TODO: Make sure this works when you're changing gravity rapidly (maybe by getting another version of newRotation once the gradual rotation is complete, and setting it then?)
		// (or alternatively, the cross product solution might not work with rapidly changing stuff, so maybe trying to implement the transformation matrix?)
		// I think I am going to have to implement the transformation matrix next, since gravity doesn't work just right in other directions with the cross product rotation involved.
		// More TODO: Clean the code, make sure this uses the camera's forward (somehow?), add the 180 degree rule(?).
		// The problem with doing this gradually is that we always lose a tiny bit of accuracy.
		// So knowing the exact rotation we need is useful.
		rotGravity = newRotation;
		oldRotation = GetActorRotation();

		UE_LOG(LogTemp, Warning, TEXT("New rot: %s Old rot: %s"), *newRotation.ToString(), *oldRotation.ToString());

		//The transition should be gradual, so we increment in terms of the percentage of the rotation.
		rotGravityPercent = 0;
	}

	if (rotGravityPercent < 1) {
		rotGravityPercent += DeltaTime * GravityRotationRate;
		if (rotGravityPercent > 1) {
			rotGravityPercent = 1;
		}

		// We gradually transition from the oldRotation to the new one. Since these are all in global space, we can't just set the
		// new rotation, so we have to transition away from the oldRotation (with 1 - rotGravityPercent).
		SetActorRotation((1 - rotGravityPercent) * oldRotation + rotGravity * rotGravityPercent);
	}

	previousGravity = gravVector;
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
		AddMovementInput(FirstPersonCameraComponent->GetForwardVector(), Value);
	}
}

void ABoardingActionCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(FirstPersonCameraComponent->GetRightVector(), Value);
	}
}

void ABoardingActionCharacter::Turn(float Val)
{
	if (Val != 0.0f) {
		FirstPersonCameraComponent->AddLocalRotation(FRotator{ 0, Val * BaseTurnRate, 0 });
		FRotator cameraRot = FirstPersonCameraComponent->GetRelativeRotation();
		// Because we're adding to local rotation, the camera can get weird about how we rotate. So we make sure to set the roll value to 0.
		FirstPersonCameraComponent->SetRelativeRotation(FRotator{ cameraRot.Pitch, cameraRot.Yaw, 0 });
	}
}

void ABoardingActionCharacter::LookUp(float Val)
{
	FRotator newRot = FirstPersonCameraComponent->GetRelativeRotation() + FRotator{-Val * BaseLookUpRate, 0, 0};
	// Restrict movement on this axis so things don't get weird.
	if (newRot.Pitch < 85 && newRot.Pitch > -85) {
		FirstPersonCameraComponent->AddLocalRotation(FRotator{ -Val * BaseLookUpRate, 0, 0 });
	}
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
