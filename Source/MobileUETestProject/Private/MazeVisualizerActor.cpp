// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeVisualizerActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "MazeControlWidget.h"
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

	if (!FloorMesh)
	{
		return;
	}

	const float Scale = CellSize / MeshUnitSize;
	const float ThicknessScale = FloorThickness / MeshUnitSize;

	for (int32 Y = 0; Y < MazeGenerator->MazeHeight; ++Y)
	{
		for (int32 X = 0; X < MazeGenerator->MazeWidth; ++X)
		{
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(X * CellSize, Y * CellSize, -FloorThickness * 0.5f));
			InstanceTransform.SetScale3D(FVector(Scale, Scale, ThicknessScale));
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

	const float LengthScale = CellSize / MeshUnitSize;
	const float ThicknessScale = WallThickness / MeshUnitSize;
	const float HeightScale = WallHeight / MeshUnitSize;
	const float HalfCell = CellSize * 0.5f;
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
			const float CenterX = X * CellSize;
			const float CenterY = Y * CellSize;

			if (MazeGenerator->HasWall(X, Y, EMazeWall::North))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX, CenterY - HalfCell, WallZ));
				T.SetScale3D(FVector(LengthScale, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (MazeGenerator->HasWall(X, Y, EMazeWall::West))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX - HalfCell, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScale, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (Y == Height - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::South))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX, CenterY + HalfCell, WallZ));
				T.SetScale3D(FVector(LengthScale, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}

			if (X == Width - 1 && MazeGenerator->HasWall(X, Y, EMazeWall::East))
			{
				FTransform T;
				T.SetLocation(FVector(CenterX + HalfCell, CenterY, WallZ));
				T.SetRotation(RunAlongY);
				T.SetScale3D(FVector(LengthScale, ThicknessScale, HeightScale));
				WallInstances->AddInstance(T);
			}
		}
	}
}
