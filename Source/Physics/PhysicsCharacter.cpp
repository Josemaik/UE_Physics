// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCharacter.h"
#include "PhysicsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Components/StaticMeshComponent.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>

#include "GameFramework/PhysicsVolume.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APhysicsCharacter::APhysicsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void APhysicsCharacter::BeginPlay()
{
	Super::BeginPlay();
	m_CurrentHealth = m_MaxHealth;
}

void APhysicsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	
	// @TODO: Stamina update
	if (bIsSprinting)
	{
		CurrentStamina -= m_StaminaDepletionRate / 100.f * DeltaSeconds;
	}
	else
	{
		CurrentStamina += m_StaminaRecoveryRate / 100.f * DeltaSeconds;
	}
	// @TODO: Physics objects highlight

	if (!m_isGrabbing)
	{
		FVector StartTrace = GetActorLocation() +  FirstPersonCameraComponent->GetRelativeLocation();
		FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * m_MaxGrabDistance;
		FHitResult HitResult;
		bool succes = UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			StartTrace,
			EndTrace,
			TraceTypeQuery1,
			false,
			{},
			EDrawDebugTrace::None,
			HitResult,
			true,
			FLinearColor::Green,
			FLinearColor::Red,
			5.f
			);
		if (succes && HitResult.GetComponent()->Mobility)
		{
			SetHighlightedMesh(Cast<UMeshComponent>(HitResult.GetComponent()));
		}
		else
		{
			SetHighlightedMesh(nullptr);
		}
	}
	
	// @TODO: Grabbed object update
	FVector forward = FirstPersonCameraComponent->GetForwardVector();
	FVector start = GetActorLocation() + FirstPersonCameraComponent->GetRelativeLocation();
	if (m_isGrabbing)
	{
		m_PhysicsHandle->SetTargetLocation(start + forward * m_CurrentGrabDistance);
		m_PhysicsHandle->SetTargetRotation(FirstPersonCameraComponent->GetComponentRotation());
	}
}

void APhysicsCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APhysicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Look);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::GrabObject);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Completed, this, &APhysicsCharacter::ReleaseObject);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APhysicsCharacter::SetIsSprinting(bool NewIsSprinting)
{
	// @TODO: Enable/disable sprinting use CharacterMovementComponent
	if (bIsSprinting == NewIsSprinting)
	{
		return;
	}

	if (CurrentStamina <= 0.f)
	{
		bIsSprinting = false;
	}
	
	bIsSprinting = NewIsSprinting;
	
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed *= m_SprintSpeedMultiplier;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed /= m_SprintSpeedMultiplier;
	}
}

void APhysicsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APhysicsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APhysicsCharacter::Sprint(const FInputActionValue& Value)
{
	SetIsSprinting(Value.Get<bool>());
}

void APhysicsCharacter::GrabObject(const FInputActionValue& Value)
{
	// @TODO: Grab objects using UPhysicsHandleComponent
	if (!m_isGrabbing)
	{
		FVector StartTrace = GetActorLocation() +  FirstPersonCameraComponent->GetRelativeLocation();
		FVector EndTrace = StartTrace + FirstPersonCameraComponent->GetForwardVector() * m_MaxGrabDistance;
		FHitResult HitResult;
		bool succes = UKismetSystemLibrary::LineTraceSingle(
			GetWorld(),
			StartTrace,
			EndTrace,
			TraceTypeQuery1,
			false,
			{},
			EDrawDebugTrace::ForDuration,
			HitResult,
			true,
			FLinearColor::Green,
			FLinearColor::Red,
			5.f
			);
		if (succes)
		{
			GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,FString::Printf(TEXT("ObjetoHiteado:%s"),*HitResult.GetActor()->GetActorNameOrLabel()));
			UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
			m_PhysicsHandle->GrabComponentAtLocationWithRotation(ComponentToGrab,HitResult.BoneName,HitResult.Location,HitResult.Component->GetComponentRotation());
			m_PhysicsHandle->SetInterpolationSpeed(m_BaseInterpolationSpeed / HitResult.GetComponent()->GetMass());
			m_CurrentGrabDistance = HitResult.Distance;
			SetHighlightedMesh(Cast<UStaticMeshComponent>(HitResult.GetComponent()));
			m_isGrabbing = true;
		}
	}
}

void APhysicsCharacter::ReleaseObject(const FInputActionValue& Value)
{
	// @TODO: Release grabebd object using UPhysicsHandleComponent
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,FString::Printf(TEXT("Release:")));
	if (m_PhysicsHandle->GetGrabbedComponent())
	{
		m_PhysicsHandle->ReleaseComponent();
	}
	m_isGrabbing = false;
}

void APhysicsCharacter::SetHighlightedMesh(UMeshComponent* StaticMesh)
{
	if(m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(nullptr);
	}
	m_HighlightedMesh = StaticMesh;
	if (m_HighlightedMesh)
	{
		m_HighlightedMesh->SetOverlayMaterial(m_HighlightMaterial);
	}
}
