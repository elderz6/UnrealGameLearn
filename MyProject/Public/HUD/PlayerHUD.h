// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
class UPlayerOverlay;
UCLASS()
class MYPROJECT_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	FORCEINLINE UPlayerOverlay* GetPlayerOverlay() const { return PlayerOverlay; }

protected:
	virtual void BeginPlay() override;

	virtual void PreInitializeComponents() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Slash")
	TSubclassOf<UPlayerOverlay> PlayerOverlayClass;
	
	UPROPERTY()
	UPlayerOverlay* PlayerOverlay;

};
