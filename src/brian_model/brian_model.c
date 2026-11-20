#include "common.h"
#include "brian.h"

/* Brian's model: its first 0x3B0 bytes, as InitBrian loads them to brian_model_VRAM (0x80206000 in the original).
 * The rest of the model (textures, vertex runs, display lists) is written from the ROM at extraction
 * (assets/brian_model/), and each animation's keys and part table are in its own file (assets/brian_anim_NN/),
 * loaded by LoadBrianAnimation to the animation buffer while that animation plays: an entry here points into
 * its file as loaded there. Generated once from the original ROM (a walk of the model and each
 * file). */

/* +0x000: the display lists by index (a part's dlist, the per-frame table's values) */
Gfx* gBrianModelDLists[25] = {
    gBrianModelDL_00, gBrianModelDL_01, gBrianModelDL_02, gBrianModelDL_03,
    gBrianModelDL_04, gBrianModelDL_05, gBrianModelDL_06, gBrianModelDL_07,
    gBrianModelDL_08, gBrianModelDL_09, gBrianModelDL_10, gBrianModelDL_11,
    gBrianModelDL_12, gBrianModelDL_13, gBrianModelDL_14, gBrianModelDL_15,
    gBrianModelDL_16, gBrianModelDL_17, gBrianModelDL_18, gBrianModelDL_19,
    gBrianModelDL_20, gBrianModelDL_21, NULL, gBrianModelDL_23,
    gBrianModelDL_24,
};

/* +0x064: his animations by ID (gBrianAnims[0]; ActorTransform.animID): the part count, the length in frames (a
 * state takes it as its timer), the part table in that animation's file, the display-list table. The IDs the
 * code sets are named in include/game/player.h (BRIAN_ANIM_...); 4, 5, 6 and 19 are structurally verified animations
 * with no identified normal-game playback path (no code sets them; they load and play when set). */
