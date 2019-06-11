// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectGrabberComponent.h"
#include "GravityGun.generated.h"

class UObjectGrabberComponent;
class UObjectLauncherComponent;

UCLASS()
class GRAVITYGUNPLAYGROUND_API AGravityGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGravityGun();

	//Input the grab command to the objectgrabbercomponent.
	//When not holding an actor, the grabber will try to grab an actor within range.
	//When holding an object, the grabber will release the actor.
	UFUNCTION(BlueprintCallable)
	virtual void TryGrab();

	//Input the launch command to the objectlauncher.
	//If the objectgrabber is holding an actor, this actor will be launcher.
	//If not, the launcher will attempt to launch an actor within range.
	UFUNCTION(BlueprintCallable)
	virtual void TryLaunch();

protected:
	//Objectgrabber reference. The component will be created and attached to this actor on construction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectInteraction")
	UObjectGrabberComponent* ObjectGrabber = nullptr;

	//ObjectLauncher reference. The component will be created and attached to this actor on construction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectInteraction")
	UObjectLauncherComponent* ObjectLauncher = nullptr;

	UPROPERTY(EditAnywhere)
	USceneComponent* ObjectTransformPlaceholder = nullptr;

private:
	
	
};
