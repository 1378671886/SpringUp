// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlatform.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

AMyPlatform::AMyPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PlatformBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PlatformBox"));
	PlatformBox->SetupAttachment(Root);
	PlatformBox->SetBoxExtent(FVector(100.0f, 100.0f, 16.0f));
	PlatformBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformBox->SetCollisionObjectType(ECC_WorldStatic);
	PlatformBox->SetCollisionResponseToAllChannels(ECR_Block);
	PlatformBox->SetMobility(EComponentMobility::Movable);

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(Root);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyPlatform::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateOneWayCollision();
}

void AMyPlatform::UpdateOneWayCollision()
{
	const float Now = GetWorld()->GetTimeSeconds();

	ACharacter* Player = nullptr;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		Player = PC->GetCharacter();
	}

	bool bShouldBlock = true;

	bool bPlayerDetected = false;
	bool bHorizontallyOverlapping = false;
	bool bStandingOnTop = false;
	bool bDropPressed = false;
	FVector PlayerLoc = FVector::ZeroVector;
	ECollisionChannel PlayerChannel = ECC_Pawn;
	const FVector PlatformLoc = PlatformBox->GetComponentLocation();
	const FVector PlatformExtent = PlatformBox->GetScaledBoxExtent();
	float PlayerBottomZ = 0.0f;
	const float PlatformTopZ = PlatformLoc.Z + PlatformExtent.Z;

	if (IsValid(Player))
	{
		const UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
		if (Capsule)
		{
			bPlayerDetected = true;

			PlayerLoc = Player->GetActorLocation();
			PlayerChannel = Capsule->GetCollisionObjectType();
			const float PlayerRadius = Capsule->GetScaledCapsuleRadius();

			PlayerBottomZ = PlayerLoc.Z - Capsule->GetScaledCapsuleHalfHeight();

			// 2D side-scroller: depth (Y) differs between player and platform,
			// so only the X axis determines horizontal overlap.
			bHorizontallyOverlapping =
				FMath::Abs(PlayerLoc.X - PlatformLoc.X) < (PlatformExtent.X + PlayerRadius);

			if (bHorizontallyOverlapping)
			{
				// Feet within a small band around the top surface = standing on this platform.
				bStandingOnTop =
					PlayerBottomZ >= PlatformTopZ - StandingTolerance &&
					PlayerBottomZ <= PlatformTopZ + StandingTolerance;

				// Start a drop-through only when standing on this platform and the key was
				// freshly pressed (edge-triggered, so it doesn't cascade to platforms below).
				bDropPressed = PC && (PC->WasInputKeyJustPressed(DropThroughKey) || PC->WasInputKeyJustPressed(EKeys::Down));
				if (bStandingOnTop && bDropPressed)
				{
					DropThroughUntilTime = Now + DropThroughDuration;
				}

				// One-way: block only when the player's feet are above the top surface
				// (so they land on top), and not during an active drop-through.
				const bool bFeetAboveTop = PlayerBottomZ >= PlatformTopZ;
				const bool bDropThroughActive = Now < DropThroughUntilTime;

				bShouldBlock = bFeetAboveTop && !bDropThroughActive;
			}
		}
	}

	SetPawnBlocking(bShouldBlock, PlayerChannel);

	if (bShowDebug)
	{
		const TCHAR* bDet = bPlayerDetected ? TEXT("Y") : TEXT("N");
		const TCHAR* bXY = bHorizontallyOverlapping ? TEXT("Y") : TEXT("N");
		const TCHAR* bStand = bStandingOnTop ? TEXT("Y") : TEXT("N");
		const TCHAR* bDrop = bDropPressed ? TEXT("Y") : TEXT("N");
		const TCHAR* bRes = bShouldBlock ? TEXT("BLOCK") : TEXT("IGNORE");

		UE_LOG(LogTemp, Warning,
			TEXT("[Platform] %s | Det=%s Ch=%d Overlap=%s Stand=%s Drop=%s FeetZ=%.0f TopZ=%.0f -> %s"),
			*GetName(),
			bDet,
			(int32)PlayerChannel,
			bXY,
			bStand,
			bDrop,
			PlayerBottomZ,
			PlatformTopZ,
			bRes);
	}
}

void AMyPlatform::SetPawnBlocking(bool bBlock, ECollisionChannel PlayerChannel)
{
	const ECollisionResponse NewResponse = bBlock ? ECR_Block : ECR_Ignore;
	if (PlatformBox->GetCollisionResponseToChannel(PlayerChannel) != NewResponse)
	{
		PlatformBox->SetCollisionResponseToChannel(PlayerChannel, NewResponse);
	}
}
