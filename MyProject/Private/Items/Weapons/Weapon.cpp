#include "Items/Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include <Characters/SlashCharacter.h>
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"


AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}


void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.AddUnique(GetOwner());

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}


	UKismetSystemLibrary::BoxTraceSingle(this, Start, End, BoxTraceSize,
		BoxTraceStart->GetComponentRotation(), ETraceTypeQuery::TraceTypeQuery1, 
		false, ActorsToIgnore, 
		bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit, true);
	IgnoreActors.AddUnique(BoxHit.GetActor());
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{

	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FName Type = FName(TEXT("Enemy"));
	if (ActorIsSameType(Type, OtherActor))
		return;

	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (BoxHit.GetActor()) 
	{
		if (ActorIsSameType(Type, BoxHit.GetActor()))
			return;

		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage,
			GetInstigator()->GetController(), this, UDamageType::StaticClass());

		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);
	}

}

bool AWeapon::ActorIsSameType(const FName& Type, AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(Type) && OtherActor->ActorHasTag(Type);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	ItemState = EItemState::EIS_Equipped;
	if (EquipSound && NewOwner->ActorHasTag("SlashCharacter"))
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (SparkleEffect)
	{
		SparkleEffect->Deactivate();
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}
