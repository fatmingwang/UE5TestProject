# Maze Generator - How To Use

## Overview

Wilson's-algorithm maze generator with a 3D visualizer actor, an in-game control panel,
a top-down minimap, and an editor-only tool window (no Play needed).

**Classes:**

- **`UWilsonMazeGenerator`** (`Public/Private WilsonMazeGenerator.h/.cpp`)
  Pure logic: carves a perfect maze on a MazeWidth x MazeHeight grid.
- **`AMazeVisualizerActor`** (`Public/Private MazeVisualizerActor.h/.cpp`)
  Places the maze in a level as floor/wall instanced static meshes.
  Owns a `UWilsonMazeGenerator` instance. Handles Play/Pause/Stop/Restart.
  Also owns `MinimapCapture` (a top-down SceneCaptureComponent2D) and
  `MinimapRenderTarget`, which a `UMazeMinimapWidget` displays.
- **`UMazeControlWidget`** (`Public/Private MazeControlWidget.h/.cpp`)
  In-game (Play mode only) on-screen panel: sliders + Play/Pause/Stop/Restart buttons.
  Builds its own UI in C++ (`RebuildWidget`) - no UMG Designer work needed.
- **`UMazeMinimapWidget`** (`Public/Private MazeMinimapWidget.h/.cpp`)
  In-game top-down minimap: shows `AMazeVisualizerActor`'s `MinimapRenderTarget` with a
  "you are here" icon (dot + facing arrow) that tracks the locally controlled pawn.
  Has its own Zoom slider (1.0 = whole maze, lower = follow-zoomed). Also builds its
  own UI in C++ (`RebuildWidget`) - no UMG Designer work needed.
- **`UMazeEditorUtilityWidget`** (`Public/Private MazeEditorUtilityWidget.h/.cpp`)
  Editor-only tool window: adjust parameters and regenerate WITHOUT pressing Play.
  > **Note:** lives in the `MobileUETestProject` module but only links its editor-only
  > dependencies (UnrealEd/Blutility/UMGEditor) when `Target.Type == Editor` (see
  > `MobileUETestProject.Build.cs`). Fine for Live Coding / normal editor use. If this
  > project's `MobileUETestProject` (packaged Game) target is ever actually built, move
  > `MazeEditorUtilityWidget.h/.cpp` into their own editor-only module first, or that
  > build will fail.

**Blueprint assets** (in `/Game/Blueprints/`):

| Asset | Description |
| --- | --- |
| `BP_MazeVisualizerActor` | Blueprint child of `AMazeVisualizerActor`. FloorMesh/WallMesh already set to `/Engine/BasicShapes/Cube`. Drop this into any level. |
| `WBP_MazeControlWidget` | Blueprint child of `UMazeControlWidget`. Assigned to `BP_MazeVisualizerActor`'s "Control Widget Class" property, so it auto-creates and adds itself to the viewport on BeginPlay. |
| `WBP_MazeMinimapWidget` | Blueprint child of `UMazeMinimapWidget`. Assign it to `BP_MazeVisualizerActor`'s "Minimap Widget Class" property (see step 1 below) so it auto-creates and adds itself to the viewport on BeginPlay, same as the control widget. |
| `EUW_MazeEditorTools` | Editor Utility Widget child of `UMazeEditorUtilityWidget`. |

## 1. Adding the maze + minimap widget to a level

a) Drag `BP_MazeVisualizerActor` from `/Game/Blueprints/` into the level viewport.
   FloorMesh/WallMesh/ControlWidgetClass are already configured on the Blueprint.

   To place it in a brand new level: File -> New Level, then drag the Blueprint in
   as above.

b) Wire up the minimap widget. Select the actor you just placed and check its
   Details panel, category "Maze|UI":

   - **Control Widget Class** - should already be `WBP_MazeControlWidget`.
   - **Minimap Widget Class** - set this to `WBP_MazeMinimapWidget` if it isn't already
     (this property has to be set per-instance, or on the Blueprint's defaults if you
     want every placed instance to get one automatically).
   - **Auto Create Control Widget / Auto Create Minimap Widget** - leave both checked
     (default true) so the widgets add themselves to the viewport on BeginPlay with
     no extra Blueprint wiring needed.

