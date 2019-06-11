// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectLauncherComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UObjectLauncherComponent::UObjectLauncherComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UObjectLauncherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UObjectLauncherComponent::LaunchActorFromViewport(AActor* ActorToLaunch)
{
	if (!CanLaunch()) { return; }

	UpdateViewportValues();
	LaunchActorFromLocation(ActorToLaunch, ViewportLocation);
}

void UObjectLauncherComponent::TryLaunchActorByLinecast()
{
	if (!CanLaunch()) { return; }

	UpdateViewportValues();
	FHitResult Hit = LineTrace(ViewportLocation, ViewportRotator.Vector());
	
	///No valid actor hit
	if (!Hit.GetActor())
	{
		OnLaunchFail.Broadcast();
		return;
	}

	LaunchActorFromLocation(Hit.GetActor(), Hit.Location);
}

void UObjectLauncherComponent::LaunchActorFromLocation(AActor* ActorToLaunch, FVector LaunchLocation)
{
	if (!CanLaunch()) { return; }

	UpdateViewportValues();
	
	UPrimitiveComponent* ComponentToLaunch = Cast<UPrimitiveComponent>(ActorToLaunch->GetRootComponent());
	ComponentToLaunch->AddImpulseAtLocation(ViewportRotator.Vector() * LinearLaunchForce, LaunchLocation);

	///If velocity should be clamped, set a timer to clamp velocity on the next frame.
	///Done this way because the impulse won't have an effect on the actor until the next frame
	if(bClampLaunchVelocitySize)
	{
		FTimerHandle VelocityNormalizingTimerHandle;
		const FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UObjectLauncherComponent::AdjustLaunchedComponentVelocity, ComponentToLaunch, ViewportRotator.Vector());
		GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
	}
	
	LastSuccesfulLaunchTime = GetWorld()->GetTimeSeconds();
	OnLaunchSuccess.Broadcast();
}

bool UObjectLauncherComponent::CanLaunch()
{
	return GetWorld()->GetTimeSeconds() > LastSuccesfulLaunchTime + LaunchCooldownSeconds;
}

void UObjectLauncherComponent::UpdateViewportValues()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->GetPlayerViewPoint(ViewportLocation, ViewportRotator);
}

void UObjectLauncherComponent::AdjustLaunchedComponentVelocity(UPrimitiveComponent* LaunchedComponent, FVector LaunchDirection)
{
	///Override the launch velocity in the launch direction to ensure the actor being shot in a straight line from the players viewport
	FVector NewLaunchVelocity = LaunchDirection;

	///Normalize the velocity and clamp it between the minimum and maximum sizes
	NewLaunchVelocity.Normalize();
	NewLaunchVelocity *= FMath::Clamp(LaunchedComponent->GetPhysicsLinearVelocity().Size(), MinimumLaunchVelocitySize, MaximumLaunchVelocitySize);
	LaunchedComponent->SetPhysicsLinearVelocity(NewLaunchVelocity);
}

FHitResult UObjectLauncherComponent::LineTrace(FVector CastOrigin, FVector CastDirection)
{
	FHitResult OutHit;
	GetWorld()->LineTraceSingleByObjectType(
		OutHit,
		CastOrigin,
		CastOrigin + CastDirection * HitRange,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		FCollisionQueryParams(FName(TEXT("")),
			false,
			GetOwner()
		));
	return OutHit;
}

