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
s32 CheckIfInventoryFull(void);
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

#endif
