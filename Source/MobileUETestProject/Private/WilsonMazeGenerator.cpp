// Fill out your copyright notice in the Description page of Project Settings.

#include "WilsonMazeGenerator.h"

UWilsonMazeGenerator::UWilsonMazeGenerator()
{
}

void UWilsonMazeGenerator::ResetMaze()
{
	MazeWidth = FMath::Max(2, MazeWidth);
	MazeHeight = FMath::Max(2, MazeHeight);

	const int32 NumCells = MazeWidth * MazeHeight;

	Cells.Init(FMazeCell(), NumCells);
	WalkNextDir.Init(-1, NumCells);

	UnvisitedCells.SetNumUninitialized(NumCells);
	UnvisitedIndexOf.SetNumUninitialized(NumCells);
	for (int32 i = 0; i < NumCells; ++i)
	{
		UnvisitedCells[i] = i;
		UnvisitedIndexOf[i] = i;
	}

	if (bUseFixedSeed)
	{
		RandomStream.Initialize(RandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}

	CurrentWalkCell = INDEX_NONE;
	WalkStartCell = INDEX_NONE;
	NumCellsInMaze = 0;

	// Wilson's algorithm needs one cell already committed to the maze to give random walks something to walk into.
	const int32 StartCell = RandomStream.RandRange(0, NumCells - 1);
	Cells[StartCell].bInMaze = true;
	NumCellsInMaze = 1;
	RemoveFromUnvisited(StartCell);
}

void UWilsonMazeGenerator::GenerateInstant()
{
	ResetMaze();

	while (!IsGenerationComplete())
	{
		StepOnce();
	}

	OnMazeGenerationComplete.Broadcast();
}

bool UWilsonMazeGenerator::GenerateStep()
{
	if (Cells.Num() == 0)
	{
		ResetMaze();
	}

	if (IsGenerationComplete())
	{
		return true;
	}

	bool bComplete = false;
	for (int32 i = 0; i < StepsPerCall && !bComplete; ++i)
	{
		bComplete = StepOnce();
	}

	if (bComplete)
	{
		OnMazeGenerationComplete.Broadcast();
	}

	return bComplete;
}

bool UWilsonMazeGenerator::StepOnce()
{
	if (IsGenerationComplete())
	{
		return true;
	}

	if (CurrentWalkCell == INDEX_NONE)
	{
		const int32 RandIdx = RandomStream.RandRange(0, UnvisitedCells.Num() - 1);
		WalkStartCell = UnvisitedCells[RandIdx];
		CurrentWalkCell = WalkStartCell;
	}

	EMazeWall ValidDirs[4];
	int32 NeighborByDir[4];
	int32 NumValidDirs = 0;

	for (int32 d = 0; d < 4; ++d)
	{
		int32 NeighborIdx;
		if (GetNeighborInDirection(CurrentWalkCell, (EMazeWall)d, NeighborIdx))
		{
			ValidDirs[NumValidDirs] = (EMazeWall)d;
			NeighborByDir[NumValidDirs] = NeighborIdx;
			++NumValidDirs;
		}
	}

	const int32 ChosenIdx = RandomStream.RandRange(0, NumValidDirs - 1);
	const EMazeWall ChosenDir = ValidDirs[ChosenIdx];
	const int32 NextCell = NeighborByDir[ChosenIdx];

	WalkNextDir[CurrentWalkCell] = (int8)ChosenDir;
	CurrentWalkCell = NextCell;

	if (Cells[NextCell].bInMaze)
	{
		// The walk has reached the maze: follow the recorded directions from the start of the walk,
		// carving as we go. Any loop taken along the way was already erased by the overwrite above.
		int32 Cell = WalkStartCell;
		while (!Cells[Cell].bInMaze)
		{
			const EMazeWall Dir = (EMazeWall)WalkNextDir[Cell];
			int32 Neighbor;
			GetNeighborInDirection(Cell, Dir, Neighbor);

			CarveConnection(Cell, Neighbor, Dir);
			Cells[Cell].bInMaze = true;
			++NumCellsInMaze;
			RemoveFromUnvisited(Cell);

			Cell = Neighbor;
		}

		CurrentWalkCell = INDEX_NONE;
		WalkStartCell = INDEX_NONE;
	}

	return IsGenerationComplete();
}

void UWilsonMazeGenerator::RemoveFromUnvisited(int32 CellIdx)
{
	const int32 PosInArray = UnvisitedIndexOf[CellIdx];
	const int32 LastCell = UnvisitedCells.Last();

	UnvisitedCells[PosInArray] = LastCell;
	UnvisitedIndexOf[LastCell] = PosInArray;
	UnvisitedCells.Pop(EAllowShrinking::No);
	UnvisitedIndexOf[CellIdx] = INDEX_NONE;
}

bool UWilsonMazeGenerator::GetNeighborInDirection(int32 CellIdx, EMazeWall Direction, int32& OutNeighborIdx) const
{
	const int32 X = CellIdx % MazeWidth;
	const int32 Y = CellIdx / MazeWidth;

	switch (Direction)
	{
	case EMazeWall::North:
		if (Y <= 0) return false;
		OutNeighborIdx = CellIndex(X, Y - 1);
		return true;
	case EMazeWall::South:
		if (Y >= MazeHeight - 1) return false;
		OutNeighborIdx = CellIndex(X, Y + 1);
		return true;
	case EMazeWall::East:
		if (X >= MazeWidth - 1) return false;
		OutNeighborIdx = CellIndex(X + 1, Y);
		return true;
	case EMazeWall::West:
		if (X <= 0) return false;
		OutNeighborIdx = CellIndex(X - 1, Y);
		return true;
	default:
		return false;
	}
}

EMazeWall UWilsonMazeGenerator::OppositeWall(EMazeWall Wall)
{
	switch (Wall)
	{
	case EMazeWall::North: return EMazeWall::South;
	case EMazeWall::South: return EMazeWall::North;
	case EMazeWall::East:  return EMazeWall::West;
	case EMazeWall::West:  return EMazeWall::East;
	default:               return EMazeWall::North;
	}
}

void UWilsonMazeGenerator::CarveConnection(int32 CellA, int32 CellB, EMazeWall DirFromAToB)
{
	Cells[CellA].Walls &= ~(1 << (uint8)DirFromAToB);
	Cells[CellB].Walls &= ~(1 << (uint8)OppositeWall(DirFromAToB));
}

bool UWilsonMazeGenerator::HasWall(int32 X, int32 Y, EMazeWall Wall) const
{
	if (!Cells.IsValidIndex(CellIndex(X, Y)))
	{
		return true;
	}
	return (Cells[CellIndex(X, Y)].Walls & (1 << (uint8)Wall)) != 0;
}

bool UWilsonMazeGenerator::IsCellInMaze(int32 X, int32 Y) const
{
	if (!Cells.IsValidIndex(CellIndex(X, Y)))
	{
		return false;
	}
	return Cells[CellIndex(X, Y)].bInMaze;
}

FMazeCell UWilsonMazeGenerator::GetCell(int32 X, int32 Y) const
{
	if (!Cells.IsValidIndex(CellIndex(X, Y)))
	{
		return FMazeCell();
	}
	return Cells[CellIndex(X, Y)];
}
