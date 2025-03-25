#include "Characters/SlashCharacter.h"
#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include "GameFramework/CharacterMovementComponent.h"
#include <Components/InputComponent.h>
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/PlayerHUD.h"
#include "HUD/PlayerOverlay.h"
#include "Items/Item.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"

ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 600.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true;
	
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");


}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
			(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashCharacterMappingContext, 0);
		}
		InitializePlayerOverlay(PlayerController);
	}
	
	Tags.Add(FName("SlashCharacter"));
}

void ASlashCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
}

void ASlashCharacter::InitializePlayerOverlay(APlayerController* PlayerController)
{
	APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PlayerController->GetHUD());
	if (PlayerHUD)
	{
		PlayerOverlay = PlayerHUD->GetPlayerOverlay();
		if (PlayerOverlay)
		{
			PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
			PlayerOverlay->SetStaminaBarPercent(1.f);
			PlayerOverlay->SetGold(0);
			PlayerOverlay->SetSouls(0);
		}
	}
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (!IsIdle()) return;
	const FVector2D MovementVector = Value.Get<FVector2d>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Forward, MovementVector.Y);

	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Right, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

int32 ASlashCharacter::PlayDeathMontage()
{
	Super::PlayDeathMontage();

	return 0;
}

void ASlashCharacter::PlayAttackMontage(float Playrate)
{
	Super::PlayAttackMontage(3);
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::AttackEnd()
{
	if (!IsDead())
	{
		ActionState = EActionState::EAS_Idle;
	}
}

bool ASlashCharacter::IsIdle()
{
	return ActionState == EActionState::EAS_Idle;
}

bool ASlashCharacter::CanUnequip()
{
	return IsIdle() && CharacterState != ECharacterState::ECS_Unequipped && EquippedWeapon;
}

bool ASlashCharacter::CanEquip()
{
	return IsIdle() && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

bool ASlashCharacter::HasStamina()
{
	return Attributes && Attributes->GetStaminaPercent() > 0.f;
}

bool ASlashCharacter::IsDead()
{
	return Super::IsDead() || ActionState == EActionState::EAS_Dead;
}

void ASlashCharacter::SheatheWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::UnSheatheWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::Jump()
{
	if (!IsIdle()) return;
	Super::Jump();
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	if (IsIdle() && CharacterState != ECharacterState::ECS_Unequipped)
	{
		ASlashCharacter::PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Dodge()
{
	if (IsDead() || !IsIdle() || !HasStamina()) return;

	if (Attributes)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		UpdateStaminaBar();
		PlayDodgeMontage();
		ActionState = EActionState::EAS_Dodging;
	}
}

void ASlashCharacter::UpdateStaminaBar()
{
	if (Attributes && PlayerOverlay)
	{
		PlayerOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::EKeyPressed()	
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		EquippedWeapon = OverlappingWeapon;
		OverlappingItem = nullptr;
	}
	else
	{
		if (CanUnequip())
		{
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_Equipping;
		}
		else if(CanEquip())
		{
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_Equipping;
		}
	}
}


void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attributes)
	{
		Attributes->RegenStamina(DeltaTime);
		UpdateStaminaBar();
	}
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent =
		CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	}

}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsDodging()) return;

	Super::GetHit_Implementation(ImpactPoint, Hitter);

	if(!IsDead())
		ActionState = EActionState::EAS_HitReaction;
}

bool ASlashCharacter::IsDodging()
{
	return ActionState == EActionState::EAS_Dodging;
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDodging()) return 0.f;
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	 OverlappingItem = Item; 
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && PlayerOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		PlayerOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && PlayerOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		PlayerOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (PlayerOverlay && Attributes)
	{
		PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
