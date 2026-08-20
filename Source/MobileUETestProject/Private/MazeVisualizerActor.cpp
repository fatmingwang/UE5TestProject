// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeVisualizerActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"
#include "MazeControlWidget.h"
#include "MazeMinimapWidget.h"
#include "StoreButtonWidget.h"
#include "Blueprint/UserWidget.h"

AMazeVisualizerActor::AMazeVisualizerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MazeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MazeRoot"));
	SetRootComponent(MazeRoot);

	FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
	FloorInstances->SetupAttachment(MazeRoot);
	FloorInstances->SetMobility(EComponentMobility::Static);

	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
	WallInstances->SetupAttachment(MazeRoot);
	WallInstances->SetMobility(EComponentMobility::Static);

	MazeGenerator = CreateDefaultSubobject<UWilsonMazeGenerator>(TEXT("MazeGenerator"));

	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
	MinimapCapture->SetupAttachment(MazeRoot);
	MinimapCapture->ProjectionType = ECameraProjectionMode::Orthographic;
	MinimapCapture->CaptureSource = SCS_FinalColorLDR;
	MinimapCapture->bCaptureEveryFrame = false;
	MinimapCapture->bCaptureOnMovement = false;
	MinimapCapture->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

void AMazeVisualizerActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoPlayOnBeginPlay)
	{
		PlayState = EMazePlayState::Stopped;
		Play();
	}

	if (bAutoCreateControlWidget && ControlWidgetClass)
	{
		ControlWidgetInstance = CreateWidget<UMazeControlWidget>(GetWorld(), ControlWidgetClass);
		if (ControlWidgetInstance)
		{
			ControlWidgetInstance->SetMazeActor(this);
			ControlWidgetInstance->AddToViewport();
		}
	}

	if (bAutoCreateMinimapWidget && MinimapWidgetClass)
	{
		MinimapWidgetInstance = CreateWidget<UMazeMinimapWidget>(GetWorld(), MinimapWidgetClass);
		if (MinimapWidgetInstance)
		{
			MinimapWidgetInstance->SetMazeActor(this);
			MinimapWidgetInstance->AddToViewport();
		}
	}

	if (bAutoCreateStoreButton)
	{
		TSubclassOf<UStoreButtonWidget> ClassToUse = StoreButtonWidgetClass;
		if (!ClassToUse)
		{
			ClassToUse = UStoreButtonWidget::StaticClass();
		}
		StoreButtonWidgetInstance = CreateWidget<UStoreButtonWidget>(GetWorld(), ClassToUse);
		if (StoreButtonWidgetInstance)
		{
			StoreButtonWidgetInstance->AddToViewport();
		}
	}
}

void AMazeVisualizerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FloorInstances->SetStaticMesh(FloorMesh);
	WallInstances->SetStaticMesh(WallMesh);

	MazeGenerator->GenerateInstant();
	BuildFloorGrid();
	RebuildWalls();
}

void AMazeVisualizerActor::Play()
{
	if (PlayState == EMazePlayState::Playing || PlayState == EMazePlayState::Completed)
	{
		return;
	}

	if (PlayState == EMazePlayState::Stopped || MazeGenerator->GetTotalCellCount() == 0)
	{
		MazeGenerator->ResetMaze();
		BuildFloorGrid();
		RebuildWalls();
	}

	SetPlayState(EMazePlayState::Playing);

	if (StepInterval <= 0.0f)
	{
		GenerateInstantly();
		return;
	}

	GetWorldTimerManager().SetTimer(StepTimerHandle, this, &AMazeVisualizerActor::TickStep, StepInterval, true);
}

void AMazeVisualizerActor::Pause()
{
	if (PlayState != EMazePlayState::Playing)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(StepTimerHandle);
	SetPlayState(EMazePlayState::Paused);
}

void AMazeVisualizerActor::Stop()
{
	GetWorldTimerManager().ClearTimer(StepTimerHandle);
	MazeGenerator->ResetMaze();
	BuildFloorGrid();
	RebuildWalls();
	SetPlayState(EMazePlayState::Stopped);
}

