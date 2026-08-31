// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxingHUDWidget.h"
#include "FightDirector.h"
#include "BoxerCharacter.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

TSharedRef<SWidget> UBoxingHUDWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	// Player HP (top-left)
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PlayerHPPanel"));
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.75f));
		Panel->SetPadding(FMargin(10.0f));
		UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		PanelSlot->SetPosition(FVector2D(24.0f, 24.0f));
		PanelSlot->SetAutoSize(false);
		PanelSlot->SetSize(FVector2D(280.0f, 60.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PlayerHPBox"));
		Panel->SetContent(Box);

		PlayerHPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayerHPText"));
		Box->AddChildToVerticalBox(PlayerHPText);

		PlayerHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PlayerHPBar"));
		PlayerHPBar->SetFillColorAndOpacity(FLinearColor(0.3f, 0.69f, 0.42f));
		UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(PlayerHPBar);
		BarSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	// Opponent HP (top-right)
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OpponentHPPanel"));
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.75f));
		Panel->SetPadding(FMargin(10.0f));
		UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		PanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		PanelSlot->SetPosition(FVector2D(-24.0f, 24.0f));
		PanelSlot->SetAutoSize(false);
		PanelSlot->SetSize(FVector2D(280.0f, 60.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OpponentHPBox"));
		Panel->SetContent(Box);

		OpponentHPText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OpponentHPText"));
		Box->AddChildToVerticalBox(OpponentHPText);

		OpponentHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("OpponentHPBar"));
		OpponentHPBar->SetFillColorAndOpacity(FLinearColor(0.79f, 0.29f, 0.29f));
		UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(OpponentHPBar);
		BarSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	// Attack panel (bottom-left)
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AttackPanel"));
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.75f));
		Panel->SetPadding(FMargin(12.0f));
		UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(0.0f, 1.0f));
		PanelSlot->SetAlignment(FVector2D(0.0f, 1.0f));
		PanelSlot->SetPosition(FVector2D(24.0f, -24.0f));
		PanelSlot->SetAutoSize(false);
		PanelSlot->SetSize(FVector2D(300.0f, 110.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AttackBox"));
		Panel->SetContent(Box);

		AttackTrack = BuildPowerBarTrack(Box, AttackMarker, TEXT("Attack"), AttackStateLabel);

		AttackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("AttackButton"));
		UTextBlock* AttackButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AttackButtonText"));
		AttackButtonText->SetText(FText::FromString(TEXT("HOLD to punch")));
		AttackButtonText->SetJustification(ETextJustify::Center);
		AttackButton->SetContent(AttackButtonText);
		UVerticalBoxSlot* AttackButtonSlot = Box->AddChildToVerticalBox(AttackButton);
		AttackButtonSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
		AttackButtonSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// Defense panel (bottom-right)
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DefensePanel"));
		Panel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.75f));
		Panel->SetPadding(FMargin(12.0f));
		UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
		PanelSlot->SetAnchors(FAnchors(1.0f, 1.0f));
		PanelSlot->SetAlignment(FVector2D(1.0f, 1.0f));
		PanelSlot->SetPosition(FVector2D(-24.0f, -24.0f));
		PanelSlot->SetAutoSize(false);
		PanelSlot->SetSize(FVector2D(300.0f, 110.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DefenseBox"));
		Panel->SetContent(Box);

		DefenseTrack = BuildPowerBarTrack(Box, DefenseMarker, TEXT("Guard"), GuardStateLabel);

		GuardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuardButton"));
		UTextBlock* GuardButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GuardButtonText"));
		GuardButtonText->SetText(FText::FromString(TEXT("HOLD to guard")));
		GuardButtonText->SetJustification(ETextJustify::Center);
		GuardButton->SetContent(GuardButtonText);
		UVerticalBoxSlot* GuardButtonSlot = Box->AddChildToVerticalBox(GuardButton);
		GuardButtonSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
		GuardButtonSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	// Win/Lose banner (center) - design doc section 4's Victory stand-in.
	{
		BannerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BannerText"));
		FSlateFontInfo Font = BannerText->GetFont();
		Font.Size = 48;
		BannerText->SetFont(Font);
		BannerText->SetJustification(ETextJustify::Center);
		BannerText->SetVisibility(ESlateVisibility::Collapsed);
		UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(BannerText);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.35f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetAutoSize(true);
	}

	// Paint initial (default baseline) zones so the tracks aren't blank before the first charge.
	PaintZones(AttackTrack, LastAttackZones);
	PaintZones(DefenseTrack, LastDefenseZones);

	BindWidgetEvents();

	return Super::RebuildWidget();
}

UCanvasPanel* UBoxingHUDWidget::BuildPowerBarTrack(UVerticalBox* Parent, TObjectPtr<UBorder>& OutMarker, const FString& Label, TObjectPtr<UTextBlock>& OutStateLabel)
{
	UTextBlock* StateLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(Label + TEXT("StateLabel")));
	StateLabel->SetText(FText::FromString(Label));
	UVerticalBoxSlot* StateLabelSlot = Parent->AddChildToVerticalBox(StateLabel);
	StateLabelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 2.0f));
	OutStateLabel = StateLabel;

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *(Label + TEXT("SizeBox")));
	SizeBox->SetWidthOverride(260.0f);
	SizeBox->SetHeightOverride(24.0f);
	Parent->AddChildToVerticalBox(SizeBox);

	UCanvasPanel* Track = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), *(Label + TEXT("Track")));
	SizeBox->SetContent(Track);

	UBorder* Marker = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *(Label + TEXT("Marker")));
	Marker->SetBrushColor(FLinearColor::White);
	UCanvasPanelSlot* MarkerSlot = Track->AddChildToCanvas(Marker);
	MarkerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
	MarkerSlot->SetOffsets(FMargin(-1.5f, 0.0f, 3.0f, 0.0f));
	OutMarker = Marker;

	return Track;
}

