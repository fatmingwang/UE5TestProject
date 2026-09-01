# Loadout / opponent data integration — progress

Tracks implementation of design doc [section 6](MAIN_BATTLE_SCENE_DESIGN.md#6-loadout--opponent-data)
(Gloves/Stances/Perks/Opponents data catalogs). Update this file as work continues so a
new session can pick up without re-deriving state from the diff.

## Goal

Let designers tune boxer stats/power-bar feel via JSON instead of hardcoding them, and
let `AFightDirector` assemble a fight (player loadout + AI opponent) by ID lookup.

## Status: full setup built and placed in MazeLevel; next is a PIE test

### Done

1. **`Source/MobileUETestProject/Public/BoxingLoadoutData.h`** — `FBoxerGloveData`,
   `FBoxerStanceData`, `FBoxerPerkData`, `FOpponentData` USTRUCTs. Field names match the
   JSON keys 1:1.
2. **`BoxingDataSubsystem.h`/`.cpp`** — `UGameInstanceSubsystem` that loads the 4
   catalogs from JSON on `Initialize()` (mirrors `UStoreSubsystem`'s pattern). Exposes
   `GetGloves()/GetStances()/GetPerks()/GetOpponents()`, `Find*()` (C++, raw pointer) and
   `TryFind*()` (Blueprint-callable, out-param) lookups by ID.
3. **`Content/BoxingData/{Gloves,Stances,Perks,Opponents}.json`** — populated catalogs
   (e.g. Gloves has `gloves_training`/`gloves_quick_wraps`/`gloves_power`). Confirmed to
   exist and parse-shaped correctly against the structs on 2026-09-01.
4. **`BoxerCharacter.h`/`.cpp`** — `ApplyGloveData()`, `ApplyStanceData()`,
   `ApplyOpponentData()`, `ApplyPerkData()` + `GetActivePerk()`. These configure a
   boxer's ATK/DEF/HP and Attack/Defense bar tuning from a catalog entry. Compiled
   successfully in-editor (Live Coding) on 2026-09-01.
5. **`FightDirector.h`/`.cpp`** — added `PlayerGloveID` (default `"gloves_training"`),
   `PlayerStanceID`, `PlayerPerkID` (optional — empty means "no perk"), `OpponentID`
   (default `"opp_rookie_ray"`) properties, and `ApplyLoadouts()` (called first thing in
   `BeginPlay()`) which looks each ID up via `UBoxingDataSubsystem` and applies whatever's
   found to `PlayerBoxer`/`OpponentBoxer`. Unknown/empty IDs are skipped silently — that
   boxer just keeps its current stats. Also syncs `AFightDirector::OpponentSkill` from the
   resolved `FOpponentData::OpponentSkill`. Compiled successfully in-editor (Live Coding)
   on 2026-09-01.
6. Confirmed `Content/BoxingData/Opponents.json` has 3 tiered entries —
   `opp_rookie_ray` (MaxHP 80, easiest, OpponentSkill 0.9), `opp_iron_mike_jr` (MaxHP
   120, mid), `opp_the_butcher` (MaxHP 160, final boss, OpponentSkill 0.3) — and set
   `opp_rookie_ray` as `FightDirector::OpponentID`'s default so a fresh
   `BP_FightDirector` isn't empty.

### Next steps

1. `OpponentID` default-value change compiled successfully (2026-09-01).
2. **Blueprint/level setup confirmed missing via Unreal MCP editor inspection on
   2026-09-01**: `/Game/Blueprints/` has no `BP_FightDirector`, `BP_BoxerCharacter`,
   `BP_BoxingCameraRig`, or `WBP_BoxingHUDWidget`, and `/Game/Level/` has no
   battle/boxing level (only `MazeLevel`, `2DLevel`, `LevelA`, `LevelB`, `MCPLevel*`).
   **User decision (2026-09-01)**: don't build a dedicated battle level — the fight
   should be startable *anytime*, either via a hotkey (for testing) or a collision-box
   trigger (for gameplay), implying it lives alongside normal play (e.g. in `MazeLevel`)
   rather than a separate isolated level.
3. **Implemented on-demand start (2026-09-01, not yet compiled)** in response to that
   decision:
   - `AFightDirector` no longer auto-starts the fight in `BeginPlay()`. Added
     `bAutoStartOnBeginPlay` (default false, opt back into old behavior),
     `StartFight()` (the new single entry point — applies loadouts, shows/creates the
     HUD, switches input to Game+UI, plays the Bell shot, starts the Bell timer; no-op if
     a fight is already running/finished), and `ResetFight()` (reverts a
     finished/in-progress fight back to dormant so `StartFight()` can run again — mainly
     for repeat hotkey testing).
   - `AFightDirector` binds a debug hotkey (`bBindTestHotkey`, default true;
     `TestHotkey`, default **F9**) straight to `StartFight()` via `EnableInput()` +
     `InputComponent->BindKey()` (same legacy-input pattern `AToggleCameraCharacter`
     already uses for its `V` camera-toggle key — no Enhanced Input setup needed).
   - New **`ABoxingFightTrigger`** actor (`BoxingFightTrigger.h`/`.cpp`) — a `UBoxComponent`
     overlap volume with a `FightDirectorRef` pointer; calls `FightDirectorRef->StartFight()`
     when the player pawn overlaps it. `bTriggerOnce` (default true) disables its own
     collision after firing once; set false + pair with `ResetFight()` for a re-triggerable
     test volume.
4. **Compiled successfully** — confirmed via Unreal MCP (`search_subclasses` found both
   `/Script/MobileUETestProject.FightDirector` and the brand-new
   `/Script/MobileUETestProject.BoxingFightTrigger` already live in the running editor's
   reflection system, 2026-09-01), so Live Coding picked up the new UCLASS fine.
5. **Built and placed via Unreal MCP editor tools, 2026-09-01** — everything below is
   saved to disk (Blueprint assets + `MazeLevel`):
   - `/Game/Blueprints/BP_BoxerCharacter` (parent `ABoxerCharacter`) — `CharacterMesh0`
     set to `SKM_Manny_Simple` + `ABP_Unarmed` (`AnimationMode=AnimationBlueprint`);
     confirmed `ABP_Unarmed`'s AnimGraph has a `Slot'DefaultSlot'` node, matching
     `MontageSlotName`'s default, so montages will blend correctly. Punch/hit-react/death
     anim slots filled with placeholder Mannequin mocap clips (`MM_Attack_01/02/03`,
     `MM_ChargedAttack`, `MM_HitReact_Front_Lgt/Med/Hvy_*`, `MM_Death_Front_01/02`) —
     **not curated for feel, just non-empty so nothing null-derefs**.
   - `/Game/Blueprints/BP_BoxerCharacter_Opponent` — duplicate of the above, mesh swapped
     to `SKM_Quinn_Simple` for visual distinction from the player.
   - `/Game/Blueprints/BP_BoxingCameraRig`, `/Game/Blueprints/BP_FightDirector`
     (`HUDWidgetClass` set straight to native `UBoxingHUDWidget` — no BP widget subclass
     needed, confirming design doc §8's "self-building C++ widget" holds), and
     `/Game/Blueprints/BP_BoxingFightTrigger` — no extra defaults needed beyond parent.
   - Placed in `MazeLevel` (all off away from the maze's play area near
     `PlayerStart_0` at (800,800,50), on a new 3000x3000 floor plane
     `BoxingRing_Floor` at (800,-2000,0) so nothing falls through empty space):
     `BoxingPlayerBoxer` (650,-2000), `BoxingOpponentBoxer` (950,-2000, facing it),
     `BoxingCameraRig` (800,-2300, side-on per design doc §7), `BoxingFightDirector`
     (800,-2000) with `PlayerBoxer`/`OpponentBoxer`/`CameraRig` all wired, and
     `BoxingFightTrigger` (800,-1700 — on the path from `PlayerStart_0` toward the ring)
     with `FightDirectorRef` wired. All confirmed via `get_properties` after wiring.
6. **Next: PIE-test one full fight loop** (walk from `PlayerStart_0` south along Y
   toward (800,-2000) to hit the trigger box, or just press **F9** anywhere) — Bell →
   Attack → Defend → Win/Lose — to confirm loadout stats affect bar feel/damage and both
   start paths work. Not yet done — needs a human in PIE, not something scriptable via
   the MCP tools used so far.
7. Placeholder art/animation choices above (Manny/Quinn mesh, generic mocap clips) are
   functional stand-ins, not final — swap when real boxing animations/character art
   exist.
8. Decide whether `PlayerGloveID`/`PlayerStanceID`/`PlayerPerkID`/`OpponentID` should be
   selectable from a pre-fight UI (loadout-select screen) rather than fixed
   `EditAnywhere` values — out of scope for this pass, but design doc doesn't specify a
   selection UI yet (see design doc §11 open questions).
9. Perk's non-numeric effects (`MitigationFloorBonus`, `AttackPowerBonusPct`,
   `bZeroDamageOnRedGuard`) are stored on the boxer via `GetActivePerk()` but **not yet
   read anywhere** — `AFightDirector::HandlePlayerAttackReleased`/
   `HandlePlayerGuardReleased` still compute damage without consulting them. Needs
   follow-up once the base loop is verified.

10. **Implemented camera/possession switch on fight start (2026-09-01, not yet
    compiled)**, per user request — previously `StartFight()` set up the HUD/input but
    never actually changed what the player saw or controlled: `CameraRig`'s Bell/Ready
    framing logic was running every tick but the `PlayerController`'s view target never
    switched to it, so none of it was ever visible.
    - `StartFight()` now stores the controller's current pawn/view target
      (`PreviousPawn`/`PreviousViewTarget`), calls `PC->Possess(PlayerBoxer)`, and blends
      the view to `CameraRig` via `SetViewTargetWithBlend(CameraRig, 0.5f, VTBlend_Cubic)`
      — so `PlayBellShot`/`EnterReadyFraming`'s cinematography (design doc §7) is what's
      actually on screen once a fight starts.
    - `ResetFight()` now restores both (re-possesses `PreviousPawn`, blends the view back
      to `PreviousViewTarget`) so backing out returns full control to whatever the player
      was controlling before (e.g. `BP_FirstPersonCharacter` in the maze).
    - The `TestHotkey` (F9) now calls a new private `ToggleFightForTesting()` instead of
      `StartFight()` directly — press once to enter (possess + camera switch), press
      again to exit (`ResetFight()`) — so testing doesn't strand you possessing
      `PlayerBoxer` with no movement input bound and no way back.
    - `ABoxerCharacter` has no movement input bindings of its own, so possessing it is
      inert for movement — this is purely for camera/control correctness during the
      fight, not to let the player walk around as the boxer.

### Playtest bugs to fix next (reported 2026-09-01, not yet investigated/fixed)

1. **Punch animation drags the player out of position.** After throwing a punch, the
   player boxer ends up physically further from the opponent than where it started —
   over repeated punches this walks the player away entirely. Likely cause: the
   placeholder Mannequin mocap clips (`MM_Attack_01/02/03`, `MM_ChargedAttack` — see
   item 5 above) have root motion baked in and enabled, and
   `ABoxerCharacter::PlayDynamicMontage()`/`PlayPunchMontage()`
   (`BoxerCharacter.cpp` — plays them via `PlaySlotAnimationAsDynamicMontage`) doesn't
   currently suppress it, so `CharacterMovementComponent` applies the clip's forward
   translation to the capsule. Fix options to weigh: (a) disable "Enable Root Motion" on
   the specific anim assets, (b) force root motion translation off for these dynamic
   montages at playback time, or (c) simplest/most robust regardless of which anim
   assets end up used — cache the actor's location when a punch/guard montage starts and
   snap back to it on montage end (or every tick while `Punching`/`Guarding`/
   `HitReacting`), so the boxer is pinned in place no matter what root motion a clip
   carries.
2. **Fight doesn't feel turn-based — the opponent just idles.** Currently only the
   player's boxer ever animates/reacts: `AFightDirector::StartPlayerAttackPhase()`
   starts the player's turn but `OpponentBoxer` has no visual response while the player
   charges/releases a punch, and `StartPlayerDefendPhase()`/`ResolveOpponentPunch()`
   compute the incoming punch's numbers but never make `OpponentBoxer` actually play a
   punch animation for it — the player is asked to guard against an attack they never
   see thrown. Need:
   - While `CurrentState == PlayerAttack` (player charging/releasing their punch),
     `OpponentBoxer` should visibly guard — it already has `EnterGuardPose()`/
     `ExitGuardPose()` (currently private, used only for the *local* boxer's own
     `StartChargingGuard()`/`ReleaseGuard()`); needs either a public wrapper or a new
     `AFightDirector`-callable hook so the opponent can be put into/out of a guard pose
     on the player's turn.
   - While `CurrentState == PlayerDefend` (opponent's turn), `OpponentBoxer` should play
     its own punch montage matching what `ResolveOpponentPunch()` computed, timed with
     the defend phase (e.g. triggered from `StartPlayerDefendPhase()`) rather than the
     punch being purely a background number crunch the player never sees.

### Known gaps / risks

- `ApplyOpponentData()` resets `CurrentHP = MaxHP` unconditionally — fine for
  fight-start use, but would be wrong if ever called mid-fight.
- No validation/logging when a `PlayerGloveID`/`OpponentID` etc. isn't found in the
  catalog beyond "silently skip" — could be confusing to debug from the editor if a
  designer typos an ID. Consider a `UE_LOG` warning in `ApplyLoadouts()` if this bites
  someone.