That's it - press Play and both the control panel (top-left) and the minimap should
appear automatically, already pointed at this maze actor (`AMazeVisualizerActor::BeginPlay`
calls `SetMazeActor()` on each widget for you).

If you only want the minimap (no control panel), leave Control Widget Class empty /
uncheck Auto Create Control Widget - the two widgets are independent.

## 2. Minimap tunables (on `WBP_MazeMinimapWidget`)

- **Zoom slider** (in the widget itself, bottom of the panel, 0.05-1.0) - 1.0 = whole
  maze visible; lower = zooms in and pans to follow the locally controlled pawn.
  Drives `AMazeVisualizerActor::UpdateMinimapView()` every tick.
- **Minimap Panel Size** - on-screen size (pixels) of the minimap panel.
- **Player Icon Color** - color of the "you are here" dot.
- **Player Direction Length** - length (pixels) of the facing arrow sticking out of the dot.
- **Minimap Capture Height / Minimap Resolution** (on `AMazeVisualizerActor`, category
  Maze|Minimap) - height of the top-down capture camera above the maze, and the
  longest edge (px) of the render target it captures into.

## 3. Watching it build in real time (Play mode)

Press Play. By default (`bAutoPlayOnBeginPlay = true`) the actor immediately starts an
animated build: it calls `GenerateStep()` every `StepInterval` seconds (default 0.05s),
and standing walls visibly fall away as corridors get carved.

A control panel auto-appears in the top-left corner (`WBP_MazeControlWidget`), showing:

- Width / Height / Step Interval / Seed sliders (with live value labels)
- Use Fixed Seed checkbox
- Play / Pause / Stop / Restart buttons (2x2 grid)
- Progress bar + status text (Stopped / Playing / Paused / Completed)

Dragging a slider updates the underlying value immediately; Restart (or Play from
Stopped) is what applies a changed Width/Height/Seed to a fresh maze.

## 4. Editing without pressing Play

Two ways, pick whichever is more convenient:

### a) Details panel (always available, no extra tool needed)

Select the actor in the viewport or World Outliner. Its properties are under:

- **Maze -> MazeGenerator** (instanced sub-object: Maze Width, Maze Height,
  Use Fixed Seed, Random Seed)
- **Maze|Visual** (Cell Size, Wall Height, Wall Thickness, Floor Thickness,
  Floor Mesh, Wall Mesh, Mesh Unit Size)
- **Maze|Playback** (Step Interval, Auto Play On Begin Play)

Any edit reruns `OnConstruction`, which regenerates the maze instantly (no Play
needed) - this is the actor's live editor preview.

### b) `EUW_MazeEditorTools` (floating tool window)

In the Content Browser, right-click `EUW_MazeEditorTools` -> Run Editor Utility Widget.

Steps:

1. Select a MazeVisualizerActor instance in the level.
2. Click "Refresh From Selection" in the tool (shows "Target: `<Actor Name>`").
3. Adjust sliders (Width, Height, Cell Size, Wall Height, Wall Thickness, Seed)
   and/or the Use Fixed Seed checkbox.
4. Click "Generate Instantly" to rebuild the maze with the new values, or
   "Clear" to reset to an empty (all-walls-up) grid.

This only calls synchronous functions (`GenerateInstantly`/`Stop`) - it does not use
the animated step timer, since editor-mode worlds don't reliably tick game timers
the way a running PIE session does.

## 5. Key tunable parameters (on `AMazeVisualizerActor`)

| Parameter | Description |
| --- | --- |
| Cell Size | world-space spacing between cells (uu) |
| Wall Height | wall instance height (uu) |
| Wall Thickness | wall instance thickness (uu) |
| Floor Thickness | floor instance thickness (uu) |
| Mesh Unit Size | the bounding-box size of FloorMesh/WallMesh's *unscaled* source asset (100 for engine BasicShapes cube/plane). Used to compute instance scale so the above sizes read correctly in uu. |
| Step Interval | seconds between `GenerateStep()` calls while Playing; 0 = instant |
| Maze Width/Height | grid dimensions (on the MazeGenerator sub-object) |
| Use Fixed Seed/Seed | deterministic vs. random generation |

## TODO

See [`Doc/TODO.txt`](TODO.txt).
