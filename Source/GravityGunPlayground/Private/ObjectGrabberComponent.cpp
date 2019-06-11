// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectGrabberComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
UObjectGrabberComponent::UObjectGrabberComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandle");
}

// Called when the game starts
void UObjectGrabberComponent::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	PlayerController = GetWorld()->GetFirstPlayerController();

	///Set the forcereleasedistance to at least to grabrange. This to prevent unintended releasing of actors
	if(ForceReleaseDistance <  GrabRange)
	{
		ForceReleaseDistance = GrabRange + 5;
	}
	OnCanGrabChanged.Broadcast(false);
}

// Called every frame
void UObjectGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }
	UpdateViewportValues();

	if (PhysicsHandle->GrabbedComponent)
	{		
		UpdateGrabbedComponent();
	}
	else
	{
		UpdateActorInRange();
	}
}

void UObjectGrabberComponent::ToggleGrabActor()
{
	if (!PhysicsHandle) { return; }

	if (PhysicsHandle->GrabbedComponent)
	{
		ReleaseActor();
	}
	else
	{
		GrabActor();
	}
}

void UObjectGrabberComponent::GrabActor()
{
	///Not enough time has passed since most recent release of an object
	if (!HasReloaded()) { return; }
	
	if (!(PhysicsHandle)) { return; }

	///Player is already holding an object
	if (PhysicsHandle->GrabbedComponent) { return; }

	const FHitResult Hit = LineTrace(ViewportLocation, ViewportRotator.Vector());
	AActor* HitActor = Hit.GetActor();
	
	///No valid actor hit
	if (!HitActor) { return; }

	///Don't grab the actor if the player is standing on top of the grabbed actor.
	///This is done to prevent the player from lifting themselves through grabbing objects.
	if (IsPlayerOverlappingActor(HitActor))
	{
		return;
	}

	///Calculate the initial rotation of the grabbed actor relative to the player's viewport
	InitialRelativeRotation = ViewportRotator.Quaternion().Inverse() * Hit.GetComponent()->GetOwner()->GetActorRotation().Quaternion();

	///Calculate the actor center
	FVector ActorCenter, ActorBounds;
	HitActor->GetActorBounds(false, ActorCenter, ActorBounds);
	
	///Attach the actor to the physicshandle, using the actor's center and current rotation
	PhysicsHandle->GrabComponentAtLocationWithRotation(
		Hit.GetComponent(),
		NAME_None,
		ActorCenter,
		Hit.GetActor()->GetActorRotation()
	);

	///Calculate the initial grabdistance. Set it to the max hover distance if the value is greater.
	InitialGrabDistance = (Hit.GetComponent()->GetOwner()->GetActorLocation() - ViewportLocation).Size();
	if(InitialGrabDistance > MaximumHoverDistance)
	{
		InitialGrabDistance = MaximumHoverDistance;
	}
	OnGrab.Broadcast();
}

void UObjectGrabberComponent::ReleaseActor()
{
	UPrimitiveComponent* GrabbedComponent = PhysicsHandle->GrabbedComponent;
	if (!GrabbedComponent) { return; }

	///If the object has too much linear velocity, limit both the linear and angular velocity.
	FVector LinearVelocity = GrabbedComponent->GetPhysicsLinearVelocity();
	if(LinearVelocity.Size() > MaxActorVelocityOnRelease)
	{
		FVector AngularVelocity = GrabbedComponent->GetPhysicsAngularVelocity();

		///Normalize the linear velocity, and limit the angular velocity by an equal amount.
		AngularVelocity /= LinearVelocity.Size();
		LinearVelocity.Normalize();
		
		LinearVelocity *= MaxActorVelocityOnRelease;
		AngularVelocity *= MaxActorVelocityOnRelease;

		GrabbedComponent->SetPhysicsLinearVelocity(LinearVelocity);
		GrabbedComponent->SetPhysicsAngularVelocity(AngularVelocity);
	}

	PhysicsHandle->ReleaseComponent();
	OnRelease.Broadcast();

	LastReleaseTime = GetWorld()->GetTimeSeconds();
}

