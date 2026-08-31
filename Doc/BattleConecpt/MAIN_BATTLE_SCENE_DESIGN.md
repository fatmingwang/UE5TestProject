# Boxing Battle Scene - Design Doc

## 0. Scope

Consolidates both HTML prototypes in this folder into one spec, reskinned from their
fantasy-RPG framing (weapon/armor/enemy) to a boxing framing (gloves/stance/opponent),
targeting a 3D scene in this project:

- `fight_scene_demo.html` - the attack/defense power-bar mechanic in isolation (§5 below).
- `main_battle_scene_demo.html` - the full round loop around it: loadout, opponent pick,
  HP, turn-by-turn resolve (§6, §9 below).

**Target look reference**: `battle.jpg` - two boxers face-to-face, both in a raised-fists
guard stance, shot from the side so both profiles read clearly, ring ropes visible
behind, moody rim-lit teal/orange lighting. This is the composition every "Ready"/"Bell"
camera beat (§7) should match - **both boxers facing each other**, not one facing the
camera.

**Reference screenshots** (real in-engine captures using the project's existing
Mannequin meshes/anims, placeholder floor - see §7 for the camera beats they match):

- `demo_wide_establishing.png` - Bell / establishing two-shot. Both boxers face each
  other (mirrored lunge, `MM_Attack_01` held near frame 0), matching `battle.jpg`.
- `demo_medium_twoshot.png` - Ready / between-exchanges framing, same corrected
  face-to-face facing as above, tighter.
- `demo_closeup_punch.png` - Punch resolve close-up. **Not yet redone**: Quinn (visible
  in the background) still faces the camera instead of Manny, the same rotation mistake
  fixed in the two shots above - redo if this one is needed as reference.
- `demo_ko_moment.png` - KO moment (uses `MM_Death_Front_01` near its final frame,
  which ends hunched-over rather than flat on the mat - see the Win/Guard content-gap
  note in §4). Facing doesn't read either way in this pose, so it wasn't redone.

Nothing here is implemented yet - this is the spec to build against.

## 1. Overview

Two boxers face off on a small mat. Combat is turn-based-real-time: the player HOLDs an
on-screen button (mouse click - "player click attack and defence button"); a marker
bounces back and forth across a bar; RELEASING locks in a power % based on how close to
dead-center the marker was when released. Center = best (green), edges = worst (red).

Loop: player throws a punch (Attack bar) -> opponent swings back and the player must
guard it (Defense bar) -> repeat until one boxer's HP hits 0 (KO).

## 2. Scene layout

- **Floor**: a simple rectangular mat (~600x600uu), placeholder geometry in the same
  style `AMazeVisualizerActor` already uses for its floor/walls (Engine BasicShapes
  Cube, scaled) - a real ring-canvas material can go on top later without changing
  anything structural.
- **Ring ropes** (optional, purely decorative for v1): 4 corner-post cubes + thin
  cylinder/box "ropes" between them.