void UBoxingHUDWidget::PaintZones(UCanvasPanel* Track, const FPowerBarZones& Zones)
{
	if (!Track)
	{
		return;
	}

	UBorder* Marker = (Track == AttackTrack) ? AttackMarker.Get() : DefenseMarker.Get();

	// Remove previously painted zone segments (everything except the marker).
	for (int32 Index = Track->GetChildrenCount() - 1; Index >= 0; --Index)
	{
		UWidget* Child = Track->GetChildAt(Index);
		if (Child != Marker)
		{
			Track->RemoveChildAt(Index);
		}
	}

	const float Span = PowerBarSpan(Zones);
	const float Center = PowerBarCenter(Zones);
	if (Span <= 0.0f)
	{
		return;
	}

	auto AddSegment = [this, Track, Span](float FromRaw, float ToRaw, const FLinearColor& Color)
	{
		const float FromPct = FMath::Clamp(FromRaw / Span, 0.0f, 1.0f);
		const float ToPct = FMath::Clamp(ToRaw / Span, 0.0f, 1.0f);
		if (ToPct <= FromPct)
		{
			return;
		}

		UBorder* Segment = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Segment->SetBrushColor(Color);
		UCanvasPanelSlot* SegSlot = Track->AddChildToCanvas(Segment);
		SegSlot->SetAnchors(FAnchors(FromPct, 0.0f, ToPct, 1.0f));
		SegSlot->SetOffsets(FMargin(0.0f));
	};

	static const FLinearColor RedColor(0.79f, 0.29f, 0.29f);
	static const FLinearColor BlueColor(0.25f, 0.5f, 0.75f);
	static const FLinearColor GreenColor(0.3f, 0.69f, 0.42f);

	AddSegment(0.0f, Center - Zones.BlueOuter, RedColor);
	AddSegment(Center - Zones.BlueOuter, Center - Zones.GreenHalf, BlueColor);
	AddSegment(Center - Zones.GreenHalf, Center + Zones.GreenHalf, GreenColor);
	AddSegment(Center + Zones.GreenHalf, Center + Zones.BlueOuter, BlueColor);
	AddSegment(Center + Zones.BlueOuter, Span, RedColor);

	// Re-add the marker last so it renders on top of the freshly painted zone segments.
	if (Marker)
	{
		Track->RemoveChild(Marker);
		UCanvasPanelSlot* MarkerSlot = Track->AddChildToCanvas(Marker);
		MarkerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
		MarkerSlot->SetOffsets(FMargin(-1.5f, 0.0f, 3.0f, 0.0f));
	}
}

