// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WilsonMazeGenerator.h"
#include "MazeVisualizerActor.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMazeControlWidget;

UENUM(BlueprintType)
enum class EMazePlayState : uint8
{
	Stopped,
	Playing,
	Paused,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMazePlayStateChanged, EMazePlayState, NewState);

// Places a Wilson's-algorithm maze into the level as floor/wall instanced static meshes, and
// animates the carving over time so the build process can be watched. Drive it with Play/Pause/
// Stop/Restart, either from Blueprint or from a UMazeControlWidget.
UCLASS(Blueprintable, ClassGroup = (Maze))
class MOBILEUETESTPROJECT_API AMazeVisualizerActor : public AActor
{
	GENERATED_BODY()

public:
	AMazeVisualizerActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze|Components")
	TObjectPtr<USceneComponent> MazeRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze|Components")
	TObjectPtr<UInstancedStaticMeshComponent> FloorInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze|Components")
	TObjectPtr<UInstancedStaticMeshComponent> WallInstances;

	// Owns the logical maze grid; its Width/Height/Seed config drives what this actor builds.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Maze")
	TObjectPtr<UWilsonMazeGenerator> MazeGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	TObjectPtr<UStaticMesh> FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual")
	TObjectPtr<UStaticMesh> WallMesh;

	// World-space size (uu) of FloorMesh/WallMesh's unmodified bounding box (e.g. 100 for engine
	// BasicShapes). Used to scale instances so CellSize/WallHeight/WallThickness read in uu.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual", meta = (ClampMin = "1.0"))
	float MeshUnitSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual", meta = (ClampMin = "1.0"))
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual", meta = (ClampMin = "1.0"))
	float WallHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual", meta = (ClampMin = "1.0"))
	float WallThickness = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visual", meta = (ClampMin = "1.0"))
	float FloorThickness = 10.0f;

	// Seconds between generation steps while Playing (animates the carving). 0 = generate instantly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Playback", meta = (ClampMin = "0.0"))
	float StepInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Playback")
	bool bAutoPlayOnBeginPlay = true;

	// If set, an instance of this widget is created and added to the viewport on BeginPlay,
	// pre-wired to this actor - drop this actor into a level, assign a WBP_ child of
	// UMazeControlWidget here, and the control panel just works with no extra Blueprint wiring.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|UI")
	TSubclassOf<UMazeControlWidget> ControlWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|UI")
	bool bAutoCreateControlWidget = true;

	UPROPERTY(BlueprintReadOnly, Category = "Maze|UI")
	TObjectPtr<UMazeControlWidget> ControlWidgetInstance;

	// Broadcast whenever Play/Pause/Stop/Restart/completion changes the playback state.
	UPROPERTY(BlueprintAssignable, Category = "Maze|Events")
	FOnMazePlayStateChanged OnPlayStateChanged;

	// Starts animated generation from a fresh maze, or resumes if currently paused.
	UFUNCTION(BlueprintCallable, Category = "Maze|Playback")
	void Play();

	// Freezes the in-progress animated generation; Play() resumes from where it left off.
	UFUNCTION(BlueprintCallable, Category = "Maze|Playback")
	void Pause();

	// Halts generation and resets to an ungenerated (all-walls-up) grid.
	UFUNCTION(BlueprintCallable, Category = "Maze|Playback")
	void Stop();

	// Stops and immediately begins a brand new animated generation.
	UFUNCTION(BlueprintCallable, Category = "Maze|Playback")
	void Restart();

	// Builds the whole maze in one call, skipping the step-by-step animation.
	UFUNCTION(BlueprintCallable, Category = "Maze|Playback", meta = (DisplayName = "Generate Instantly"))
	void GenerateInstantly();

	// Applies new grid dimensions and stops (call Play()/Restart() to build with the new size).
	UFUNCTION(BlueprintCallable, Category = "Maze|Config")
	void SetMazeSize(int32 NewWidth, int32 NewHeight);

	// Applies seed config and stops (call Play()/Restart() to build with the new seed).
	UFUNCTION(BlueprintCallable, Category = "Maze|Config")
	void SetUseFixedSeed(bool bNewUseFixedSeed, int32 NewSeed);

	UFUNCTION(BlueprintPure, Category = "Maze|Playback")
	EMazePlayState GetPlayState() const { return PlayState; }

	UFUNCTION(BlueprintPure, Category = "Maze|Playback")
	float GetGenerationProgress() const { return MazeGenerator ? MazeGenerator->GetGenerationProgress() : 0.0f; }

protected:
	virtual void BeginPlay() override;
	// Builds an instant preview in the editor viewport (outside PIE) so the actor doesn't sit
	// empty until you press Play; BeginPlay() takes over and re-generates animated afterward.
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	EMazePlayState PlayState = EMazePlayState::Stopped;
	FTimerHandle StepTimerHandle;

	void SetPlayState(EMazePlayState NewState);
	void TickStep();

	// Full floor grid is built once per reset; it never changes shape during generation.
	void BuildFloorGrid();
	// Walls are rebuilt every step so standing walls visibly fall away as corridors are carved.
	void RebuildWalls();
};