- **Characters**: 2 Characters on the shared UE5 Mannequin skeleton, already in this
  project - `SKM_Manny_Simple` (Player, blue corner) and `SKM_Quinn_Simple` (Opponent,
  red corner), both under `Content/Characters/Mannequins/Meshes/`. Both can share the
  existing `ABP_Unarmed` Animation Blueprint. **Face to face**, per `battle.jpg`: place
  them on opposite sides of the mat, ~200uu apart, each rotated so its forward vector
  points at the other boxer (i.e. their yaws are 180 degrees apart, not parallel) -
  both idle in a raised-fists guard stance (§4's Guard stand-in), not one of them facing
  the camera/audience.
- **Lighting**: a single DirectionalLight + SkyLight is enough for v1 (matches
  `MazeLevel`'s current lighting); a 3-point rig (overhead key + 2 corner-colored rim
  lights) is a nice-to-have, not a blocker.

## 3. Characters / classes

- **`ABoxerCharacter : ACharacter`** - one shared class for *both* boxers (player and
  opponent). Holds `CurrentHP`/`MaxHP`, an `EBoxerState` (`Idle`, `ChargingAttack`,
  `ChargingGuard`, `Punching`, `Guarding`, `HitReacting`, `KO`, `Victorious`), and reads
  the shared power-bar math (§5) - kept as a plain struct + free functions, not a
  heavyweight component, matching how `UWilsonMazeGenerator` is a dependency-light
  "pure logic" object in this project rather than an engine-coupled component.
- Player-side `ABoxerCharacter` is possessed by the local `APlayerController` as usual.
- Opponent-side `ABoxerCharacter` is **not** nav-driven AI (no pathfinding, it doesn't
  move around a maze) - it just runs its own Attack/Defense bar with a "release near a
  target time" policy, tuned per `UOpponentData` entry (§6) so tougher opponents release
  closer to their own bar's center more often.

## 4. Animation

Existing assets under `Content/Characters/Mannequins/Anims/Unarmed/` already cover most
of this - mapped to boxing roles and to the zone/outcome that should trigger them:

| Existing asset | Boxing role | Trigger |
| --- | --- | --- |
| `MM_Idle` / `BS_Idle_Walk_Run` | Ready stance | Default state between exchanges |
| `MM_Attack_01` | Jab (light punch) | RED/BLUE-zone attack release |
| `MM_Attack_02` | Cross (medium punch) | BLUE-zone / alternate combo beat |
| `MM_Attack_03` | Hook (heavy punch) | Alternate combo beat, flavor variety |
| `MM_ChargedAttack` | Power punch | GREEN / GREEN-PERFECT attack release |
| `MM_HitReact_Front_Lgt_01..04` | Light hit reaction | Damage taken in BLUE zone (partial guard) |
| `MM_HitReact_Front_Med_01/02` | Medium hit reaction | Damage taken in RED zone / unguarded |
| `MM_HitReact_Front_Hvy_01` | Heavy hit reaction / stagger | Near-KO threshold, or hit by a GREEN-PERFECT punch |
| `MM_HitReact_Back_Med_01` | Spin-reaction (flavor) | Optional variant for a big counter-punch |
| `MM_Death_Front_01/02/03`, `MM_Death_Back_01`, `MM_Death_Left_01`, `MM_Death_Right_01` | KO / Lose | HP hits 0 - pick the variant matching the finishing punch's side for variety |

**Gaps - not in the current Mannequin pack, need sourcing before this reads as boxing
rather than "two mannequins standing near each other":**

- **Guard/Block held pose** - no existing asset. v1 stand-in: hold `MM_Attack_01`'s
  first frame (a raised-fists pose) as a static pose during the Defense-bar hold window.
  Source a proper "boxing guard idle" (Mixamo or hand-animated) as a follow-up.
- **Perfect-Guard "parry/counter" flourish** - no dedicated asset; skip the flourish for
  v1 (no damage + a screen-flash/SFX cue is enough), don't block the feature on it.
- **Win/Victory celebration** - no existing asset. v1 stand-in: freeze on Idle, let the
  camera do the celebrating (§7 Victory orbit) plus a UI "WINNER" banner, until a real
  celebration anim is sourced.

**Implementation**: one `UAnimMontage` per punch, per hit-react tier, and per KO
direction, played from `ABoxerCharacter` via `PlayAnimMontage()`. Montages layer on top
of `ABP_Unarmed`'s existing locomotion state machine untouched - no new Animation
Blueprint work needed, matching this project's general preference for driving gameplay
reactions from C++ rather than growing the ABP graph.

## 5. Attack/Defense power-bar system

Ported 1:1 from `fight_scene_demo.html` - it's already tuned and demoed there, no need
to redesign the math, only to reimplement it in C++:

- **Geometry**: three radii measured from the bar's center - `GreenHalf`, `BlueOuter`,
  `RedOuter` (which also defines the bar's own half-width/edge, so total marker travel
  = `2 * RedOuter`, center = `RedOuter`).
- **Marker motion while held**: cubic ease-in speed ramp,
  `speed(t) = V0 + (Vmax - V0) * (t / RampTime)^3`, bouncing between the two edges
  (reflect at 0 and at the span). Once `speed >= 0.85 * Vmax` the bar reads "BOOST!".
- **Power on release** - piecewise-linear in distance-from-center `d`:
  - `d <= GreenHalf`: `power = 100 - 29 * (d / GreenHalf)` -> 71-100 ("GREEN"; >=97 is
    "GREEN-PERFECT")
  - `GreenHalf < d <= BlueOuter`: `power = 70 - 39 * t` -> 31-70 ("BLUE")
  - `BlueOuter < d <= RedOuter`: `power = 30 - 30 * t` -> 0-30 ("RED")
- **Attack resolve**: `baseHit = max(1, ATK - Opponent.DEF)`;
  `mult = 0.2 + (MaxMult - 0.2) * (power/100)`; `dmg = ceil(baseHit * mult)`;
  GREEN-PERFECT adds +25% on top.
- **Defense resolve**: `incoming = max(1, Opponent.ATK - DEF)`;
  `mitigation = power/100`; `dmg = ceil(incoming * (1 - mitigation))`;
  GREEN-PERFECT ("PERFECT GUARD") = 0 damage taken.
- `V0`/`Vmax`/`RampTime`/the 3 zone radii, and `ATK`/`DEF`, are all driven by the
  boxer's equipped loadout (§6) - same role the prototype's "weapon" (Attack bar) and
  "armor" (Defense bar) played.

**Implementation**: a small, dependency-light C++ header (e.g. `PowerBarMath.h`, no
`.cpp` needed) exposing `FPowerBarZones` (the 3 radii) plus free functions
`ComputePower(Pos, Zones)` / `ZoneOf(Pos, Zones)` - callable identically from the HUD
widget's held-button tick, from `ABoxerCharacter`, and from the opponent's own
release-timing policy. Mirrors `UWilsonMazeGenerator`'s "pure logic, minimal engine
dependency" style already used in this project.

## 6. Loadout / opponent data

Reskinned 1:1 from the prototype's Weapon/Armor/Talent/Enemy maps:

- **Gloves** (was Weapon) - ATK bonus + Attack-bar `V0`/`Vmax`/`RampTime`/`MaxMult` +
  zone radii. E.g. "Quick Wraps" (fast/narrow, like Twin Daggers), "Power Gloves"
  (slow/wide, like War Hammer), "Training Gloves" (baseline).
- **Stance** (was Armor) - DEF bonus + Defense-bar `V0`/`Vmax`/`RampTime` + zone radii.
  E.g. "Guard Up" (steady/wide, like Guardian Plate), "Bob & Weave" (risky/narrow, like
  Swift Leather), "Neutral Stance" (baseline).
- **Perk** (was Talent) - e.g. "Iron Chin" (flat +10% mitigation floor, like Guardian),
  "Counter Puncher" (+15% power but 0 dmg on RED, like Berserker), "Focus" (both
  `RampTime`s x1.25).
- **Opponent** (was Enemy) - HP/ATK/DEF plus its own bar tuning driving the AI release
  policy (§3) - higher-tier opponents release closer to center more often.

**Implementation**: `USTRUCT`s (`FBoxerGloveData`, `FBoxerStanceData`,
`FBoxerPerkData`, `FOpponentData`) loaded from JSON catalogs (e.g.
`Content/BoxingData/Gloves.json`, `Stances.json`, `Perks.json`, `Opponents.json`) via a
small subsystem, mirroring `UStoreSubsystem`/`FStoreItemData`'s existing
JSON-catalog-on-a-`UGameInstanceSubsystem` pattern exactly - designers edit JSON, no
Blueprint/DataAsset per item needed.

## 7. Camera

Boxing needs a dedicated fight camera, separate from `AToggleCameraCharacter`'s
first/third-person free-look toggle used elsewhere in this project - propose
**`ABoxingCameraRig`** (plain Actor, SpringArm + Camera, no player input) that the
fight's director possesses/blends to for the match's duration:

- **Bell** (fight start): wide static two-shot matching `battle.jpg` - camera to the
  side of the mat (not down its long axis), both boxers face-to-face in profile,
  ring/ropes in frame, held 2-3s, then blend to Ready.
- **Ready / between exchanges**: medium shoulder-height shot, same side-on face-to-face
  framing as Bell just tighter, favoring whichever boxer currently has an active bar
  (that boxer reads slightly larger/more centered) without cutting to a different axis.
- **Charging punch-in**: slow dolly-in toward the attacking boxer over the hold
  duration, timed to the bar's own `RampTime` (camera "leans in" as the bar approaches
  BOOST) - a plain per-tick Lerp on the SpringArm's `TargetArmLength`/socket offset, no
  Sequencer needed, matching this project's hand-rolled camera code style (see
  `AMazeVisualizerActor::UpdateMinimapView`'s Lerp-based framing).
