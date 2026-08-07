// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "WilsonMazeGenerator.generated.h"

// Bit index into FMazeCell::Walls for each side of a cell.
UENUM(BlueprintType)
enum class EMazeWall : uint8
{
	North = 0,
	East  = 1,
	South = 2,
	West  = 3
};

USTRUCT(BlueprintType)
struct FMazeCell
{
	GENERATED_BODY()

	// Bitmask of walls still standing on this cell (bit set via EMazeWall = wall present).
	UPROPERTY(BlueprintReadOnly, Category = "MyMaze", meta = (DisplayName = "Walls"))
	uint8 Walls = 0x0F;

	UPROPERTY(BlueprintReadOnly, Category = "MyMaze", meta = (DisplayName = "In Maze"))
	bool bInMaze = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMazeGenerationComplete);

// Generates a perfect maze on a rectangular grid using Wilson's algorithm (loop-erased random walk),
// which produces a uniformly-random spanning tree (no bias toward long corridors like recursive backtracking has).
UCLASS(BlueprintType, Blueprintable)
class MOBILEUETESTPROJECT_API UWilsonMazeGenerator : public UObject
{
	GENERATED_BODY()

public:
	UWilsonMazeGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Config", meta = (ClampMin = "2", DisplayName = "Maze Width"))
	int32 MazeWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Config", meta = (ClampMin = "2", DisplayName = "Maze Height"))
	int32 MazeHeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Config", meta = (DisplayName = "Use Fixed Seed"))
	bool bUseFixedSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Config", meta = (EditCondition = "bUseFixedSeed", DisplayName = "Random Seed"))
	int32 RandomSeed = 0;

	// How many loop-erased-random-walk steps GenerateStep() advances per call. Lower = smoother animation, higher = faster.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Config", meta = (ClampMin = "1", DisplayName = "Steps Per Call"))
	int32 StepsPerCall = 1;

	// Broadcast once the maze finishes generating, from either GenerateInstant() or the final GenerateStep().
	UPROPERTY(BlueprintAssignable, Category = "MyMaze|Events")
	FOnMazeGenerationComplete OnMazeGenerationComplete;

	// Clears the grid (all walls up) and re-seeds the RNG. GenerateInstant()/GenerateStep() call this
	// automatically when needed, but call it directly to restart generation with new dimensions/seed.
	UFUNCTION(BlueprintCallable, Category = "MyMaze")
	void ResetMaze();

	// Mode A: generate the whole maze synchronously in a single call.
	UFUNCTION(BlueprintCallable, Category = "MyMaze", meta = (DisplayName = "Generate Maze (Instant)"))
	void GenerateInstant();

	// Mode B: advance generation by StepsPerCall steps (e.g. call once per Tick to animate the carving).
	// Returns true once the maze is fully generated.
	UFUNCTION(BlueprintCallable, Category = "MyMaze", meta = (DisplayName = "Generate Maze (Step)"))
	bool GenerateStep();

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	bool IsGenerationComplete() const { return Cells.Num() > 0 && NumCellsInMaze >= Cells.Num(); }

	// Fraction of cells carved into the maze so far (0..1). Useful for driving a progress bar.
	UFUNCTION(BlueprintPure, Category = "MyMaze")
	float GetGenerationProgress() const { return Cells.Num() > 0 ? (float)NumCellsInMaze / (float)Cells.Num() : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	int32 GetTotalCellCount() const { return Cells.Num(); }

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	bool HasWall(int32 X, int32 Y, EMazeWall Wall) const;

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	bool IsCellInMaze(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	FMazeCell GetCell(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "MyMaze")
	TArray<FMazeCell> GetCells() const { return Cells; }

	// Master switch for ExportToFile()/ImportFromFile(), checked identically at runtime and in-editor.
	// Turn off to lock the current maze data down (e.g. a shipping build that shouldn't read/write
	// maze files). Defaults to on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|IO", meta = (DisplayName = "Enable File IO"))
	bool bEnableFileIO = true;

	// Writes the current grid (dimensions + per-cell walls/InMaze flags) to FilePath as JSON.
	// Returns false if bEnableFileIO is off, there is no maze data yet, or the write fails.
	UFUNCTION(BlueprintCallable, Category = "MyMaze|IO")
	bool ExportToFile(const FString& FilePath) const;

	// Reads a maze previously written by ExportToFile() and replaces the current grid with it.
	// Returns false if bEnableFileIO is off, or the file is missing/invalid.
	UFUNCTION(BlueprintCallable, Category = "MyMaze|IO")
	bool ImportFromFile(const FString& FilePath);

private:
	TArray<FMazeCell> Cells;

	// Direction each cell currently points to while it is part of the in-progress random walk.
	// Re-walking over a cell overwrites its entry here, which is what erases loops for free.
	TArray<int8> WalkNextDir;

	TArray<int32> UnvisitedCells;
	TArray<int32> UnvisitedIndexOf;

	int32 CurrentWalkCell = INDEX_NONE;
	int32 WalkStartCell = INDEX_NONE;
	int32 NumCellsInMaze = 0;

	FRandomStream RandomStream;

	// Performs one loop-erased-random-walk step. Returns true if the maze is complete as a result.
	bool StepOnce();

	void RemoveFromUnvisited(int32 CellIdx);

	FORCEINLINE int32 CellIndex(int32 X, int32 Y) const { return Y * MazeWidth + X; }
	bool GetNeighborInDirection(int32 CellIdx, EMazeWall Direction, int32& OutNeighborIdx) const;
	static EMazeWall OppositeWall(EMazeWall Wall);
	void CarveConnection(int32 CellA, int32 CellB, EMazeWall DirFromAToB);
};
