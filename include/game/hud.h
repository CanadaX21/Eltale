#ifndef GAME_HUD_H
#define GAME_HUD_H

/* The functions of src/game/hud.c, declared once for every file that calls them. */

#include "common.h"

void ResetHUD(void);
void DrawHUD(mkst* dyn);
void DrawHPMPGauge(ActorTransform* brian);
void DrawSpiritCounters(mkst* dyn, BrianData1* stats);
void DrawCompass(mkst* dyn);
void DrawSpellCross(mkst* dyn, PlayerState* player);
void DrawSpellBanner(PlayerState* state);
void SpiritGainWindow(mkst* dyn, PlayerState* player);
void UpdateTalkMarker(void);
void DrawSpiritMarker(void);
void DrawObjectMarker(void);
void DrawSignMarker(void);
void DrawEnemyMarker(u16 enemy);
void DrawEscapeLabel(void);
void UpdateStatusVisuals(MonsterBattleData* actor, u16 statusBits, u16 kind, s16 duration);
void DrawMonsterStatusIcons(MonsterBattleData* actor);
void ClearStatusEffects(MonsterBattleData* actor);
void func_80023DEC(void);
void func_80023DF4(void);
void func_80023DFC(void);
void func_80023E04(void);
void func_80023E0C(void);
void DrawMonsterCard(void);
void Init_MISS_Icon(void);
void Setup_MISS_Icon(MonsterBattleData* actor);
void Draw_MISS_Icon(void);
void DrawSpellLabel(s32 x, s32 y, u8* path, s32 presses, u16 context);
void DrawHUDText(s32 x, s32 y, s32 colour, u8* text);
s32 GetHUDTextWidth(u8* text);
void DrawNumber(u16 flags, s32 x, s32 y, s32 value);   /* draw a number */
void func_80024A4C(void);
void LoadHUDTexture(HudImage* tex, s32 s, s32 t, s32 width, s32 height);
void DrawClippedTexRect(s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy);
void DrawTexturePart(HudImage* image, s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy);
void Inventory_Init(void);
s32 InventoryHasRoom(void);
u32 CountItem(u8 itemID);
u32 CountInventory(void);
void AddItemToInventory(u8 itemID);
s32 CheckIfItemUsable(u8 itemIndex);
void UseItem(u8 itemID, MonsterBattleData* brian);
s32 RemoveKeyItem(u16 id);
void ItemMenu(void);
s32 NoEffectItem(ActorTransform* pos, ItemData* item);
s32 CheckIfHealingNeeded(ActorTransform* brian, ItemData* item);
s32 CheckIfTeleportPossible(ActorTransform* brian, ItemData* item);
u8 CheckIfSpellItemUsable(ActorTransform* brian, ItemData* item);
void UseNoEffectItem(MonsterBattleData* user, ItemData* item);
void UseHealingItem(MonsterBattleData* BrianData, ItemData* item);
void UseWings(ActorTransform* brian, ItemData* item);
void UseSpellItem(MonsterBattleData* brian, ItemData* item);
void DrawItemBox(s32 ulx, s32 uly, s32 width, s32 height);

/* The state src/game/hud.c defines (.bss), in its storage order: the order of these declarations is the
 * unit's .bss layout (include/bss_order.h). */
extern u16 gTalkMarkerBobPhase;   /* the talk marker's bob phase */
extern u16 gSpiritGainFlags;
extern s32 gClipLeft;   /* clip rectangle: left */
extern s32 gClipTop;   /* top */
extern s32 gClipRight;   /* right */
extern u16 gSpiritMarkerBobPhase;   /* the icon's bob phase */
extern s32 gClipBottom;   /* bottom */
extern MissIcon gMISSData[8];
extern u32 gSpiritFlashTimer;   /* unsigned: DrawSpiritCounters fades it with a divu */
extern u32 gSpiritGainElement;
extern OddVector gHudVector;   /* (Quest D_8008C748) a scratch: the compass marker's rotated offset (x, y; DrawCompass)
                                   and a status visual's effect size (UpdateStatusVisuals) */
extern u16 gMarkerBobPhase;   /* the object marker icon's bob phase */
extern u16 gSignMarkerBobPhase;   /* the sign marker icon's bob phase */
extern u16 gEnemyMarkerBobPhase;   /* the enemy marker's bob phase */
extern u16 gItemBarSlide;   /* the item bar's slide */
extern u16 gHudSlideState;   /* 1 = sliding in, 2 = sliding out */
extern u16 gHudHpBarWidth;   /* the HP bar width drawn last (eases toward the stat) */
extern u16 gHudMpBarWidth;   /* the MP bar width drawn last */
extern u16 gSpellBannerFade;   /* spell-name banner fade-in frames */
extern u16 gSpellCrossFlags;
extern u16 gSpellBannerTimer;
extern u16 gSpiritGainGlow;   /* the spirit-gain window's glow frames */
extern u16 gMonsterCardTimer;   /* the enemy info window's slide-in counter */
extern s32 D_8008C164;   /* hud.c does not use it; lighting.c declares and reads it */
extern s32 gFieldFreeze;
extern s32 gInputFreeze;
extern s32 gPendingSound;   /* a sound waiting to start, -1 = none */
extern s32 gScreenOffsetX;   /* the 2D draw offset x */
extern s32 gScreenOffsetY;   /* the 2D draw offset y */
extern s32 gMarkerScreenY;   /* the marker's screen y (0xF0 = off screen) */
extern u16 gSpiritGlowAngle;
extern s32 D_8008C188[2];   /* unreferenced (8 bytes: gItemLightRingSize, 8-aligned, starts after them) */
extern OddVector gItemLightRingSize;   /* the item lights' effect size (UseHealingItem, UseWings) */
extern s32 gItemScroll;
extern s32 gItemCursor;
extern s32 D_8008C1A4;
extern s32 D_8008C1A8[2];   /* unreferenced (8 bytes: gVisibleInvItemIDs, 8-aligned, starts after them) */
extern u8 gVisibleInvItemIDs[8];
extern u8 D_8008C1B8[0x800];   /* the eight cached item icons, 16 x 16 CI8 (0x100 bytes each) */
extern u8 gInventory[151];   /* the inventory: item ids, 0xFF = empty (Quest gInventory): 150 slots
                                            the item code walks, and a 151st byte Inventory_Init clears and the
                                            save copies with them (0x97, SaveData.inventory) */

/* The data src/game/hud.c defines. */
extern const char D_8006F530[];

typedef struct {
    s16 s;
    s16 t;
    u16 w;
    u16 h;
} IconRect;                          /* a rectangle in the icon sheet */

/* Storage the C does not define yet (assembly or binary), declared here for its type. */
extern IconRect gStatusIcons[];   /* the status icons */

#endif