bool UObjectGrabberComponent::GetGrabbedActor(AActor*& OutGrabbedActor)
{
	if(!PhysicsHandle->GetGrabbedComponent())
	{
		OutGrabbedActor = nullptr;
		return false;
	}
	OutGrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
	return true;
}

void UObjectGrabberComponent::UpdateGrabbedComponent()
{
	UPrimitiveComponent* GrabbedComponent = PhysicsHandle->GetGrabbedComponent();

	///No actor currently being held
	if (!GrabbedComponent) { return; }

	///Check if the owning actor is attached to a parent (E.g the gun is equipped). 
	///If not, release held object.
	if(!GetOwner()->GetAttachParentActor())
	{
		ReleaseActor();
		return;
	}
	///Release the actor if the player is standing on top of the grabbed actor.
	///This is done to prevent the player from lifting themselves through grabbing objects.
	if (IsPlayerOverlappingActor(GrabbedComponent->GetOwner()))
	{
		ReleaseActor();
		return;
	}

	///Decide the hover distance based on object size. 
	///Object size is calculated by subtracting distance to the closest point on the actor from the distance to the actor location.
	FVector ActorCenter, ActorBounds;
	GrabbedComponent->GetOwner()->GetActorBounds(false, ActorCenter, ActorBounds);
	const float DistanceToCenter = (ActorCenter - ViewportLocation).Size();
	FVector ClosestPointOnCollision;
	const float DistanceToClosestPoint = PhysicsHandle->GetGrabbedComponent()->GetDistanceToCollision(ViewportLocation, ClosestPointOnCollision);
	const float DistanceDelta = DistanceToCenter - DistanceToClosestPoint;
	const float HoverDistance = DistanceDelta + InitialGrabDistance;

	///Release the actor if it's too far away from the player. 
	if (DistanceToClosestPoint > ForceReleaseDistance)
	{
		ReleaseActor();
		return;
	}

	///Update transform values on the hovering object.
	PhysicsHandle->SetTargetLocationAndRotation(ViewportLocation + ViewportRotator.Vector() * HoverDistance,
		FRotator(ViewportRotator.Quaternion() * InitialRelativeRotation));
}

void UObjectGrabberComponent::UpdateViewportValues()
{
	if(PlayerController)
	{
		PlayerController->GetPlayerViewPoint(ViewportLocation, ViewportRotator);
	}
}

void UObjectGrabberComponent::UpdateActorInRange()
{
	FHitResult HitResult = LineTrace(ViewportLocation, ViewportRotator.Vector());
	AActor* HitActor = HitResult.GetActor();
	if (HitResult.GetActor() != nullptr && ActorCurrentlyAimedAt == nullptr)
	{
		OnCanGrabChanged.Broadcast(true);
		ActorCurrentlyAimedAt = HitResult.GetActor();
	}
	else if (HitResult.GetActor() == nullptr && ActorCurrentlyAimedAt != nullptr)
	{
		OnCanGrabChanged.Broadcast(false);
		ActorCurrentlyAimedAt = nullptr;
	}
}

bool UObjectGrabberComponent::IsPlayerOverlappingActor(AActor* ActorToCheck) const
{
	///Check if the owning actor is attached to a parent (E.g the gun is equipped). 
	///If so, check if the parent is overlapping with the grabbed actor.
	AActor* ActorParent = GetOwner()->GetAttachParentActor();
	if (!ActorParent) { return false; }
	return ActorParent->IsOverlappingActor(ActorToCheck);
}

bool UObjectGrabberComponent::HasReloaded()
{
	return GetWorld()->GetTimeSeconds() > LastReleaseTime + GrabCooldownSeconds;
}

FHitResult UObjectGrabberComponent::LineTrace(FVector CastOrigin, FVector CastDirection) const
{
	FHitResult OutHit;
	GetWorld()->LineTraceSingleByObjectType(
		OutHit, 
		CastOrigin, 
		CastOrigin + CastDirection * GrabRange, 
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), 
		FCollisionQueryParams(FName(TEXT("")), 
		false, 
		GetOwner()
		));
	return OutHit;
}