- **Punch resolve**: a fast ~0.15s punch-in + camera shake on release, scaled by the
  punch's power % (GREEN-PERFECT = biggest kick).
- **Guard resolve**: smaller/quicker shake than a punch-resolve; near-zero on PERFECT
  GUARD (reads as "nothing got through").
- **KO**: on the losing boxer's HP hitting 0, blend to a slow 2-3s orbit around the
  fallen boxer (optionally dip time dilation to ~0.5 briefly for weight), then cut to
  the Result screen.
- **Victory**: orbit around the winning boxer (Idle stand-in per §4) while the Result
  screen appears.

## 8. HUD / input

Reuse this project's established "self-building C++ UMG widget" pattern (see
`UMazeControlWidget`/`UMazeMinimapWidget` - `RebuildWidget()` in C++, no UMG Designer
layout work) rather than hand-laying-out a new widget Blueprint. Propose
**`UBoxingHUDWidget : UUserWidget`**:

- Player HP bar (top-left) / Opponent HP bar (top-right) - matches the HTML's
  `.hpbar`/`.hpfill`.
- Attack bar + "HOLD to punch" button (bottom-left), Defense bar + "HOLD to guard"
  button (bottom-right) - the HTML's two `.track`/`.marker`/`button.hold` groups,
  driven every tick from `ABoxerCharacter`'s exposed `FPowerBarZones` + current marker
  position.