AnimEntry gBrianModelAnims[30] = {
    { ARRAY_COUNT(gBrianAnim00Parts),  70, gBrianAnim00Parts, gBrianModelDLists },   /*  0 BRIAN_ANIM_STAND: standing in the field (gBrianIdleAnim with HP at least a third) */
    { ARRAY_COUNT(gBrianAnim01Parts), 300, gBrianAnim01Parts, gBrianModelDLists },   /*  1 BRIAN_ANIM_WALK: walking (BrianState_Walk; rate want * 14 + 3); stepping to a door */
    { ARRAY_COUNT(gBrianAnim02Parts), 300, gBrianAnim02Parts, gBrianModelDLists },   /*  2 BRIAN_ANIM_RUN: running (BrianState_Run) */
    { ARRAY_COUNT(gBrianAnim03Parts),  30, gBrianAnim03Parts, gBrianModelDLists },   /*  3 BRIAN_ANIM_CAST: casting a spell at a target (BrianStartCast) */
    { ARRAY_COUNT(gBrianAnim04Parts),  13, gBrianAnim04Parts, gBrianModelDLists },   /*  4 structurally verified, no normal-game playback path identified (no code sets it) */
    { ARRAY_COUNT(gBrianAnim05Parts),  16, gBrianAnim05Parts, gBrianModelDLists },   /*  5 structurally verified, no normal-game playback path identified (no code sets it) */
    { ARRAY_COUNT(gBrianAnim06Parts),  27, gBrianAnim06Parts, gBrianModelDLists },   /*  6 structurally verified, no normal-game playback path identified (no code sets it) */
    { ARRAY_COUNT(gBrianAnim07Parts),  20, gBrianAnim07Parts, gBrianModelDLists },   /*  7 BRIAN_ANIM_DODGE: an attack missed him (StartBrianDodge) */
    { ARRAY_COUNT(gBrianAnim08Parts),  15, gBrianAnim08Parts, gBrianModelDLists },   /*  8 BRIAN_ANIM_HIT: hit (DamageBrian) */
    { ARRAY_COUNT(gBrianAnim09Parts), 300, gBrianAnim09Parts, gBrianModelDLists },   /*  9 BRIAN_ANIM_DEFEATED: knocked out at HP 0 (BrianState_KnockedDown) */
    { ARRAY_COUNT(gBrianAnim10Parts),  30, gBrianAnim10Parts, gBrianModelDLists },   /* 10 BRIAN_ANIM_STAFF_SWING: the staff swing (StartBrianStaffSwing) */
    { ARRAY_COUNT(gBrianAnim11Parts),  36, gBrianAnim11Parts, gBrianModelDLists },   /* 11 BRIAN_ANIM_STAFF_COMBO: the combo's second swing (BrianState_StaffSwing) */
    { ARRAY_COUNT(gBrianAnim12Parts),  30, gBrianAnim12Parts, gBrianModelDLists },   /* 12 BRIAN_ANIM_SIGN_START: turning to read a sign, then 13 (UpdateBrian) */
    { ARRAY_COUNT(gBrianAnim13Parts),  60, gBrianAnim13Parts, gBrianModelDLists },   /* 13 BRIAN_ANIM_SIGN_LOOP: reading a sign, after 12 */
    { ARRAY_COUNT(gBrianAnim14Parts),  30, gBrianAnim14Parts, gBrianModelDLists },   /* 14 BRIAN_ANIM_SIGN_END: after a sign's message (BrianState_FaceAndMessage) */
    { ARRAY_COUNT(gBrianAnim15Parts),  32, gBrianAnim15Parts, gBrianModelDLists },   /* 15 BRIAN_ANIM_STOP: stopping from speed, from its frame 3 (BrianState_Walk, BrianState_Run) */
    { ARRAY_COUNT(gBrianAnim16Parts),  83, gBrianAnim16Parts, gBrianModelDLists },   /* 16 BRIAN_ANIM_VICTORY: a battle won (UpdateBrian) */
    { ARRAY_COUNT(gBrianAnim17Parts),  27, gBrianAnim17Parts, gBrianModelDLists },   /* 17 BRIAN_ANIM_BATTLE_STAND: standing in battle (gBrianIdleAnim) */
    { ARRAY_COUNT(gBrianAnim18Parts),  35, gBrianAnim18Parts, gBrianModelDLists },   /* 18 BRIAN_ANIM_BATTLE_START: a battle's start (SetupBrianForBattle) */
    { ARRAY_COUNT(gBrianAnim19Parts),  20, gBrianAnim19Parts, gBrianModelDLists },   /* 19 structurally verified, no normal-game playback path identified (no code sets it) */
    { ARRAY_COUNT(gBrianAnim20Parts),  90, gBrianAnim20Parts, gBrianModelDLists },   /* 20 BRIAN_ANIM_DOOR: through a door model whose door's flags have bit 0 clear (animCue; root motion) */
    { ARRAY_COUNT(gBrianAnim21Parts), 105, gBrianAnim21Parts, gBrianModelDLists },   /* 21 BRIAN_ANIM_DOOR_FLAG1: through a door model whose door's flags have bit 0 set (animCue; root motion) */
    { ARRAY_COUNT(gBrianAnim22Parts),  10, gBrianAnim22Parts, gBrianModelDLists },   /* 22 BRIAN_ANIM_USE_LOOP: using an item or a spell on himself, after 23 */
    { ARRAY_COUNT(gBrianAnim23Parts),  18, gBrianAnim23Parts, gBrianModelDLists },   /* 23 BRIAN_ANIM_USE_START: using an item or a spell on himself, then 22 (StartBrianUseItem, BrianStartCast) */
    { ARRAY_COUNT(gBrianAnim24Parts),  24, gBrianAnim24Parts, gBrianModelDLists },   /* 24 BRIAN_ANIM_USE_END: after an item or a spell on himself (BrianState_UseItem, BrianState_Cast) */
    { ARRAY_COUNT(gBrianAnim25Parts),  24, gBrianAnim25Parts, gBrianModelDLists },   /* 25 BRIAN_ANIM_STAND_LOW_HP: standing with HP under a third (gBrianIdleAnim) */
    { ARRAY_COUNT(gBrianAnim26Parts),  53, gBrianAnim26Parts, gBrianModelDLists },   /* 26 BRIAN_ANIM_DOOR_KIND3: leaving through a door of kind 3, 75 frames (BrianWalkThroughDoor) */
    { ARRAY_COUNT(gBrianAnim27Parts),  40, gBrianAnim27Parts, gBrianModelDLists },   /* 27 BRIAN_ANIM_NO_TARGET: A with nothing to act on, or a sign whose record has flag 2 (UpdateBrian) */
    { ARRAY_COUNT(gBrianAnim28Parts),  41, gBrianAnim28Parts, gBrianModelDLists },   /* 28 BRIAN_ANIM_PICK_UP: taking an item from a map object, then 29 (StartBrianPickUp; root motion) */
    { ARRAY_COUNT(gBrianAnim29Parts),  50, gBrianAnim29Parts, gBrianModelDLists },   /* 29 BRIAN_ANIM_PICK_UP_END: after 28 */
};

/* +0x1CC: part 0's display list for each drawn frame, read [totalAnimFrames % 80] by DrawModelPart where the part's
 * dlistFrames points here (animations 0, 1, 2, 13, 25, 27): list 0, with 23 and 24 at 20, 21, 28, 29. */
s32 gBrianModelDlFrames[80] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    23, 24, 0, 0, 0, 0, 0, 0, 23, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* +0x30C: raw data: 41 words that no part points at. By their values more of part 0's per-frame list indices
 * as if the table ran on to 121 entries (23 and 24 at its entries 87, 88, 89, 90, 91, 92); no code or record is known to read them. */
RawWord gBrianModelRaw_0030C[41] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000017,
    0x00000017, 0x00000017, 0x00000017, 0x00000017, 0x00000018, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000,
};