void AMazeVisualizerActor::Restart()
{
	GetWorldTimerManager().ClearTimer(StepTimerHandle);
	PlayState = EMazePlayState::Stopped;
	Play();
}

void AMazeVisualizerActor::GenerateInstantly()
{
	GetWorldTimerManager().ClearTimer(StepTimerHandle);

	if (MazeGenerator->GetTotalCellCount() == 0)
	{
		MazeGenerator->ResetMaze();
		BuildFloorGrid();
	}

	MazeGenerator->GenerateInstant();
	RebuildWalls();
	SetPlayState(EMazePlayState::Completed);
}

void AMazeVisualizerActor::SetMazeSize(int32 NewWidth, int32 NewHeight)
{
	MazeGenerator->MazeWidth = NewWidth;
	MazeGenerator->MazeHeight = NewHeight;
	Stop();
}

void AMazeVisualizerActor::SetUseFixedSeed(bool bNewUseFixedSeed, int32 NewSeed)
{
	MazeGenerator->bUseFixedSeed = bNewUseFixedSeed;
	MazeGenerator->RandomSeed = NewSeed;
	Stop();
}

void AMazeVisualizerActor::SetPlayState(EMazePlayState NewState)
{
	if (PlayState == NewState)
	{
		return;
	}

	PlayState = NewState;
	OnPlayStateChanged.Broadcast(PlayState);
}

void AMazeVisualizerActor::TickStep()
{
	const bool bComplete = MazeGenerator->GenerateStep();
	RebuildWalls();

	if (bComplete)
	{
		GetWorldTimerManager().ClearTimer(StepTimerHandle);
		SetPlayState(EMazePlayState::Completed);
	}
}

void AMazeVisualizerActor::BuildFloorGrid()
{
	FloorInstances->ClearInstances();

	UpdateMinimapFraming();

	if (!FloorMesh)
	{
		return;
	}

	const float ScaleX = CellSizeX / MeshUnitSize;
	const float ScaleY = CellSizeY / MeshUnitSize;
	const float ThicknessScale = FloorThickness / MeshUnitSize;

	for (int32 Y = 0; Y < MazeGenerator->MazeHeight; ++Y)
	{
		for (int32 X = 0; X < MazeGenerator->MazeWidth; ++X)
		{
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(X * CellSizeX, Y * CellSizeY, -FloorThickness * 0.5f));
			InstanceTransform.SetScale3D(FVector(ScaleX, ScaleY, ThicknessScale));
			FloorInstances->AddInstance(InstanceTransform);
		}
	}
}

void AMazeVisualizerActor::RebuildWalls()
{
	WallInstances->ClearInstances();

	if (!WallMesh)
	{
		return;
	}

	// North/South walls run along X (unrotated), so their length matches CellSizeX; West/East
	// walls run along Y (rotated 90 degrees), so their length matches CellSizeY instead.
	const float LengthScaleX = CellSizeX / MeshUnitSize;
	const float LengthScaleY = CellSizeY / MeshUnitSize;
	const float ThicknessScale = WallThickness / MeshUnitSize;
	const float HeightScale = WallHeight / MeshUnitSize;
	const float HalfCellX = CellSizeX * 0.5f;
	const float HalfCellY = CellSizeY * 0.5f;
	const float WallZ = WallHeight * 0.5f;
	const FQuat RunAlongY(FRotator(0.0f, 90.0f, 0.0f));

	const int32 Width = MazeGenerator->MazeWidth;
	const int32 Height = MazeGenerator->MazeHeight;

	// Each interior wall is only owned by one side (North/West) so it's added exactly once;
	// the outer South/East boundary is only carved when its owning cell is on the last row/column.
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const float CenterX = X * CellSizeX;
			const float CenterY = Y * CellSizeY;

			if (MazeGenerator->HasWall(X, Y, EMazeWall::North))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX, CenterY - HalfCellY, WallZ));
				T.SetScale3D(FVector(LengthScaleX, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (MazeGenerator->HasWall(X, Y, EMazeWall::West))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX - HalfCellX, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScaleY, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (Y == Height - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::South))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX, CenterY + HalfCellY, WallZ));
				T.SetScale3D(FVector(LengthScaleX, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (X == Width - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::East))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX + HalfCellX, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScaleY, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}
		}
	}

	// Walls changed shape, so the minimap's rendered image is stale regardless of whether its
	// camera moved this frame.
	if (MinimapCapture && MinimapRenderTarget)
	{
		MinimapCapture->CaptureScene();
	}
}

