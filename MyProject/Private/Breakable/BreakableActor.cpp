#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
	bIsBroken = false;
	PrimaryActorTick.bCanEverTick = false;
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnChaosBreakEvent);
}

void ABreakableActor::OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent)
{
	if (bIsBroken) return;
	SpawnLoot();
}

int32 ABreakableActor::DetermineDrop()
{
	float TotalRate = 0;
	for (TSubclassOf<ATreasure> Treasure : TreasureClasses)
	{
		//Getting pointer to Items in the array
		ATreasure* IterItem = Treasure.GetDefaultObject();
		if (IterItem->GetDropRate())
		{
			//Sum total drop chances
			TotalRate += IterItem->GetDropRate();
		}
	}
	
	float DiceRoll = FMath::FRand() * TotalRate;
	
	for (size_t i = 0; i < TreasureClasses.Num(); i++)
	{
		//Getting pointer to Items in the array
		ATreasure* IterItem = TreasureClasses[i].GetDefaultObject();
		const float DropRate = IterItem->GetDropRate();
		
		if (DiceRoll < DropRate)
		{
			return i;
		}
		DiceRoll -= DropRate;
	}
	return 0;
}

void ABreakableActor::PlayBreakSound()
{
}

void ABreakableActor::SpawnLoot()
{
	UWorld* World = GetWorld();
	if (World && TreasureClasses.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 75.f;

		int32 Selection = DetermineDrop();

		World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());
		Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		bIsBroken = true;
		SetLifeSpan(3.f);
	}
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (bIsBroken) return;
	SpawnLoot();
}

