#include "Characters/BaseCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ABaseCharacter::IsDead()
{
	if (Attributes)
		return Attributes->GetHealthPercent() == 0.f;

	return false;
}

void ABaseCharacter::Die()
{
	PlayDeathMontage();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
	DisableCapsule();
	Tags.Add("Dead");
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag("Dead"))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::CanAttack()
{
}

void ABaseCharacter::HandleDamage(float Damage)
{
	if (Attributes)
		Attributes->ReceiveDamage(Damage);
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector ForwardVector = GetActorForwardVector();
	//Lower Impact Point to Enemy Z
	const FVector ImpactLower(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();

	// Forward * Tohit = |Forward| |ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(ForwardVector, ToHit);
	// ACos of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);
	//convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);
	// If CrossProduct points down, theta is negative
	const FVector CrossProd = FVector::CrossProduct(ForwardVector, ToHit);
	if (CrossProd.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayHitReactMontage(Section);

}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);

}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{

	if (HitParticles && GetWorld())
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage, PlayRate);
		const int32 Selection = FMath::RandRange(1, Montage->GetNumSections());
		FName SectionName = Montage->GetSectionName(Selection - 1);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);

		return Selection;
	}
	return 0;
}

void ABaseCharacter::PlayAttackMontage(float PlayRate)
{
	PlayRandomMontageSection(AttackMontage, PlayRate);
}

void ABaseCharacter::PlayDodgeMontage(float PlayRate)
{
	PlayRandomMontageSection(DodgeMontage, PlayRate);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage, 1.5);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, 1);
	DeathPose = EDeathPose(Selection - 1);
	return Selection;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}

}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

	if (IsAlive() && Hitter)
		DirectionalHitReact(Hitter->GetActorLocation());
	else
		Die();

}

