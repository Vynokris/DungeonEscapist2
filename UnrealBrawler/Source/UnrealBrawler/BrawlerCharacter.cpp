#include "BrawlerCharacter.h"

#include "EnemyAiController.h"
#include "DebugUtils.h"
#include "KnifeActor.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

ABrawlerCharacter::ABrawlerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	// Setup camera spring arm.
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	SpringArmComponent->bEnableCameraLag        = true;
	SpringArmComponent->CameraLagSpeed          = CameraLag;
	SpringArmComponent->TargetArmLength         = CameraDistance;
	SpringArmComponent->bUsePawnControlRotation = true;

	// Setup camera.
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = true;

	// Setup character movement.
	CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bUseControllerDesiredRotation = false;
	CharacterMovementComponent->bOrientRotationToMovement     = true;

	// Setup walking particles.
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));

	// Setup reach hitbox.
	ReachComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("ReachBoxComponent"));
	ReachComponent->SetWorldScale3D(FVector(0.3f,0.3f,1.0f));
	ReachComponent->SetGenerateOverlapEvents(true);
	ReachComponent->SetupAttachment(RootComponent);
}

void ABrawlerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Update camera parameters.
	SpringArmComponent->CameraLagSpeed = CameraLag;
	SpringArmComponent->TargetArmLength = CameraDistance;

	// If the character is controller by AI, treat this character as an enemy.
	if (AEnemyAiController* Ai = Cast<AEnemyAiController>(GetController()))
	{
		CharacterIsPlayer = false;
		Health = EnemyMaxHealth;
		Tags.Add("Enemy");

		// Force the enemy to always face the player.
		CharacterMovementComponent->bUseControllerDesiredRotation = true;
		CharacterMovementComponent->bOrientRotationToMovement     = false;
		Ai->SetFocus(Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)), EAIFocusPriority::Gameplay);

		// Remove the camera and spring arm.
		GetComponentByClass(UCameraComponent   ::StaticClass())->DestroyComponent();
		GetComponentByClass(USpringArmComponent::StaticClass())->DestroyComponent();

		// Change the movement speed.
		GetCharacterMovement()->MaxWalkSpeed = 500;

		// Give a knife to the enemy.
		if (EnemyDefaultWeapon)
		{
			AKnifeActor* Knife = Cast<AKnifeActor>(GetWorld()->SpawnActor(EnemyDefaultWeapon));
			Knife->GetPickedUp(this);
		}
	}

	// If the character is controlled by the player, treat this character as a player.
	else
	{
		Health = PlayerMaxHealth;
		Tags.Add("Player");
		PlayerName == FString("Undefined") ? PlayerName = FString("Player") : PlayerName;

		// Give a knife to the player.
		if (PlayerDefaultWeapon)
		{
			KnifeActor = Cast<AKnifeActor>(GetWorld()->SpawnActor(PlayerDefaultWeapon));
			KnifeActor->GetPickedUp(this);
		}
	}
}

void ABrawlerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateWalkingFX();

	// Decrement timers.
	if (InvincibilityTimer > 0) InvincibilityTimer -= DeltaSeconds;
	if (AttackTimer > 0) AttackTimer -= DeltaSeconds;

	if(IsDefending()) DefendEvent();
}

void ABrawlerCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	// Movement.
	NewInputComponent->BindAxis("MoveForward", this, &ABrawlerCharacter::MoveForward);
	NewInputComponent->BindAxis("MoveRight", this, &ABrawlerCharacter::MoveRight);

	// Camera.
	NewInputComponent->BindAxis("LookRight", this, &ABrawlerCharacter::AddControllerYawInput);
	NewInputComponent->BindAxis("LookUp", this, &ABrawlerCharacter::AddControllerPitchInput);

	// Attack key.
	NewInputComponent->BindAction("Attack", IE_Pressed, this, &ABrawlerCharacter::AttackEvent);
	
	// Defend key.
	NewInputComponent->BindAction("Defend", IE_Pressed,  this, &ABrawlerCharacter::StartDefendingEvent);
	NewInputComponent->BindAction("Defend", IE_Released, this, &ABrawlerCharacter::StopDefendingEvent);
	
	// Drop Key.
	NewInputComponent->BindAction("Drop", IE_Pressed, this, &ABrawlerCharacter::DropWeaponEvent);

	// Enemy counter test key;
	NewInputComponent->BindAction("TestKey", IE_Pressed, this, &ABrawlerCharacter::EnemyKilledEvent);
	
	// Take Damage test key.
	/*{
		FInputActionBinding TakeDamageBinding("TestKey", IE_Pressed);
		TakeDamageBinding.ActionDelegate.GetDelegateForManualSet().BindLambda([this] { TakeDamageEvent(1); });
		NewInputComponent->AddActionBinding(TakeDamageBinding);
	}*/
}


void ABrawlerCharacter::MoveForward(const float Amount)
{
	if(IsDead()) return;
	if(IsAttacking()) return;
	if(IsDefending()) return;
	
	const FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X)
							+ FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Z);
	AddMovementInput(Direction, Amount);
}

