// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

enum class EItemState : uint8 
{
	EIS_Hovering UMETA(DisplayName = "Hovering"),
	EIS_Equipped UMETA(DisplayName = "Equipped")
};

class USphereComponent;
UCLASS()
class MYPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RunningTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Values")
	float Amplitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Values")
	float TimeConstant;

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	UFUNCTION(BlueprintPure)
	float TransformedCosin();

	template<typename T>
	T Avg(T First, T Second);

	UFUNCTION()	
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void SpawnPickupSystem();

	virtual void PlayPickupSound();

	EItemState ItemState;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraComponent* SparkleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* PickupEffect;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* PickupSound;

private:


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

template<typename T>
inline T AItem::Avg(T First, T Second)
{
	return (First + Second) / 2;
}