void AMazeVisualizerActor::UpdateMinimapFraming()
{
	if (!MinimapCapture || !MazeGenerator)
	{
		return;
	}

	const int32 Width = FMath::Max(1, MazeGenerator->MazeWidth);
	const int32 Height = FMath::Max(1, MazeGenerator->MazeHeight);

	// MinimapCapture is pitched -90 with no yaw, so its local Right axis lines up with world Y and
	// its local Up axis lines up with world X (see UMazeMinimapWidget::UpdatePlayerIcon). The render
	// target's horizontal pixel axis therefore tracks world Y extent, and its vertical axis tracks
	// world X extent - not the other way around. Aspect ratio must follow physical size, not cell
	// counts, since cells aren't necessarily square.
	const float CaptureRightExtent = Height * CellSizeY;
	const float CaptureUpExtent = Width * CellSizeX;

	const int32 LongEdge = FMath::Max(8, MinimapResolution);
	int32 RTWidth = LongEdge;
	int32 RTHeight = LongEdge;
	if (CaptureRightExtent >= CaptureUpExtent)
	{
		RTHeight = FMath::Max(8, FMath::RoundToInt(LongEdge * CaptureUpExtent / CaptureRightExtent));
	}
	else
	{
		RTWidth = FMath::Max(8, FMath::RoundToInt(LongEdge * CaptureRightExtent / CaptureUpExtent));
	}

	if (!MinimapRenderTarget)
	{
		MinimapRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("MinimapRenderTarget"));
		MinimapRenderTarget->RenderTargetFormat = RTF_RGBA8;
		MinimapRenderTarget->ClearColor = FLinearColor::Black;
		MinimapRenderTarget->bAutoGenerateMips = false;
		MinimapRenderTarget->InitAutoFormat(RTWidth, RTHeight);
		MinimapRenderTarget->UpdateResourceImmediate(true);
		MinimapCapture->TextureTarget = MinimapRenderTarget;
	}
	else if (MinimapRenderTarget->SizeX != RTWidth || MinimapRenderTarget->SizeY != RTHeight)
	{
		MinimapRenderTarget->InitAutoFormat(RTWidth, RTHeight);
		MinimapRenderTarget->UpdateResourceImmediate(true);
	}

	// Default framing: whole maze, centered on the maze itself. UpdateMinimapView() - normally
	// driven by a UMazeMinimapWidget every tick - overrides this to pan/zoom around a tracked pawn;
	// this is just the fallback used before that ever runs (e.g. the editor preview outside PIE).
	const FVector MazeCenterLocal((Width - 1) * CellSizeX * 0.5f, (Height - 1) * CellSizeY * 0.5f, 0.0f);
	const FVector MazeCenterWorld = MazeRoot->GetComponentTransform().TransformPosition(MazeCenterLocal);
	UpdateMinimapView(MazeCenterWorld, 1.0f);
}

