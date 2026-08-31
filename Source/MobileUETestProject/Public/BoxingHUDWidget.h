// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PowerBarMath.h"
#include "BoxingHUDWidget.generated.h"

class UButton;
class UProgressBar;
class UTextBlock;
class UBorder;
class UCanvasPanel;
class UVerticalBox;
class AFightDirector;
class ABoxerCharacter;

// Self-building boxing HUD: constructs its own UMG layout in C++ (RebuildWidget), mirroring
// UMazeControlWidget - no UMG Designer work required. Assign this class (or a Blueprint child)
// to AFightDirector::HUDWidgetClass and it just works.
UCLASS()
class MOBILEUETESTPROJECT_API UBoxingHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boxing")
	void SetFightDirector(AFightDirector* NewFightDirector);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing")
	TObjectPtr<AFightDirector> FightDirector;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> PlayerHPBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerHPText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> OpponentHPBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpponentHPText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> AttackButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> GuardButton;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> AttackTrack;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AttackMarker;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackStateLabel;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> DefenseTrack;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DefenseMarker;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GuardStateLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BannerText;

	UFUNCTION()
	void HandleAttackPressed();

	UFUNCTION()
	void HandleAttackReleased();

	UFUNCTION()
	void HandleGuardPressed();

	UFUNCTION()
	void HandleGuardReleased();

private:
	UCanvasPanel* BuildPowerBarTrack(UVerticalBox* Parent, TObjectPtr<UBorder>& OutMarker, const FString& Label, TObjectPtr<UTextBlock>& OutStateLabel);
	void PaintZones(UCanvasPanel* Track, const FPowerBarZones& Zones);
	void UpdateBar(ABoxerCharacter* Boxer, UCanvasPanel* Track, UBorder* Marker, UTextBlock* StateLabel, FPowerBarZones& LastPaintedZones);
	void BindWidgetEvents();

	FPowerBarZones LastAttackZones;
	FPowerBarZones LastDefenseZones;
};
