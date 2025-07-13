#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "WeaponDamageType.generated.h"

UENUM()
enum class EDamageType
{
	Lineal,
	Radial,
	HitScan
};


UCLASS(Blueprintable, EditInlineNew)
class PHYSICS_API UWeaponDamageType : public UObject
{
	GENERATED_BODY()

public:
	/** @TODO: Create damage data object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Damage")
	EDamageType DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Damage")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Damage")
	float Radius;
};
