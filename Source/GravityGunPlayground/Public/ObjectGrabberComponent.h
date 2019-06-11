// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectGrabberComponent.generated.h"


class UPhysicsHandleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrabEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCanGrabEvent, bool, CanGrab);

/*
 * Component that allows the actor to grab, release and hold physicsactors.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAVITYGUNPLAYGROUND_API UObjectGrabberComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UObjectGrabberComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Initiates a grab if the player is not currently holding an actor. 
	//Conversely, if an actor is being held, the actor will be released.
	UFUNCTION(BlueprintCallable)
	virtual void ToggleGrabActor();

	//Grab an actor. The target actor will be decided by Linecasting from the players viewport.
	UFUNCTION(BlueprintCallable)
	virtual void GrabActor();

	//Release the object currently being held
	UFUNCTION(BlueprintCallable)
	virtual void ReleaseActor();

	//Event called when the grabber grabs an object
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FGrabEvent OnGrab;

	//Event called when the grabber grabs an object
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FGrabEvent OnRelease;

	//Event called when the grabber grabs an object
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FCanGrabEvent OnCanGrabChanged;

	//Returns whether or not there's an actor currently being held. Assigns the GrabbedActor to the supplied out parameter
	UFUNCTION(BlueprintCallable)
	virtual bool GetGrabbedActor(AActor*& OutGrabbedActor);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	//The maximum distance from which an actor can be grabbed
	UPROPERTY(EditAnywhere, Category = "GrabSettings")
	float GrabRange = 950.f;

	//The hovering distance between the player and the closest point of the actor to the player
	UPROPERTY(EditAnywhere, Category = "GrabSettings")
	float MaximumHoverDistance = 300.f;

	//The maximum velocity an actor is allowed to have when releasing. This prevents launching actors at insane speeds when releasing them
	UPROPERTY(EditAnywhere, Category = "GrabSettings")
	float MaxActorVelocityOnRelease = 900.f;

	//The cooldown in seconds between consecutive grabs
	UPROPERTY(EditAnywhere, Category = "GrabSettings")
	float GrabCooldownSeconds = 0.25f;

	//If the hovering actor is more than this distance separated from the player, it will automatically be released.
	//If lower than the grabrange, it will automatically be set to the same value as the grabrange to prevent unintended releasing of the object.
	UPROPERTY(EditAnywhere, Category = "GrabSettings")
	float ForceReleaseDistance = 800.f;

	//Most recent time at which an actor was released
	float LastReleaseTime = 0.f;

	//Distance at which an actor is first grabbed
	//Used to calculate the hover distance
	float InitialGrabDistance = 0.f;

	//Rotation of the grabbed actor when first grabbed
	//This is later applied to the actor to keep the same relative rotation to the player
	FQuat InitialRelativeRotation;

	//Actor currently being aimed at by the player
	AActor* ActorCurrentlyAimedAt = nullptr;

	//Reference to the attached physicshandle. The grabbed component will be attached to this component.
	UPhysicsHandleComponent* PhysicsHandle = nullptr;

	//Cached reference to the playercontroller, because this reference will be used every tick.
	APlayerController* PlayerController = nullptr;

	//The location of the viewport(and thus the player) this frame
	FVector ViewportLocation;
	//The rotator of the viewport(and thus the player) this frame
	FRotator ViewportRotator;

	//Updates the viewport location and rotator 
	virtual void UpdateViewportValues();
	
	//Updates the transform values on the grabbed component
	virtual void UpdateGrabbedComponent();
	
	//Updates if there's an actor in the right range and location to initiate a grab
	//Fires an event if this state changes
	//Example Usage: Update player crosshair color when aiming at a potential grab target
	void UpdateActorInRange();

	//Check if player is overlapping an actor
	//Mainly used to prevent player from lifting themselves with grabbed objects
	bool IsPlayerOverlappingActor(AActor* ActorToCheck) const;

	//Returns whether all criteria have been met before grabbing an object
	virtual bool HasReloaded();

	FHitResult LineTrace(FVector CastOrigin, FVector CastDirection) const;
};
