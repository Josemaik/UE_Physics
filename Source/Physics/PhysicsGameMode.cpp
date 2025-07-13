// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsGameMode.h"
#include "PhysicsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "BreakableTarget.h"
#include <Kismet/GameplayStatics.h>

APhysicsGameMode::APhysicsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void APhysicsGameMode::BeginPlay()
{
	Super::BeginPlay();
	// @TODO: Get total and current target configuration
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ABreakableTarget::StaticClass(),FoundActors);
	m_TotalTargets = FoundActors.Num();
	m_RemainingTargets = FoundActors.Num();
	ABreakableTarget::OnTargetBroken.AddDynamic(this,&APhysicsGameMode::ReduceRemainingTargets);
}

void APhysicsGameMode::ReduceRemainingTargets(ABreakableTarget* BrokenTarget)
{
	// @TODO: make sure to notify other components, including blueprints
	--m_RemainingTargets;
	OnTargetCountChange.Broadcast();
	if (m_RemainingTargets <= 0)
	{
		OnWinConditionMet.Broadcast();
	}
}
