// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameViewportClient.h"

void UMyGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	// Force unlit rendering (no lighting) when the game runs or is packaged.
	ViewModeIndex = VMI_Unlit;

	// Disable post-processing (tone mapping / ACES) so 2D art is output with
	// gamma-only correction, making sprite colors match the source textures 1:1.
	EngineShowFlags.SetPostProcessing(false);
}