void AMazeVisualizerActor::UpdateMinimapView(const FVector& FocusWorldLocation, float Scale)
{
	if (!MinimapCapture || !MazeGenerator || !MinimapRenderTarget)
	{
		return;
	}

	const float ClampedScale = FMath::Clamp(Scale, 0.05f, 1.0f);

	const int32 Width = FMath::Max(1, MazeGenerator->MazeWidth);
	const int32 Height = FMath::Max(1, MazeGenerator->MazeHeight);

	const float FullWorldWidth = Width * CellSizeX;
	const float FullWorldHeight = Height * CellSizeY;

	const float ViewWorldWidth = FullWorldWidth * ClampedScale;
	const float ViewWorldHeight = FullWorldHeight * ClampedScale;

	const FVector MapCenterLocal((Width - 1) * CellSizeX * 0.5f, (Height - 1) * CellSizeY * 0.5f, 0.0f);
	const FVector FocusLocal = MazeRoot->GetComponentTransform().InverseTransformPosition(FocusWorldLocation);

	// At Scale 1 we're centered on the maze (guarantees the whole grid is visible); as Scale shrinks
	// we blend toward centering on the focus point (e.g. the player) instead.
	FVector CenterLocal = FMath::Lerp(MapCenterLocal, FVector(FocusLocal.X, FocusLocal.Y, 0.0f), 1.0f - ClampedScale);

	// Keep the view window from hanging off the edge of the maze when the window is smaller than it.
	const float MapMinX = -CellSizeX * 0.5f;
	const float MapMaxX = FullWorldWidth - CellSizeX * 0.5f;
	const float MapMinY = -CellSizeY * 0.5f;
	const float MapMaxY = FullWorldHeight - CellSizeY * 0.5f;

	if (ViewWorldWidth < FullWorldWidth)
	{
		CenterLocal.X = FMath::Clamp(CenterLocal.X, MapMinX + ViewWorldWidth * 0.5f, MapMaxX - ViewWorldWidth * 0.5f);
	}
	if (ViewWorldHeight < FullWorldHeight)
	{
		CenterLocal.Y = FMath::Clamp(CenterLocal.Y, MapMinY + ViewWorldHeight * 0.5f, MapMaxY - ViewWorldHeight * 0.5f);
	}

	CenterLocal.Z = MinimapCaptureHeight;

	// OrthoWidth is the capture's horizontal (Right) extent, which lines up with world Y - see the
	// axis note in UpdateMinimapFraming().
	const float NewOrthoWidth = ViewWorldHeight > 0.0f ? ViewWorldHeight : FullWorldHeight;
	const bool bMoved = !MinimapCapture->GetRelativeLocation().Equals(CenterLocal, 0.5f)
		|| !FMath::IsNearlyEqual(MinimapCapture->OrthoWidth, NewOrthoWidth, 0.5f);

	MinimapCapture->SetRelativeLocation(CenterLocal);
	MinimapCapture->OrthoWidth = NewOrthoWidth;

	if (bMoved)
	{
		MinimapCapture->CaptureScene();
	}
}

bool AMazeVisualizerActor::ExportMazeToFile(const FString& FilePath) const
{
	if (!MazeGenerator)
	{
		return false;
	}

	return MazeGenerator->ExportToFile(FilePath);
}

bool AMazeVisualizerActor::ImportMazeFromFile(const FString& FilePath)
{
	if (!MazeGenerator)
	{
		return false;
	}

	GetWorldTimerManager().ClearTimer(StepTimerHandle);

	if (!MazeGenerator->ImportFromFile(FilePath))
	{
		return false;
	}

	BuildFloorGrid();
	RebuildWalls();
	SetPlayState(MazeGenerator->IsGenerationComplete() ? EMazePlayState::Completed : EMazePlayState::Stopped);

	return true;
}

FVector2D AMazeVisualizerActor::WorldToMinimapUV(const FVector& WorldLocation) const
{
	if (!MinimapCapture || !MinimapRenderTarget || MinimapRenderTarget->SizeX <= 0 || MinimapRenderTarget->SizeY <= 0 || MinimapCapture->OrthoWidth <= 0.0f)
	{
		return FVector2D(0.5f, 0.5f);
	}

	const FTransform CaptureTransform = MinimapCapture->GetComponentTransform();
	const FVector Offset = WorldLocation - CaptureTransform.GetLocation();

	const float ViewWidth = MinimapCapture->OrthoWidth;
	const float ViewHeight = ViewWidth * (float)MinimapRenderTarget->SizeY / (float)MinimapRenderTarget->SizeX;

	const float RightOffset = FVector::DotProduct(Offset, CaptureTransform.GetUnitAxis(EAxis::Y));
	const float UpOffset = FVector::DotProduct(Offset, CaptureTransform.GetUnitAxis(EAxis::Z));

	const float U = 0.5f + (RightOffset / ViewWidth);
	const float V = 0.5f - (UpOffset / ViewHeight);

	return FVector2D(U, V);
}
