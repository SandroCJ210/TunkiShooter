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
#include "AmaruAbilitySystemComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);
static void PrintInputDebug(const UObject* WorldContext, const FString& Msg)
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("%s"), *Msg);

	if (GEngine && WorldContext)
	{
		if (const UWorld* World = WorldContext->GetWorld())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Msg);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// AAmaruShooterCharacter

AAmaruShooterCharacter::AAmaruShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

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
	ApplyStartupEffectsFromDefinition();
	GiveAbilitiesFromDefinition();
}

void AAmaruShooterCharacter::Server_DisableAbilitiesForMode()
{
	if (!HasAuthority()) return;
	ApplyStartupEffectsFromDefinition();
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

void AAmaruShooterCharacter::InitAbilityActorInfo()
{
	CachedPS = GetPlayerState<AAmaruPlayerState>();
	if (!CachedPS)
	{
		CachedASC = nullptr;
		return;
	}

	CachedASC = CachedPS->GetAmaruAbilitySystemComponent();
	if (!CachedASC)
	{
		return;
	}

	CachedASC->InitAbilityActorInfo(CachedPS, this);
	CachedASC->SetReplicationMode(
		IsLocallyControlled() ? EGameplayEffectReplicationMode::Mixed : EGameplayEffectReplicationMode::Minimal
	);

	if (UAmaruAttributeSet* AS = CachedPS->GetAttributeSet())
	{
		GetCharacterMovement()->MaxWalkSpeed = AS->GetMoveSpeed();

		const bool bASCChanged = BoundASC.Get() != CachedASC;
		const bool bASChanged  = BoundAS.Get() != AS;

	if ((bASCChanged || bASChanged))
	{
		MoveSpeedChangedHandle.Reset();
		ChargeAbility1ChangedHandle.Reset();
		ChargeAbility2ChangedHandle.Reset();

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
				if (Data.NewValue < Data.OldValue)
				{
					CachedASC->BP_ApplyGameplayEffectToSelf(InkaDefinition->Abilities[0].CooldownEffect,0.f, FGameplayEffectContextHandle());
				}else
				{
					if (Data.NewValue >= GetAmaruAttributeSet()->GetChargeAbility1())
					{
						FGameplayTagContainer TagsToRemove;
						TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Ability1.Recharging")));
						CachedASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
					}
				}
				if (IsLocallyControlled())
				{
					OnChargeAbility1Changed.Broadcast(EAmaruAbilityInputID::Ability1, Data.NewValue);
				}
			});
	}

	if (!ChargeAbility2ChangedHandle.IsValid())
	{
		ChargeAbility2ChangedHandle =
			CachedASC->GetGameplayAttributeValueChangeDelegate(AS->GetChargeAbility2Attribute())
			.AddLambda([this](const FOnAttributeChangeData& Data)
			{
				if (Data.NewValue < Data.OldValue)
				{
					CachedASC->BP_ApplyGameplayEffectToSelf(InkaDefinition->Abilities[1].CooldownEffect, 0.f, FGameplayEffectContextHandle());
				}
				else
				{
					if (Data.NewValue >= GetAmaruAttributeSet()->GetChargeAbility2())
					{
						FGameplayTagContainer TagsToRemove;
						TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Ability2.Recharging")));
						CachedASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
					}
				}
				if (IsLocallyControlled())
				{
					OnChargeAbility1Changed.Broadcast(EAmaruAbilityInputID::Ability2, Data.NewValue);
				}
			});
	}
	}
	if (IsLocallyControlled())
	{
		OnAbilityLoadoutChanged.Broadcast();
	}
}


void AAmaruShooterCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AAmaruShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::OnJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::OnJumpCompleted);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAmaruShooterCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAmaruShooterCharacter::Look);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AAmaruShooterCharacter::Shoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AAmaruShooterCharacter::StopShooting);

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

	for (const FAmaruGrantedAbility& Entry : InkaDefinition->Abilities)
	{
		if (!Entry.AbilityClass) continue;

		FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.Level, Entry.InputID, this);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
	}
}

void AAmaruShooterCharacter::ApplyStartupEffectsFromDefinition()
{
	if (!HasAuthority()) return;
	if (!InkaDefinition) return;

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
}

void AAmaruShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
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

	if (Controller != nullptr)
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
}

void AAmaruShooterCharacter::StopShooting(const FInputActionValue& Value)
{
}

void AAmaruShooterCharacter::Ability1Pressed(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ability1);
}
void AAmaruShooterCharacter::Ability1Released(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability1);
}
void AAmaruShooterCharacter::Ability1Canceled(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability1);
}

void AAmaruShooterCharacter::Ability2Pressed(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ability2);
}
void AAmaruShooterCharacter::Ability2Released(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability2);
}
void AAmaruShooterCharacter::Ability2Canceled(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ability2);
}

void AAmaruShooterCharacter::UltimatePressed(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ultimate);

}
void AAmaruShooterCharacter::UltimateReleased(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputPressed(EAmaruAbilityInputID::Ultimate);
}
void AAmaruShooterCharacter::UltimateCanceled(const FInputActionValue& Value)
{
	CachedASC->HandleAbilityLocalInputReleased(EAmaruAbilityInputID::Ultimate);
}