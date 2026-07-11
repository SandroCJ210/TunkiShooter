// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmaruShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "AmaruPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruGameplayAbility.h"
#include "Enums.h"
#include "InkaDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AmaruAbilitySystemComponent.h"
#include "AmaruGameplayTags.h"
#include "GameModes/AmaruGameMode.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AAmaruShooterCharacter

AAmaruShooterCharacter::AAmaruShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(CameraBoom);
	ThirdPersonCameraComponent->bUsePawnControlRotation = true;

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
}

void AAmaruShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AAmaruShooterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// El ASC vive en el PlayerState y sobrevive al personaje (respawn):
	// si no desuscribimos aquí, las lambdas quedan apuntando a un 'this' destruido,
	// y si no limpiamos las habilidades, cada respawn las duplicaría en el ASC.
	Server_DisableAbilitiesForMode();
	UnbindAttributeDelegates();
	Super::EndPlay(EndPlayReason);
}

void AAmaruShooterCharacter::UnbindAttributeDelegates()
{
	UAbilitySystemComponent* ASC = BoundASC.Get();
	UAmaruAttributeSet* AS = BoundAS.Get();
	if (ASC && AS)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMoveSpeedAttribute()).Remove(MoveSpeedChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetChargeAbility1Attribute()).Remove(ChargeAbility1ChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetChargeAbility2Attribute()).Remove(ChargeAbility2ChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetAmmoAttribute()).Remove(AmmoChangedHandle);
	}
	if (ASC)
	{
		ASC->RegisterGameplayTagEvent(AmaruTags::State_Stealth, EGameplayTagEventType::NewOrRemoved).Remove(StealthTagChangedHandle);
	}
	MoveSpeedChangedHandle.Reset();
	ChargeAbility1ChangedHandle.Reset();
	ChargeAbility2ChangedHandle.Reset();
	AmmoChangedHandle.Reset();
	StealthTagChangedHandle.Reset();
	BoundASC = nullptr;
	BoundAS = nullptr;
}

void AAmaruShooterCharacter::OnStealthTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bStealthed = NewCount > 0;
	if (IsLocallyControlled())
	{
		OnStealthChanged(bStealthed);
	}
	else if (Mesh3P)
	{
		Mesh3P->SetVisibility(!bStealthed, true);
	}
}

void AAmaruShooterCharacter::Multicast_OnDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	if (Mesh3P)
	{
		Mesh3P->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		Mesh3P->SetSimulatePhysics(true);
	}
}

UAbilitySystemComponent* AAmaruShooterCharacter::GetAbilitySystemComponent() const
{
	return CachedASC;
}

UAmaruAttributeSet* AAmaruShooterCharacter::GetAmaruAttributeSet() const
{
	return CachedPS ? CachedPS->GetAttributeSet() : nullptr;
}

void AAmaruShooterCharacter::Server_EnableAbilitiesForMode()
{
	if (!HasAuthority()) return;

	if (!CachedPS || !CachedASC)
	{
		InitAbilityActorInfo();
	}

	if (!InkaDefinition)
	{
		return;
	}

	ClearGrantedAbilities();
	ApplyStartupEffectsFromDefinition();
	GiveAbilitiesFromDefinition();
}

void AAmaruShooterCharacter::Server_DisableAbilitiesForMode()
{
	if (!HasAuthority()) return;
	RemoveStartupEffects();
	ClearGrantedAbilities();
}

void AAmaruShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
}

void AAmaruShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AAmaruShooterCharacter::RefreshInkaDefinition(int32 PlayerIndex)
{
	if (!CachedPS) return;

	if (CachedPS->GetPlayerId() != PlayerIndex)
	{
		return;
	}

	const bool bInkaNull = CachedPS->SelectedInka.IsNull();

	if (!bInkaNull)
	{
		InkaDefinition = CachedPS->SelectedInka.LoadSynchronous();
	}
	else
	{
		InkaDefinition = nullptr;
	}

	if (HasAuthority())
	{
		Server_EnableAbilitiesForMode();
	}

	if (IsLocallyControlled())
	{
		OnAbilityLoadoutChanged.Broadcast();
	}
}

