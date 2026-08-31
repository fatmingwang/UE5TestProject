// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeVisualizerActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "MazeControlWidget.h"
#include "MazeMinimapWidget.h"
#include "StoreButtonWidget.h"
#include "Blueprint/UserWidget.h"

AMazeVisualizerActor::AMazeVisualizerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MazeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MazeRoot"));
	SetRootComponent(MazeRoot);

	// Movable, not Static: BuildFloorGrid()/RebuildWalls() call ClearInstances()/AddInstances() on
	// these repeatedly, both in the editor (OnConstruction, on every relevant property edit) and at
	// runtime (Play/GenerateInstantly/TickStep/Restart/Stop/Import). Static-mobility primitives use
	// cached mesh draw commands that get invalidated and fully rebuilt on any state change, including
	// the component's *selected* state in the editor - with ClearInstances()/AddInstances() churn on
	// top of that, simply re-selecting this actor in the World Outliner was enough to force an
	// expensive rebuild and stall the editor UI for a few seconds. Movable components skip that
	// cached-draw-command path entirely, which is also the semantically correct choice for content
	// that keeps changing shape after being placed.
	FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
	FloorInstances->SetupAttachment(MazeRoot);
	FloorInstances->SetMobility(EComponentMobility::Movable);
	// Nothing in this project pathfinds through the maze (the player is a Character, not a
	// nav-driven AI), but bCanEverAffectNavigation defaults to true - so every ClearInstances()/
	// AddInstances() call (OnConstruction on every level load/property edit, plus every Play/
	// GenerateInstantly/Restart/Import at runtime) was queuing a full navmesh rebuild over the whole
	// grid. That's what turned a level reopen into a multi-second editor freeze. Off, since there's
	// no navmesh consumer to keep in sync.
	FloorInstances->SetCanEverAffectNavigation(false);

	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
	WallInstances->SetupAttachment(MazeRoot);
	WallInstances->SetMobility(EComponentMobility::Movable);
	WallInstances->SetCanEverAffectNavigation(false);

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
		if (!bAssignMazeFromPoolOnBeginPlay || !AssignRandomMazeFromDirectory(MazePoolDirectory))
		{
			Play();
		}
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

	if (!NeedsEditorRebuild())
	{
		return;
	}

	// Running the full generation algorithm here made every OnConstruction call - which fires on
	// every level load, every relevant property edit, and (for a Blueprint-placed actor whose class
	// was just hot-reloaded) sometimes on mere (re)selection - pay for a synchronous maze generation
	// on the game thread, which is what was freezing the editor UI. The preview only needs *something*
	// to look at: try the pre-generated pool first (as cheap as reading one small JSON file), and
	// otherwise fall back to an all-walls-up grid (O(cell count), no random walk) instead of actually
	// generating one. Play()/GenerateInstantly() (the button, or BeginPlay via bAutoPlayOnBeginPlay)
	// still run full generation when it's actually wanted.
	if (!bAssignMazeFromPoolOnBeginPlay || !AssignRandomMazeFromDirectory(MazePoolDirectory))
	{
		MazeGenerator->ResetMaze();
		BuildFloorGrid();
		RebuildWalls();
	}

	RecordBuildSignature();
}

bool AMazeVisualizerActor::NeedsEditorRebuild() const
{
	if (!bHasBuiltOnce || !MazeGenerator)
	{
		return true;
	}

	return LastBuiltFloorMesh.Get() != FloorMesh
		|| LastBuiltWallMesh.Get() != WallMesh
		|| LastBuiltMeshUnitSize != MeshUnitSize
		|| LastBuiltCellSizeX != CellSizeX
		|| LastBuiltCellSizeY != CellSizeY
		|| LastBuiltWallHeight != WallHeight
		|| LastBuiltWallThickness != WallThickness
		|| LastBuiltFloorThickness != FloorThickness
		|| LastBuiltMazeWidth != MazeGenerator->MazeWidth
		|| LastBuiltMazeHeight != MazeGenerator->MazeHeight
		|| LastBuiltUseFixedSeed != MazeGenerator->bUseFixedSeed
		|| LastBuiltRandomSeed != MazeGenerator->RandomSeed;
}

