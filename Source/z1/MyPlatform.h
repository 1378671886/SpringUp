// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "MyPlatform.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ACharacter;

UCLASS()
class Z1_API AMyPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMyPlatform();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Physical collision: blocks the player so they can stand on top of it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	TObjectPtr<UBoxComponent> PlatformBox;

	// Optional visual mesh. Collision is disabled; PlatformBox owns all collision.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	// Key pressed while standing on the platform to drop down through it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FKey DropThroughKey = EKeys::S;

	// How long the platform keeps ignoring the player after a drop-through starts.
	// Increase this for thick platforms so the player has time to fall clear.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (ClampMin = "0.0"))
	float DropThroughDuration = 0.3f;

	// How far above the platform's top surface a player's feet can be and still count
	// as "standing on top" (used to trigger drop-through on exactly this platform).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform", meta = (ClampMin = "0.0"))
	float StandingTolerance = 30.0f;

	// When enabled, prints the one-way collision state on screen every frame (for debugging).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	bool bShowDebug = false;

private:
	void UpdateOneWayCollision();
	void SetPawnBlocking(bool bBlock, ECollisionChannel PlayerChannel);

	float DropThroughUntilTime = 0.0f;
};