void UBoxingHUDWidget::UpdateBar(ABoxerCharacter* Boxer, UCanvasPanel* Track, UBorder* Marker, UTextBlock* StateLabel, FPowerBarZones& LastPaintedZones)
{
	if (!Boxer || !Track || !Marker)
	{
		return;
	}

	const FPowerBarZones CurrentZones = Boxer->GetActiveZones();
	if (CurrentZones.GreenHalf != LastPaintedZones.GreenHalf ||
		CurrentZones.BlueOuter != LastPaintedZones.BlueOuter ||
		CurrentZones.RedOuter != LastPaintedZones.RedOuter)
	{
		PaintZones(Track, CurrentZones);
		LastPaintedZones = CurrentZones;
	}

	const float NormalizedPos = Boxer->GetBarNormalizedPosition();
	if (UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot))
	{
		const float AnchorX = FMath::Clamp(NormalizedPos, 0.0f, 1.0f);
		MarkerSlot->SetAnchors(FAnchors(AnchorX, 0.0f, AnchorX, 1.0f));
		MarkerSlot->SetOffsets(FMargin(-1.5f, 0.0f, 3.0f, 0.0f));
	}

	if (StateLabel)
	{
		const float Power = ComputePower(NormalizedPos * PowerBarSpan(CurrentZones), CurrentZones);
		StateLabel->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Power)));
	}
}

void UBoxingHUDWidget::BindWidgetEvents()
{
	if (AttackButton)
	{
		AttackButton->OnPressed.AddUniqueDynamic(this, &UBoxingHUDWidget::HandleAttackPressed);
		AttackButton->OnReleased.AddUniqueDynamic(this, &UBoxingHUDWidget::HandleAttackReleased);
	}
	if (GuardButton)
	{
		GuardButton->OnPressed.AddUniqueDynamic(this, &UBoxingHUDWidget::HandleGuardPressed);
		GuardButton->OnReleased.AddUniqueDynamic(this, &UBoxingHUDWidget::HandleGuardReleased);
	}
}

void UBoxingHUDWidget::SetFightDirector(AFightDirector* NewFightDirector)
{
	FightDirector = NewFightDirector;
}

void UBoxingHUDWidget::HandleAttackPressed()
{
	if (FightDirector)
	{
		FightDirector->HandleAttackButtonPressed();
	}
}

void UBoxingHUDWidget::HandleAttackReleased()
{
	if (FightDirector)
	{
		FightDirector->HandleAttackButtonReleased();
	}
}

void UBoxingHUDWidget::HandleGuardPressed()
{
	if (FightDirector)
	{
		FightDirector->HandleGuardButtonPressed();
	}
}

void UBoxingHUDWidget::HandleGuardReleased()
{
	if (FightDirector)
	{
		FightDirector->HandleGuardButtonReleased();
	}
}

void UBoxingHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!FightDirector)
	{
		return;
	}

	ABoxerCharacter* Player = FightDirector->PlayerBoxer;
	ABoxerCharacter* Opponent = FightDirector->OpponentBoxer;

	if (Player)
	{
		if (PlayerHPBar)
		{
			PlayerHPBar->SetPercent(Player->MaxHP > 0.0f ? Player->CurrentHP / Player->MaxHP : 0.0f);
		}
		if (PlayerHPText)
		{
			PlayerHPText->SetText(FText::FromString(FString::Printf(TEXT("Player  %d / %d"), FMath::RoundToInt(Player->CurrentHP), FMath::RoundToInt(Player->MaxHP))));
		}
	}

	if (Opponent)
	{
		if (OpponentHPBar)
		{
			OpponentHPBar->SetPercent(Opponent->MaxHP > 0.0f ? Opponent->CurrentHP / Opponent->MaxHP : 0.0f);
		}
		if (OpponentHPText)
		{
			OpponentHPText->SetText(FText::FromString(FString::Printf(TEXT("Opponent  %d / %d"), FMath::RoundToInt(Opponent->CurrentHP), FMath::RoundToInt(Opponent->MaxHP))));
		}
	}

	const EFightState State = FightDirector->CurrentState;

	if (AttackButton)
	{
		AttackButton->SetIsEnabled(State == EFightState::PlayerAttack);
	}
	if (GuardButton)
	{
		GuardButton->SetIsEnabled(State == EFightState::PlayerDefend);
	}

	if (Player && Player->CurrentState == EBoxerState::ChargingAttack)
	{
		UpdateBar(Player, AttackTrack, AttackMarker, AttackStateLabel, LastAttackZones);
	}
	if (Player && Player->CurrentState == EBoxerState::ChargingGuard)
	{
		UpdateBar(Player, DefenseTrack, DefenseMarker, GuardStateLabel, LastDefenseZones);
	}

	if (BannerText)
	{
		if (State == EFightState::Win)
		{
			BannerText->SetText(FText::FromString(TEXT("WINNER!")));
			BannerText->SetVisibility(ESlateVisibility::Visible);
		}
		else if (State == EFightState::Lose)
		{
			BannerText->SetText(FText::FromString(TEXT("KO...")));
			BannerText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BannerText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