void AMazeVisualizerActor::RecordBuildSignature()
{
	bHasBuiltOnce = true;
	LastBuiltFloorMesh = FloorMesh;
	LastBuiltWallMesh = WallMesh;
	LastBuiltMeshUnitSize = MeshUnitSize;
	LastBuiltCellSizeX = CellSizeX;
	LastBuiltCellSizeY = CellSizeY;
	LastBuiltWallHeight = WallHeight;
	LastBuiltWallThickness = WallThickness;
	LastBuiltFloorThickness = FloorThickness;
	if (MazeGenerator)
	{
		LastBuiltMazeWidth = MazeGenerator->MazeWidth;
		LastBuiltMazeHeight = MazeGenerator->MazeHeight;
		LastBuiltUseFixedSeed = MazeGenerator->bUseFixedSeed;
		LastBuiltRandomSeed = MazeGenerator->RandomSeed;
	}
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

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(MazeGenerator->MazeWidth * MazeGenerator->MazeHeight);

	for (int32 Y = 0; Y < MazeGenerator->MazeHeight; ++Y)
	{
		for (int32 X = 0; X < MazeGenerator->MazeWidth; ++X)
		{
			FTransform& InstanceTransform = InstanceTransforms.AddDefaulted_GetRef();
			InstanceTransform.SetLocation(FVector(X * CellSizeX, Y * CellSizeY, -FloorThickness * 0.5f));
			InstanceTransform.SetScale3D(FVector(ScaleX, ScaleY, ThicknessScale));
		}
	}

	// One bulk call instead of one AddInstance() per cell - each individual call pays for its own
	// bounds/render-state update, which adds up fast for large grids and is a major source of the
	// stall this rebuild otherwise causes in the editor.
	FloorInstances->AddInstances(InstanceTransforms, /*bShouldReturnIndices=*/false);
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

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(Width * Height * 2);

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
				FTransform& T = InstanceTransforms.AddDefaulted_GetRef();
				T.SetLocation(FVector(CenterX, CenterY - HalfCellY, WallZ));
				T.SetScale3D(FVector(LengthScaleX, ThicknessScale, HeightScale));
			}

			if (MazeGenerator->HasWall(X, Y, EMazeWall::West))
			{
				FTransform& T = InstanceTransforms.AddDefaulted_GetRef();
				T.SetLocation(FVector(CenterX - HalfCellX, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScaleY, ThicknessScale, HeightScale));
			}

			if (Y == Height - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::South))
			{
				FTransform& T = InstanceTransforms.AddDefaulted_GetRef();
				T.SetLocation(FVector(CenterX, CenterY + HalfCellY, WallZ));
				T.SetScale3D(FVector(LengthScaleX, ThicknessScale, HeightScale));
			}

			if (X == Width - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::East))
			{
				FTransform& T = InstanceTransforms.AddDefaulted_GetRef();
				T.SetLocation(FVector(CenterX + HalfCellX, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScaleY, ThicknessScale, HeightScale));
			}
		}
	}

	// One bulk call instead of one AddInstance() per wall segment - see BuildFloorGrid() for why.
	WallInstances->AddInstances(InstanceTransforms, /*bShouldReturnIndices=*/false);

	// Walls changed shape, so the minimap's rendered image is stale regardless of whether its
	// camera moved this frame. Skip this outside a running game (e.g. editor OnConstruction/preview
	// rebuilds) since nothing is displaying MinimapRenderTarget there - it's just an extra forced
	// scene render on every edit.
	const UWorld* World = GetWorld();
	if (MinimapCapture && MinimapRenderTarget && World && World->IsGameWorld())
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

	// UpdateResourceImmediate(true) forces a synchronous render-thread flush - a major stall source
	// when this runs from OnConstruction (every Details-panel edit / Blueprint-editor preview
	// refresh). false still queues the resize, just without blocking the game thread on it.
	if (!MinimapRenderTarget)
	{
		MinimapRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("MinimapRenderTarget"));
		MinimapRenderTarget->RenderTargetFormat = RTF_RGBA8;
		MinimapRenderTarget->ClearColor = FLinearColor::Black;
		MinimapRenderTarget->bAutoGenerateMips = false;
		MinimapRenderTarget->InitAutoFormat(RTWidth, RTHeight);
		MinimapRenderTarget->UpdateResourceImmediate(false);
		MinimapCapture->TextureTarget = MinimapRenderTarget;
	}
	else if (MinimapRenderTarget->SizeX != RTWidth || MinimapRenderTarget->SizeY != RTHeight)
	{
		MinimapRenderTarget->InitAutoFormat(RTWidth, RTHeight);
		MinimapRenderTarget->UpdateResourceImmediate(false);
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

	// See the equivalent check in RebuildWalls(): skip the forced scene render outside a running
	// game, since nothing is displaying MinimapRenderTarget there.
	const UWorld* World = GetWorld();
	if (bMoved && World && World->IsGameWorld())
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

int32 AMazeVisualizerActor::GenerateAndExportMazes(int32 NumMazes, const FString& OutputDirectory, const FString& FileNamePrefix)
{
	if (!MazeGenerator || !MazeGenerator->bEnableFileIO || NumMazes <= 0)
	{
		return 0;
	}

	GetWorldTimerManager().ClearTimer(StepTimerHandle);

	const bool bOriginalUseFixedSeed = MazeGenerator->bUseFixedSeed;
	const int32 OriginalSeed = MazeGenerator->RandomSeed;

	int32 NumExported = 0;
	for (int32 Index = 0; Index < NumMazes; ++Index)
	{
		if (bOriginalUseFixedSeed)
		{
			// A fixed seed would otherwise regenerate the exact same maze every iteration.
			MazeGenerator->RandomSeed = OriginalSeed + Index;
		}

		MazeGenerator->GenerateInstant();

		const FString FilePath = FPaths::Combine(OutputDirectory, FString::Printf(TEXT("%s_%03d.json"), *FileNamePrefix, Index));
		if (MazeGenerator->ExportToFile(FilePath))
		{
			++NumExported;
		}
	}

	MazeGenerator->bUseFixedSeed = bOriginalUseFixedSeed;
	MazeGenerator->RandomSeed = OriginalSeed;

	// Reflect the last generated maze in the visualization, matching GenerateInstantly()'s end state.
	BuildFloorGrid();
	RebuildWalls();
	SetPlayState(EMazePlayState::Completed);

	return NumExported;
}

bool AMazeVisualizerActor::AssignMazeFromFile(const FString& FilePath)
{
	return ImportMazeFromFile(FilePath);
}

bool AMazeVisualizerActor::AssignRandomMazeFromDirectory(const FString& Directory)
{
	TArray<FString> JsonFileNames;
	IFileManager::Get().FindFiles(JsonFileNames, *Directory, TEXT("*.json"));

	if (JsonFileNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMazeVisualizerActor::AssignRandomMazeFromDirectory: no .json files found in '%s'."), *Directory);
		return false;
	}

	const FString ChosenFile = JsonFileNames[FMath::RandHelper(JsonFileNames.Num())];
	return AssignMazeFromFile(FPaths::Combine(Directory, ChosenFile));
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