- **Buttons**: `UButton::OnPressed`/`OnReleased` bound in C++ (mirrors the HTML's
  `mousedown`/`mouseup`) - this is exactly what "player click attack and defence
  button" means here. Only the currently-relevant button is enabled at a time (Attack
  while it's the player's turn to punch, Defense while guarding an incoming punch),
  matching the prototype's `atkBtn.disabled`/`defBtn.disabled` toggling.
- Battle log panel (optional for v1, matches the HTML's `#log`) - nice for
  debugging/juice, not required for the core loop.

## 9. Turn flow / state machine

Directly ports the HTML's alternating loop (see the `onRelease` handlers in
`main_battle_scene_demo.html`):

1. **Bell** -> camera establishing shot -> **PlayerAttack** (Attack bar enabled,
   Defense disabled).
2. Player holds+releases the Attack bar -> resolve damage to Opponent (§5) -> play the
   punch montage (§4) -> camera punch-in+shake (§7) -> if Opponent HP <= 0, go to
   **Win**.
3. Else -> **PlayerDefend** (Defense bar enabled, Attack disabled) - the opponent is
   swinging back; its AI release timing (§3/§6) determines the incoming hit's
   underlying strength, but the *player's* Defense-bar hold/release is what determines
   how much of it lands.
4. Player holds+releases the Defense bar -> resolve damage to Player (§5) -> play the
   hit-react/guard montage (§4) -> camera shake (§7) -> if Player HP <= 0, go to
   **Lose**.
5. Else -> back to step 2.
6. **Win**/**Lose** -> KO or Victory animation+camera (§4/§7) -> Result screen
   (rematch / return to menu).

## 10. Suggested new classes

| Class | Role |
| --- | --- |
| `ABoxerCharacter` | Shared boxer Character - HP/state/montage playback (§3, §4) |
| `AFightDirector` | Owns the turn state machine (§9), owns/possesses `ABoxingCameraRig`, ticks the opponent's bar policy |
| `ABoxingCameraRig` | The fight-only camera (§7) |
| `UBoxingHUDWidget` | HUD - HP bars + attack/defense bars + hold buttons (§8) |
| `PowerBarMath.h` (`FPowerBarZones` + free functions) | Shared bar math (§5), no `.cpp` needed |
| `FBoxerGloveData` / `FBoxerStanceData` / `FBoxerPerkData` / `FOpponentData` + a small JSON-catalog subsystem | Loadout/opponent data (§6) |

## 11. Open questions / follow-ups

- Guard and Win animations need sourcing (§4) before this feels like boxing rather than
  "two mannequins standing near each other" - a content task, not a code task; doesn't
  block building the rest of the system against the stand-ins noted above.
- Local 2-player PvP is out of scope for v1 (opponent is AI-timed per §3/§6) - note if
  you want that later, since it'd move the opponent's Defense-bar off "AI timing" onto
  a second real input instead.
- Ring/arena art is placeholder-only per §2 (BasicShapes-cube convention, matching the
  maze) - fine for building/testing the system, needs real art later.
