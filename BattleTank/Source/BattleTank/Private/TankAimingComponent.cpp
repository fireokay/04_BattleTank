// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAimingComponent.h"
#include "TankBarrel.h"
#include "TankTurret.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false; // TODO Should this really tick?

	// ...
}

void UTankAimingComponent::SetBarrelReference(UTankBarrel* BarrelToSet)
{
	if (!BarrelToSet) { return; }
	Barrel = BarrelToSet;
}

void UTankAimingComponent::SetTurretReference(UTankTurret* TurretToSet)
{
	if (!TurretToSet) { return; }
	Turret = TurretToSet;
}

void UTankAimingComponent::AimAt(FVector OutHitLocation, float LaunchSpeed)
{
	if (!Barrel) {	return;	}
	// UE_LOG(LogTemp, Warning, TEXT("%f"), LaunchSpeed);
	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	// UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f"), StartLocation.GetSafeNormal().X, StartLocation.GetSafeNormal().Y, StartLocation.GetSafeNormal().Z);
	bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
		this,
		OutLaunchVelocity,
		StartLocation,
		OutHitLocation,
		LaunchSpeed,
		false,
		0,
		0,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);
	if (bHaveAimSolution)
	{

		auto AimDirection = OutLaunchVelocity.GetSafeNormal();
		// auto Time = GetWorld()->GetTimeSeconds();
		// UE_LOG(LogTemp, Warning, TEXT("%f AimDirection: %f, %f, %f"), Time, OutLaunchVelocity.GetSafeNormal().X, OutLaunchVelocity.GetSafeNormal().Y, OutLaunchVelocity.GetSafeNormal().Z);
		MoveBarrelTowards(AimDirection);
	}
	// else
	// {
		// auto Time = GetWorld()->GetTimeSeconds();
		// UE_LOG(LogTemp, Warning, TEXT("%f Not found"), Time)
	// }

}

void UTankAimingComponent::MoveBarrelTowards(FVector AimDirection)
{
	// UE_LOG(LogTemp, Warning, TEXT("AimDirection: %f"), AimDirection.X);
	// Work-oyt difference between current barrel roation, and AimDirection
	auto BarrelRotator = Barrel->GetForwardVector().Rotation();
	// UE_LOG(LogTemp, Warning, TEXT("BarrelRotatorYAW: %f"), BarrelRotator.Yaw);
	auto AimAsRotator = AimDirection.Rotation();
	// UE_LOG(LogTemp, Warning, TEXT("AimAsRotator: %f"), AimAsRotator.Yaw);
	auto DeltaRotator = AimAsRotator - BarrelRotator;
	// UE_LOG(LogTemp, Warning, TEXT("DeltaRotation: %f"), DeltaRotator.Yaw);

	Barrel->Elevate(DeltaRotator.Pitch);

	if (FMath::Abs(DeltaRotator.Yaw) < 180)
	{
		Turret->Rotate(DeltaRotator.Yaw);
	}
	else
	{
		Turret->Rotate(-DeltaRotator.Yaw);
	}
}
