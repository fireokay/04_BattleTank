// Copyright EmbraceIT Ltd.

#include "TankAimingComponent.h"
#include "TankBarrel.h"
#include "Projectile.h"
#include "TankTurret.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UTankAimingComponent::BeginPlay()
{
	// so that first is after initial reload
	LastFireTime = FPlatformTime::Seconds();
}

void UTankAimingComponent::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
	Barrel = BarrelToSet;
	Turret = TurretToSet;
}

void UTankAimingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	// UE_LOG(LogTemp, Warning, TEXT("%f"), FPlatformTime::Seconds() - LastFireTime);
	if ((FPlatformTime::Seconds() - LastFireTime) < ReloadTimeInSeconds)
	{
		FiringState = EFiringState::Reloading;
	}
	else if (IsBarrelMoving())
	{
		FiringState = EFiringState::Aiming;
	}
	else
	{
		FiringState = EFiringState::Locked;
	}
}

bool UTankAimingComponent::IsBarrelMoving()
{
	if (!ensure(Barrel)) { return false; }
	auto BarrelForward = Barrel->GetForwardVector();
	return !BarrelForward.Equals(AimDirection, 0.01);
}


void UTankAimingComponent::AimAt(FVector OutHitLocation)
{
	if (!ensure(Barrel)) {	return;	}
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
		AimDirection = OutLaunchVelocity.GetSafeNormal();
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
	if (!ensure(Barrel) || !ensure(Turret)) { return; }
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

void UTankAimingComponent::Fire()
{
	if (FiringState != EFiringState::Reloading)
	{
		if (!ensure(Barrel)) { return; }
		if (!ensure(ProjectileBlueprint)) { return; }
		// Spawn a projectile at the socket location on the barrel
		auto Projectile = GetWorld()->SpawnActor<AProjectile>
			(
				ProjectileBlueprint,
				Barrel->GetSocketLocation(FName("Projectile")),
				Barrel->GetSocketRotation(FName("Projectile"))
				);

		Projectile->LaunchProjectile(LaunchSpeed);
		LastFireTime = FPlatformTime::Seconds();
	}
}