void AAmaruShooterCharacter::InitAbilityActorInfo()
{
	AAmaruPlayerState* OldPS = CachedPS;
	CachedPS = GetPlayerState<AAmaruPlayerState>();
	
	if (CachedPS && CachedPS != OldPS)
	{
		CachedPS->OnInkaChanged.AddUniqueDynamic(this, &AAmaruShooterCharacter::RefreshInkaDefinition);
	}

	if (!CachedPS)
	{
		CachedASC = nullptr;
		return;
	}

	const bool bInkaNull = CachedPS->SelectedInka.IsNull();

	if (!bInkaNull)
	{
		InkaDefinition = CachedPS->SelectedInka.LoadSynchronous();
	}
	else
	{
		InkaDefinition = nullptr;
	}

	CachedASC = CachedPS->GetAmaruAbilitySystemComponent();

	if (!CachedASC)
	{
		return;
	}

	CachedASC->InitAbilityActorInfo(CachedPS, this);

	if (!StealthTagChangedHandle.IsValid())
	{
		StealthTagChangedHandle = CachedASC->RegisterGameplayTagEvent(AmaruTags::State_Stealth, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &AAmaruShooterCharacter::OnStealthTagChanged);
	}

	if (UAmaruAttributeSet* AS = CachedPS->GetAttributeSet())
	{
		GetCharacterMovement()->MaxWalkSpeed = AS->GetMoveSpeed();

		const bool bASCChanged = BoundASC.Get() != CachedASC;
		const bool bASChanged  = BoundAS.Get() != AS;

	if ((bASCChanged || bASChanged))
	{
		// Desuscribir del ASC anterior antes de resetear los handles,
		// si no las lambdas viejas siguen vivas en el otro ASC.
		UnbindAttributeDelegates();

		BoundASC = CachedASC;
		BoundAS  = AS;
	}

	if (!MoveSpeedChangedHandle.IsValid())
	{
		MoveSpeedChangedHandle =
			CachedASC->GetGameplayAttributeValueChangeDelegate(AS->GetMoveSpeedAttribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (UCharacterMovementComponent* Move = GetCharacterMovement())
				{
					Move->MaxWalkSpeed = Data.NewValue;
				}
			});
	}


	if (!ChargeAbility1ChangedHandle.IsValid())
	{
		ChargeAbility1ChangedHandle =
			CachedASC->GetGameplayAttributeValueChangeDelegate(AS->GetChargeAbility1Attribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (HasAuthority())
				{
					if (Data.NewValue < Data.OldValue)
					{
						if (InkaDefinition && InkaDefinition->Abilities.IsValidIndex(0) && InkaDefinition->Abilities[0].CooldownEffect)
						{
							CachedASC->BP_ApplyGameplayEffectToSelf(InkaDefinition->Abilities[0].CooldownEffect, 0.f, FGameplayEffectContextHandle());

						}
					}
					else
					{

						FGameplayTagContainer TagsToRemove;
						TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Ability1.Recharging")));
						CachedASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
					}
				}
				if (IsLocallyControlled())
				{
					OnChargeAbilityChanged.Broadcast(EAmaruAbilityInputID::Ability1, Data.NewValue);
				}
			});
	}

	if (!ChargeAbility2ChangedHandle.IsValid())
	{
		ChargeAbility2ChangedHandle =
			CachedASC->GetGameplayAttributeValueChangeDelegate(AS->GetChargeAbility2Attribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (HasAuthority())
				{
					if (Data.NewValue < Data.OldValue)
					{
						if (InkaDefinition && InkaDefinition->Abilities.IsValidIndex(1) && InkaDefinition->Abilities[1].CooldownEffect)
						{
							CachedASC->BP_ApplyGameplayEffectToSelf(InkaDefinition->Abilities[1].CooldownEffect, 0.f, FGameplayEffectContextHandle());
						}
					}
					else
					{

						FGameplayTagContainer TagsToRemove;
						TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Ability2.Recharging")));
						CachedASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
					}
				}
				if (IsLocallyControlled())
				{
					OnChargeAbilityChanged.Broadcast(EAmaruAbilityInputID::Ability2, Data.NewValue);
				}
			});
	}

	if (!AmmoChangedHandle.IsValid())
	{
		AmmoChangedHandle =
			CachedASC->GetGameplayAttributeValueChangeDelegate(AS->GetAmmoAttribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (!IsLocallyControlled()) return;
				if (Data.NewValue > 0.f) return;
				if (!CachedASC) return;

				const FGameplayTag ReloadingTag = FGameplayTag::RequestGameplayTag(FName("Status.Reloading"));
				if (CachedASC->HasMatchingGameplayTag(ReloadingTag)) return;

				CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Reload);
				CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Reload);
			});
	}
	}
	if (IsLocallyControlled())
	{
		OnAbilityLoadoutChanged.Broadcast();
	}
}

void AAmaruShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::OnJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::OnJumpCompleted);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAmaruShooterCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAmaruShooterCharacter::Look);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::Shoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::StopShooting);

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::ReloadPressed);
		}

		EnhancedInputComponent->BindAction(Ability1Action, ETriggerEvent::Started, this, &AAmaruShooterCharacter::Ability1Pressed);
		EnhancedInputComponent->BindAction(Ability1Action, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::Ability1Released);
		EnhancedInputComponent->BindAction(Ability1Action, ETriggerEvent::Canceled, this, &AAmaruShooterCharacter::Ability1Canceled);

		EnhancedInputComponent->BindAction(Ability2Action, ETriggerEvent::Started, this, &AAmaruShooterCharacter::Ability2Pressed);
		EnhancedInputComponent->BindAction(Ability2Action, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::Ability2Released);
		EnhancedInputComponent->BindAction(Ability2Action, ETriggerEvent::Canceled, this, &AAmaruShooterCharacter::Ability2Canceled);

		EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::UltimatePressed);
		EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::UltimateReleased);
		EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Canceled, this, &AAmaruShooterCharacter::UltimateCanceled);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AAmaruShooterCharacter::GiveAbilitiesFromDefinition()
{
	if (!HasAuthority()) return;
	if (!InkaDefinition) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (GrantedAbilityHandles.Num() > 0) return;

	// Deathmatch: solo arma primaria y recarga, sin kit de héroe.
	bool bWeaponsOnly = false;
	if (const AAmaruGameMode* GM = GetWorld()->GetAuthGameMode<AAmaruGameMode>())
	{
		bWeaponsOnly = !GM->bHeroAbilitiesEnabled;
	}

	for (const FAmaruGrantedAbility& Entry : InkaDefinition->Abilities)
	{
		if (!Entry.AbilityClass) continue;

		if (bWeaponsOnly &&
			Entry.InputID != static_cast<int32>(EAmaruAbilityInputID::PrimaryFire) &&
			Entry.InputID != static_cast<int32>(EAmaruAbilityInputID::Reload))
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.Level, Entry.InputID, this);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
	}
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPlayerState(), FGameplayTag::RequestGameplayTag(FName("Event.GrantedAbility")), FGameplayEventData());
	if (EquipWeaponAbility)
	{
		FGameplayAbilitySpec Spec(EquipWeaponAbility, 1, -1, this);
		EquipWeaponHandle = ASC->GiveAbility(Spec);
	}
	FGameplayEventData EventDataWeapon;
	EventDataWeapon.TargetTags = FGameplayTagContainer{ InkaDefinition->WeaponTag };
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPlayerState(), FGameplayTag::RequestGameplayTag(FName("Event.Weapon.Equip")), EventDataWeapon);
}

void AAmaruShooterCharacter::ApplyStartupEffectsFromDefinition()
{
	if (!HasAuthority()) return;
	if (!InkaDefinition) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	RemoveStartupEffects();

	for (const auto EffectClass : InkaDefinition->StartupEffects)
	{
		if (!EffectClass) continue;

		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		Ctx.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.f, Ctx);
		if (SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle ActiveHandle =
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			StartupEffectHandles.Add(ActiveHandle);
		}
	}

}

void AAmaruShooterCharacter::RemoveStartupEffects()
{
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	for (const FActiveGameplayEffectHandle& H : StartupEffectHandles)
	{
		if (H.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(H);
		}
	}
	StartupEffectHandles.Reset();
}

void AAmaruShooterCharacter::ClearGrantedAbilities()
{
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		ASC->ClearAbility(Handle);
	}
	GrantedAbilityHandles.Reset();
	if (EquipWeaponHandle.IsValid())
	{
		ASC->ClearAbility(EquipWeaponHandle);
		EquipWeaponHandle = FGameplayAbilitySpecHandle();
	}
}

void AAmaruShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AAmaruShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAmaruShooterCharacter::OnJumpStarted(const FInputActionValue& Value)
{
	Jump();
}

void AAmaruShooterCharacter::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}

void AAmaruShooterCharacter::Shoot(const FInputActionValue& Value)
{
	if (!CachedASC)
	{
		InitAbilityActorInfo();
	}
	if (!CachedASC) return;

	if (const UAmaruAttributeSet* AS = GetAmaruAttributeSet())
	{
		if (AS->GetAmmo() <= 0.f)
		{
			CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Reload);
			CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Reload);
			return;
		}
	}

	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::PrimaryFire);
}

void AAmaruShooterCharacter::StopShooting(const FInputActionValue& Value)
{
	if (!CachedASC)
	{
		InitAbilityActorInfo();
	}
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::PrimaryFire);
}

void AAmaruShooterCharacter::ReloadPressed(const FInputActionValue& Value)
{
	if (!CachedASC)
	{
		InitAbilityActorInfo();
	}
	if (!CachedASC) return;

	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Reload);
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Reload);
}

void AAmaruShooterCharacter::Ability1Pressed(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ability1);
}
void AAmaruShooterCharacter::Ability1Released(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability1);
}
void AAmaruShooterCharacter::Ability1Canceled(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability1);
}

void AAmaruShooterCharacter::Ability2Pressed(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ability2);
}
void AAmaruShooterCharacter::Ability2Released(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability2);
}
void AAmaruShooterCharacter::Ability2Canceled(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability2);
}

void AAmaruShooterCharacter::UltimatePressed(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ultimate);

}
void AAmaruShooterCharacter::UltimateReleased(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ultimate);
}
void AAmaruShooterCharacter::UltimateCanceled(const FInputActionValue& Value)
{
	if (!CachedASC) return;
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ultimate);
}
