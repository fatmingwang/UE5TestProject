# Maze Generator - How To Use

## Overview

Wilson's-algorithm maze generator with a 3D visualizer actor, an in-game control panel,
a top-down minimap, a self-wiring store button, maze export/import (JSON) with random
pool assignment, and an editor-only tool window (no Play needed).

**Classes:**

- **`UWilsonMazeGenerator`** (`Public/Private WilsonMazeGenerator.h/.cpp`)
  Pure logic: carves a perfect maze on a MazeWidth x MazeHeight grid. Also owns
  `ExportToFile()`/`ImportFromFile()` (JSON, gated by `bEnableFileIO`).
- **`AMazeVisualizerActor`** (`Public/Private MazeVisualizerActor.h/.cpp`)
  Places the maze in a level as floor/wall instanced static meshes.
  Owns a `UWilsonMazeGenerator` instance. Handles Play/Pause/Stop/Restart, plus
  exporting/importing/pool-assigning maze JSON files (see section 5 below).
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
- **`UStoreButtonWidget`** (`Public/Private StoreButtonWidget.h/.cpp`)
  Self-building "Store" button. `AMazeVisualizerActor` creates one on BeginPlay (see
  "Auto Create Store Button" below); clicking it lazily creates and toggles a
  `UStoreWidget` popup (item list comes from `UStoreSubsystem`) - unrelated to the maze
  itself, just piggybacks on the same actor for convenience.
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
| `WBP_StoreButtonWidget` | Blueprint child of `UStoreButtonWidget`. Not required on "Store Button Widget Class" - `AMazeVisualizerActor` falls back to the base `UStoreButtonWidget` class automatically if that property is left unset (see step 1 below). |
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
   - **Store Button Widget Class / Auto Create Store Button** - leave "Auto Create
     Store Button" checked (default true) to get a Store button added to the viewport
     too; "Store Button Widget Class" can be left empty (falls back to the base
     `UStoreButtonWidget`) or set to `WBP_StoreButtonWidget` if you've customized it.
     This is unrelated to the maze itself (see `UStoreButtonWidget` above) - uncheck
     "Auto Create Store Button" if you don't want it.

That's it - press Play and the control panel (top-left), the minimap, and the store
button should all appear automatically, already pointed at this maze actor
(`AMazeVisualizerActor::BeginPlay` calls `SetMazeActor()` on the control/minimap widgets
for you).

If you only want the minimap (no control panel), leave Control Widget Class empty /
uncheck Auto Create Control Widget - the widgets are independent of each other.

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
- **Maze|Visual** (Cell Size X, Cell Size Y, Wall Height, Wall Thickness,
  Floor Thickness, Floor Mesh, Wall Mesh, Mesh Unit Size)
- **Maze|Playback** (Step Interval, Auto Play On Begin Play)

Any edit reruns `OnConstruction`, which regenerates the maze instantly (no Play
needed) - this is the actor's live editor preview.

### b) `EUW_MazeEditorTools` (floating tool window)

In the Content Browser, right-click `EUW_MazeEditorTools` -> Run Editor Utility Widget.

Steps:

1. Select a MazeVisualizerActor instance in the level.
2. Click "Refresh From Selection" in the tool (shows "Target: `<Actor Name>`").
3. Adjust sliders (Width, Height, Cell Size X/Y, Wall Height, Wall Thickness, Seed)
   and/or the Use Fixed Seed checkbox.
4. Click "Generate Instantly" to rebuild the maze with the new values, or
   "Clear" to reset to an empty (all-walls-up) grid.

This only calls synchronous functions (`GenerateInstantly`/`Stop`) - it does not use
the animated step timer, since editor-mode worlds don't reliably tick game timers
the way a running PIE session does.

## 5. Exporting/importing mazes (JSON) and picking from a pool

Gated end-to-end by `UWilsonMazeGenerator->bEnableFileIO` (category MyMaze|IO, default
true) - turn it off to lock maze data down (e.g. a shipping build that shouldn't read/
write files). While off, every function below returns false/0 and does nothing, and the
Export/Import buttons in 5a/5b hide themselves.

### a) Quick export/import of the *current* maze (UI, no Blueprint needed)

Two different buttons exist, at two different fixed/flexible locations - pick whichever
fits what you're doing:

