// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitscanWeaponComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "PhysicsCharacter.h"
#include "PhysicsWeaponComponent.h"
#include <Camera/CameraComponent.h>
#include <Components/SphereComponent.h>

#include "UObject/FastReferenceCollector.h"

void UHitscanWeaponComponent::Fire()
{
	Super::Fire();
	m_WeaponDamageType->DamageType = EDamageType::HitScan;
	
	// @TODO: Add firing functionality
	FHitResult OutHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	
	FVector StartTrace = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	FVector EndTrace = PlayerController->PlayerCameraManager->GetActorForwardVector() * m_fRange + StartTrace; 
	

	DrawDebugLine(
		GetWorld(),
		StartTrace,
		EndTrace,
		FColor::Red,
		true,		3
		);
	GetWorld()->LineTraceSingleByChannel(
		OutHit,
		StartTrace,
		EndTrace,
		ECC_Visibility,
		Params
		);

	if (OutHit.bBlockingHit && OutHit.GetActor())
	{
		// FVector HitLocation = -OutHit.Normal * 100000.f;
		// OutHit.Component->AddImpulse(HitLocation);
		onHitscanImpact.Broadcast(OutHit.GetActor(), OutHit.ImpactPoint, OutHit.GetActor()->GetActorForwardVector());
		UGameplayStatics::ApplyPointDamage(
			OutHit.GetActor(),
			m_WeaponDamageType->Damage,
			-OutHit.Normal, // Dirección de la fuerza
			OutHit,
			0, // Controlador del instigador
			0,
			m_WeaponDamageType->DamageTypeClass
		);
	}
}
