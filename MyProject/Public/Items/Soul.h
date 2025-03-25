// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Soul.generated.h"

/**
 * 
 */
class UNiagaraSystem;
UCLASS()
class MYPROJECT_API ASoul : public AItem
{
	GENERATED_BODY()

public:
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE void SetSouls(int32 Amount) { Souls = Amount; }

	//Defined in blueprints
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateNiagaraVariables();


protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soul Properties")
	int32 Souls;

private:


};
