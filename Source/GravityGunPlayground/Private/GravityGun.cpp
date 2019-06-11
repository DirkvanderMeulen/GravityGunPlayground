// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityGun.h"
#include "ObjectGrabberComponent.h"
#include "ObjectLauncherComponent.h"
#include "Engine/World.h"

// Sets default values
AGravityGun::AGravityGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ObjectGrabber = CreateDefaultSubobject<UObjectGrabberComponent>("ObjectGrabber");
	
	ObjectLauncher = CreateDefaultSubobject<UObjectLauncherComponent>("ObjectLauncher");
}

void AGravityGun::TryGrab()
{
	if (!(ObjectGrabber)) return;

	ObjectGrabber->ToggleGrabActor();
}

void AGravityGun::TryLaunch()
{
	if (!(ObjectLauncher && ObjectGrabber)) return;

	AActor* GrabbedObject;
	if (ObjectGrabber->GetGrabbedActor(GrabbedObject))
	{
		ObjectGrabber->ReleaseActor();
		ObjectLauncher->LaunchActorFromViewport(GrabbedObject);
	}
	else
	{
		ObjectLauncher->TryLaunchActorByLinecast();
	}
}