void ABrawlerCharacter::MoveRight(const float Amount)
{
	if(IsDead()) return;
	if(IsAttacking()) return;
	if(IsDefending()) return;
	
	const FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, Amount);
}

void ABrawlerCharacter::UpdateWalkingFX() const
{
	if (IsDead()) return;
	
	const bool ShouldShowFX = GetVelocity() != FVector(0, 0, 0) && !CharacterMovementComponent->IsFalling();
	
	if (ShouldShowFX && ParticleSystemComponent->bWasDeactivated)
		ParticleSystemComponent->ActivateSystem();
	else if (!ShouldShowFX && !ParticleSystemComponent->bWasDeactivated)
		ParticleSystemComponent->DeactivateSystem();
}

#pragma region Player Events
void ABrawlerCharacter::TakeDamageEvent(const int& Amount)
{
	if (IsDead()) return;
	if (IsInvincible()) return;
	
	Health -= Amount;
	UNiagaraFunctionLibrary::SpawnSystemAttached(BloodSplatterEffect, this->RootComponent, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
	
	if (IsDead()) {
		DeathEvent();
		if(IsEnemy()) EnemyKilledEvent(); // if enemy was killed, means player killed him
		DebugWarning("%s is dead!", *GetPlayerName());
	}
	else {
		InvincibilityEvent();
		DebugInfo("%s hit, lost %d HP, %d HP remaining", *GetPlayerName(), Amount, Health);
	}
}

void ABrawlerCharacter::AttackEvent()
{
	if(IsDead()) return;
	if(IsDefending()) return;
	
	AttackTimer = AttackDuration;

	if(TargetActor == nullptr) return;
	
	ABrawlerCharacter* EnemyBrawler = Cast<ABrawlerCharacter>(TargetActor);
	DebugInfo("%s is attacking!", *GetPlayerName());
	EnemyBrawler->TakeDamageEvent(1);
}

void ABrawlerCharacter::DefendEvent()
{
	if(IsDead()) return;

	DebugInfo("%s is defending!", *GetPlayerName());
}


// Detect collision between actor hitbox and other actor
void ABrawlerCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if(!OtherActor->IsA(StaticClass())) return;
	
	if(Cast<ABrawlerCharacter>(OtherActor)->IsEnemy() && ReachComponent->IsOverlappingActor(OtherActor))
	{
		// storing latest in range target
		TargetActor = OtherActor;
	}
}

// Detect if no more collision between actor hitbox and other actor
void ABrawlerCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	if(OtherActor == TargetActor && !ReachComponent->IsOverlappingActor(TargetActor))
	{
		// removing target, not in range anymore
		TargetActor = nullptr;
	}
}


void ABrawlerCharacter::StartDefendingEvent()
{
	Defending = true;
}

void ABrawlerCharacter::StopDefendingEvent()
{
	Defending = false;
}

void ABrawlerCharacter::InvincibilityEvent()
{
	InvincibilityTimer = InvincibilityDuration;
}

void ABrawlerCharacter::DeathEvent()
{
	Health = 0;
	ParticleSystemComponent->DeactivateSystem();
	
	if (Controller && IsEnemy()) Controller->UnPossess();
}

void ABrawlerCharacter::EnemyKilledEvent()
{
	KillCount++;
	DebugData("KillCount %d: ", KillCount);

	if(!GameMode) return;
	
	AUnrealBrawlerGameModeBase* BrawlerMode = Cast<AUnrealBrawlerGameModeBase>(GameMode);
	UGameHUD* HUD = BrawlerMode->GetGameHUD();
	if(HUD) HUD->UpdateCounterEvent(FString::FromInt(KillCount));
}

void ABrawlerCharacter::DropWeaponEvent()
{
	if(KnifeActor == nullptr) return;
	
	KnifeActor->DropPickedUp(this);
}
#pragma endregion

#pragma region Getter & Setter
int ABrawlerCharacter::IsPlayer() const
{
	return CharacterIsPlayer;
}

int ABrawlerCharacter::IsEnemy() const
{
	return !CharacterIsPlayer;
}

int ABrawlerCharacter::GetHealth() const
{
	return Health;
}

int ABrawlerCharacter::GetAttackDamage() const
{
	return AttackDamage;
}

int ABrawlerCharacter::GetKillCount() const
{
	return KillCount;
}

bool ABrawlerCharacter::IsAttacking() const
{
	return AttackTimer > 0;
}

bool ABrawlerCharacter::IsDefending() const
{
	return Defending;
}

bool ABrawlerCharacter::IsInvincible() const
{
	return InvincibilityTimer > 0;
}

bool ABrawlerCharacter::IsDead() const
{
	return Health <= 0;
}

void ABrawlerCharacter::SetWeaponActor(AKnifeActor* KnifeWeapon)
{
	KnifeActor = KnifeWeapon;
}

FString ABrawlerCharacter::GetPlayerName() const
{
	return PlayerName;
}
#pragma endregion
