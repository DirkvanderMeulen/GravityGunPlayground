// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectLauncherComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLaunchEvent);

/*
 * Component that allows the owner to launch physicsactors.
 */
UCLASS(Blueprintable , ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAVITYGUNPLAYGROUND_API UObjectLauncherComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectLauncherComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Launch the supplied actor directly away from the players viewport.
	//Example Usage: Launch object currently being held by the player.
	virtual void LaunchActorFromViewport(AActor* ActorToLaunch);

	//Tries to find an actor through linecast and launches it if found.
	virtual void TryLaunchActorByLinecast();

	//Event called when the grabber successfully launches an object
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLaunchEvent OnLaunchSuccess;

	//Event called when the grabber fails to find an actor to launch
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLaunchEvent OnLaunchFail;

private:
	//The linear force with which an actor is launched
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	float LinearLaunchForce = 750000.0f;

	//The minimum length of the velocity vector with which an actor is launched
	//Intended to make it possible to launch heavier objects as well
	//WARNING: Has no effect if bClampLaunchVelocitySize is set to false
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	float MinimumLaunchVelocitySize = 1400;

	//The maximum length of the velocity vector with which an actor is launched
	//Intended to have control over the speed at which smaller actors are launched
	//WARNING: Has no effect if bClampLaunchVelocitySize is set to false
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	float MaximumLaunchVelocitySize = 4200;

	//Setting this to true will cause the launch velocity to be clamped between the set min and max values
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	bool bClampLaunchVelocitySize = true;

	//The maximum range from which this component can launch an object
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	float HitRange = 650.f;

	//The cooldown in seconds between consecutive launch attempts
	UPROPERTY(EditAnywhere, Category = "LaunchSettings")
	float LaunchCooldownSeconds = 0.3f;

	//Most recent time at which an actor was launched
	float LastSuccesfulLaunchTime = 0.f;
		
	//The location of the viewport(and thus the player) this frame
	FVector ViewportLocation;
	//The rotator of the viewport(and thus the player) this frame
	FRotator ViewportRotator;

	//Returns whether or not enough time has passed for the launcher to launch a new actor
	virtual bool CanLaunch();

	//Launch the supplied actor with an impulse originating from the supplied location.
	virtual void LaunchActorFromLocation(AActor* ActorToLaunch, FVector LaunchLocation);

	//Updates the Viewport's location and rotation
	virtual void UpdateViewportValues();

	//Updates the launch direction to the players aim direction, to make sure the actor is launched in the intended direction
	//Also clamps the launched component's linear and angular velocity between a set minimum and maximum size, 
	//this is done to make the gravity gun properly usable on very heavy or very light objects
	UFUNCTION()
	virtual void AdjustLaunchedComponentVelocity(UPrimitiveComponent* LaunchedComponent, FVector LaunchDirection);

	FHitResult LineTrace(FVector CastOrigin, FVector CastDirection);
};
