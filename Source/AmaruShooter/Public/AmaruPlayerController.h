// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "AmaruPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMARUSHOOTER_API AAmaruPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> HUDInkaClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUserWidget* HUDInka;

	UFUNCTION()
	void CreateHUDInka();

public:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;
};