- **In-game control panel** (`WBP_MazeControlWidget`, Play mode) - **Export**/**Import**
  buttons next to Play/Pause/Stop/Restart. No file picker (mobile-safe): they always
  read/write a fixed path, `<ProjectSavedDir>/MazeExports/Maze.json`
  (`UMazeControlWidget::GetDefaultMazeFilePath`). Use this to quickly snapshot whatever
  maze is currently on screen, or reload that same snapshot later.
- **`EUW_MazeEditorTools`** (editor tool, no Play needed) - its own **Export.../Import...**
  buttons, next to Generate Instantly/Clear. These open a native Save/Open file dialog
  (default folder `<ProjectSavedDir>`), so you choose the exact filename/path each time.
  This is the normal way to hand-pick where a single maze JSON file goes.

Both call the exact same `AMazeVisualizerActor::ExportMazeToFile`/`ImportMazeFromFile`
as the Blueprint API below - Import always rebuilds the floor/walls/minimap to match and
stops any in-progress animated generation.

### b) Batch-generating a pool of maze JSON files

There's no button for this (it's meant to be run once, offline, to pre-bake content) -
call `AMazeVisualizerActor::GenerateAndExportMazes(NumMazes, OutputDirectory, FileNamePrefix = "Maze")`
yourself, e.g. from a Blueprint (a Level Blueprint node, an Editor Utility Blueprint, or
a temporary button added to a Blueprint child of this actor) or from C++/console. It
batch-generates `NumMazes` distinct mazes at the current Maze Width/Height and writes
each to `OutputDirectory` as `"{FileNamePrefix}_000.json"`, `"{FileNamePrefix}_001.json"`,
etc. (if `Use Fixed Seed` is on, each maze in the batch still gets a distinct seed so it
isn't N copies of the same maze; the original seed is restored afterward). Leaves the
last generated maze on screen and returns how many files were exported successfully.

### c) Using a specific JSON file as the maze for a level ("stage data")

To pin one exact, hand-picked file (not a random pick from a folder), call
**`AssignMazeFromFile(FilePath)`** with that file's path - this loads it and makes it
current without running Wilson's algorithm at all, in place of `Play()`/
`GenerateInstantly()`. There's no Details-panel property for a single fixed file, so
wire this yourself: on a Blueprint child of `AMazeVisualizerActor`, override
`ReceiveBeginPlay`/`Event BeginPlay` and call `Assign Maze From File` with the path
(e.g. a file shipped under `Content/` or `ProjectSavedDir()/MazeExports/Level01.json`) -
or call it from a Level Blueprint / GameMode instead if you'd rather keep the maze actor
Blueprint untouched.

To instead pick **randomly from a folder of pre-generated files** (e.g. one produced by
5b) rather than one specific file, either:

- call `AssignRandomMazeFromDirectory(Directory)` yourself the same way as above, or
- set **Assign Maze From Pool On Begin Play** (category Maze|IO) and point **Maze Pool
  Directory** at that folder - `BeginPlay()` then calls this automatically, with no
  Blueprint wiring needed. If the directory is empty or unreadable, it silently falls
  back to the normal `Play()` (live-generated, animated) behavior instead.

## 6. Key tunable parameters (on `AMazeVisualizerActor`)

| Parameter | Description |
| --- | --- |
| Cell Size X / Cell Size Y | world-space spacing between cells along X/Y (uu); total maze size = MazeWidth * CellSizeX by MazeHeight * CellSizeY |
| Wall Height | wall instance height (uu) |
| Wall Thickness | wall instance thickness (uu) |
| Floor Thickness | floor instance thickness (uu) |
| Mesh Unit Size | the bounding-box size of FloorMesh/WallMesh's *unscaled* source asset (100 for engine BasicShapes cube/plane). Used to compute instance scale so the above sizes read correctly in uu. |
| Step Interval | seconds between `GenerateStep()` calls while Playing; 0 = instant |
| Maze Width/Height | grid dimensions (on the MazeGenerator sub-object) |
| Use Fixed Seed/Seed | deterministic vs. random generation |
| Enable File IO (on MazeGenerator) | master switch for export/import/pool functions in section 5 |
| Assign Maze From Pool On Begin Play / Maze Pool Directory | auto-pick a random pool maze on BeginPlay instead of live-generating one; see section 5 |

## TODO

See [`Doc/TODO.txt`](TODO.txt).
