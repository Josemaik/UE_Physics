// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponDamageType.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include <Kismet/GameplayStatics.h>

#include "Physics/ImmediatePhysics/ImmediatePhysicsShared/ImmediatePhysicsCore.h"

APhysicsProjectile::APhysicsProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &APhysicsProjectile::OnHit);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void APhysicsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// @TODO: Handle impact

	
	
	if (m_OwnerWeapon->m_WeaponDamageType->DamageType == EDamageType::Radial)
	{
		// TArray<FHitResult> HitActors;
		// FCollisionShape HitShape = FCollisionShape::MakeSphere(200.f);
		//
		// bool bsweepHit = GetWorld()->SweepMultiByChannel(HitActors,
		// 	Hit.Location,
		// 	Hit.Location + FVector(0.01f,0.01f,0.01f),
		// 	FQuat::Identity,
		// 	ECC_WorldStatic,
		// 	HitShape
		// 	);
		//
		// DrawDebugSphere(GetWorld(),Hit.Location,200.f,50, FColor::Orange,false,2.f);
		// if (bsweepHit)
		// {
		// 	for (auto& HitActor : HitActors)
		// 	{
		// 		if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HitActor.GetActor()->GetRootComponent()))
		// 		{
		// 			MeshComp->SetSimulatePhysics(true);
		// 			MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		// 			if (MeshComp)
		// 			{
		// 				MeshComp->AddRadialForce(Hit.Location,200.f,200000.f,RIF_Constant,false);
		// 			}
		// 		}
		// 	}
		// }
		TArray<AActor*> IgnoredActors{};
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			m_OwnerWeapon->m_WeaponDamageType->Damage,
			Hit.Location,
			m_OwnerWeapon->m_WeaponDamageType->Radius,
			m_OwnerWeapon->m_WeaponDamageType->DamageTypeClass,
			IgnoredActors,
			this,
			GetInstigatorController(),  
			true                       
		);
	}
	else
	{
		//OtherComp->SetSimulatePhysics(true);
		//OtherComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		// OtherComp->AddImpulse(-Hit.Normal * LinealForce);
		// UGameplayStatics::ApplyDamage(
		// 	Hit.GetActor(),
		// 	m_OwnerWeapon->m_WeaponDamageType->Damage,
		// 	Hit.GetActor()->GetInstigatorController(),
		// 	this,
		// 	m_OwnerWeapon->m_WeaponDamageType->DamageTypeClass
		// 	);
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			m_OwnerWeapon->m_WeaponDamageType->Damage,
			-Hit.Normal, // Dirección de la fuerza
			Hit,
			GetInstigatorController(), // Controlador del instigador
			this,
			m_OwnerWeapon->m_WeaponDamageType->DamageTypeClass
		);
	}
	
	if (m_DestroyOnHit)
	{
		Destroy();
	}
}