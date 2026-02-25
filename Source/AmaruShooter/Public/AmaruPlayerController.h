// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "AmaruPlayerController.generated.h"

class UInkaDataAsset;

UCLASS()
class AMARUSHOOTER_API AAmaruPlayerController : public APlayerController
{
	GENERATED_BODY()


    template <typename ParamStruct = void>
    static bool CallBPFunction(UObject* Target, const FName& FuncName, ParamStruct* Params = nullptr)
    {
        if (!Target)
        {
            UE_LOG(LogTemp, Warning, TEXT("Target is null for %s"), *FuncName.ToString());
            return false;
        }

        if (UFunction* Func = Target->FindFunction(FuncName))
        {
            Target->ProcessEvent(Func, Params);
            return true;
        }

        UE_LOG(LogTemp, Warning, TEXT("%s not implemented in %s"),
            *FuncName.ToString(), *Target->GetName());
        return false;
    }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> HUDInkaClass;

	UPROPERTY(Transient, BlueprintReadOnly)
	UUserWidget* HUDInka;

	UFUNCTION()
	void CreateHUDInka();

	UFUNCTION(Server, Reliable)
    void Server_SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka);

    UFUNCTION()
    void TryBindHUDToASC();

public:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_PlayerState() override;
};
