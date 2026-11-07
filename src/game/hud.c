#include "common.h"
extern s32 gScreenOffsetX;   /* the 2D draw offset x */
extern s32 gScreenOffsetY;   /* the 2D draw offset y */
extern s32 gClipLeft;   /* clip rectangle: left */
extern s32 gClipTop;   /* top */
extern s32 gClipRight;   /* right */
extern s32 gClipBottom;   /* bottom */
extern Gfx gItemBoxGfx[];
extern unk803a9190s gWingData[];
extern PendingWarp gPendingWarp;
extern s32 gGameState;
extern s32 gModeCountdown;
extern f32 gBrianSpawnX;
extern f32 gBrianSpawnZ;
extern f32 gBrianSpawnYaw;
extern s32 gArrivalFlags;
extern u16 gTransitionStyle;
void SpawnDamageNumber(MonsterBattleData* target, s32 kind, s32 damage);
extern BaseItemLightRingData gBaseItemLightRingData;
extern OddVector gItemLightRingSize;    /* the item lights' effect size (UseHealingItem, UseWings) */
extern EffectSpawnDef gItemEffectModels[];
void PlaySound(u8 delay, s8 id, u8 level);
void StartBrianUseItem(u8 itemID);
extern u16 gBattleState;   /* battle state flags (0x100 = boss battle) */
extern u8 item_icon_palettes_ROM_START[];
extern u16 gInventoryPalette[];
void dma_write(u8* devAddr, void* vaddr, u32 nbytes);
extern u8 gVisibleInvItemIDs[8];
extern u8 gInventory[150];   /* the inventory: item ids, 0xFF = empty (Quest gInventory) */
extern s32 gItemScroll;
extern s32 gItemCursor;
extern s32 D_8008C1A4;
extern u16 gStatusIconBits[];
extern u16 gStatusFxBits[];
extern StatusFxDef gStatusFxDefs[];
extern OddVector gHudVector;    /* (Quest D_8008C748) a scratch: the compass marker's rotated offset (x, y; DrawCompass)
                                   and a status visual's effect size (UpdateStatusVisuals) */
extern EffectSpawnDef gStatusFxModels[];
void*SpawnEffectObject(u16 colour, u16 duration, f32 x, f32 y, f32 z, f32 angle, EffectSpawnDef* model, OddVector* size, ActorTransform* actor);
extern PlayerState gPlayerState;
extern u16 gMapFlags;   /* map flags (Quest D_80084EE0): 0xA = no teleporting from here */
extern Submap* gSubmapHeader;
extern ItemData gItemData[];
extern s32 (*gItemCheckHandlers[])(ActorTransform*, ItemData*); /* item use handlers (Quest D_8004D480) */
extern void (*gItemUseHandlers[])(s32, ItemData*, u8, s32);
s32 count_digits(s32 arg0, u8* arg1, u16 arg2);
void DrawClippedTexRect(s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy);
extern Gfx* gDisplayListHead;
extern u8 gStatusSymbolsPalette[];
extern HudImage gStatusSymbolsImage;
s32 WorldToScreen(f32 arg0, f32 arg1, f32 arg2, s32 *arg3, s32 *arg4);
s16 sins(u16 angle);         /* sins */
void DrawTexturePart(HudImage* image, s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy);
void LoadHUDTexture(HudImage* tex, s32 s, s32 t, s32 width, s32 height);
extern f64 D_8006F560;
extern MissIcon gMISSData[8];

extern s32 gFieldFreeze;
extern s32 gInputFreeze;
extern s32 gPendingSound;
extern u16 gMonsterCardTimer;              /* the enemy info window's slide-in counter */
extern u16 gSpellBannerTimer;
extern u16 gSpellCrossFlags;
extern u16 gSpellBannerFade;              /* spell-name banner fade-in frames */
extern u8** gSpellNames[];   /* spell names: per element, a pointer per spell */
void DrawHUDText(s32 x, s32 y, s32 colour, u8* text);
s32 GetHUDTextWidth(u8* text);
void DrawNumber(u16 flags, s32 x, s32 y, s32 value);   /* draw a number */
extern HudImage gHudImage;   /* the HUD image (compass, HP/MP frame) */
extern u32 gSpiritFlashTimer;          /* unsigned: DrawSpiritCounters fades it with a divu */
extern u16 gSpiritGainFlags;
void ResetHUD(void) {
    gFieldFreeze = 0;
    gInputFreeze = 0;
    gPendingSound = -1;
    gScreenOffsetX = gScreenOffsetY = 0;
    gClipLeft = gClipTop = 0;
    gClipRight = 320;
    gClipBottom = 240;
    gMonsterCardTimer = 0;
    gSpellBannerTimer = 0;
    gSpellBannerFade = gSpellCrossFlags = 0;
    gSpiritFlashTimer = 0;
    gSpiritGainFlags = 0;
}

extern s32 gMatrixHead;               /* matrices used this frame */
extern Mtx D_2000000[];
extern Mtx D_2000040;                /* segment 2: the HUD's orthographic projection */
extern Gfx gHudRenderSetup[];             /* HUD render setup */
extern u16 gItemBarSlide;               /* the item bar's slide */
extern u16 gHudSlideState;               /* 1 = sliding in, 2 = sliding out */
extern s32 gTalkTarget;               /* which talk / pick-up marker shows (UpdateTalkMarker .. DrawSignMarker) */
extern Gfx gTalkMarkerIcon[];              /* the talk marker icon (64 x 29; UpdateTalkMarker, DrawSignMarker) */
extern Spirit* gSpiritTarget;
extern MapObject* gObjectTarget;
extern SignSlot* gSignTarget;
extern s32 gSpellLightState;
extern s32 gEnemyCount;               /* enemies in battle */
extern EnemyAction gEnemies[];
void DrawHPMPGauge(ActorTransform* brian);
void DrawSpiritCounters(mkst* dyn, BrianData1* stats);
void DrawCompass(mkst* dyn);
void DrawSpellCross(mkst* arg0, PlayerState* player);
void DrawSpellBanner(PlayerState* player);
void SpiritGainWindow(mkst* arg0, PlayerState* player);
void UpdateTalkMarker(void);
void DrawSpiritMarker(void);
void DrawObjectMarker(void);
void DrawSignMarker(void);
void DrawEnemyMarker(u16 enemy);
void DrawEscapeLabel(void);
void DrawMonsterStatusIcons(MonsterBattleData* actor);
void DrawMonsterCard(void);
void Draw_MISS_Icon(void);
void DrawHUD(mkst* dyn) {
    s32 n;
    EnemyAction* enemy;

    gSPMatrix(gDisplayListHead++, &D_2000040, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    guTranslate(&dyn->unkC0[gMatrixHead], -160.0f, -120.0f, 0.0f);
    gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
    gMatrixHead++;
    gClipRight = 320;
    gClipBottom = 240;
    gSPDisplayList(gDisplayListHead++, gHudRenderSetup);
    if ((gGameState & 2) && gHudSlideState != 1) {
        gHudSlideState = 1;
    } else if (!(gGameState & 2) && gHudSlideState == 1) {
        gHudSlideState = 2;
    }
    if (!(gGameState & 0xC00)) {
        DrawHPMPGauge(&gPlayerState.brian.transform);
        DrawSpiritCounters(dyn, gPlayerState.stats);
        if (gHudSlideState == 1) {
            if (++gItemBarSlide > 12) {
                gItemBarSlide = 12;
            }
        } else if (gHudSlideState == 2) {
            if (--gItemBarSlide == 0) {
                gHudSlideState = 0;
            }
        }
    }
    if ((gMapFlags & 0xD) && !(gBattleState & 1) && !(gSubmapHeader->flags & 8) && !(gGameState & 2)) {
        DrawCompass(dyn);
    }
    if ((gBattleState & 1) && gMonsterCardTimer != 0) {
        DrawMonsterCard();
        gMonsterCardTimer--;
    }
    gScreenOffsetX = gScreenOffsetY = 0;
    if (gBattleState & 1) {
        Draw_MISS_Icon();
        if (!(gBattleState & 2) && gPlayerState.staffTarget != 0 && !(gPlayerState.brian.transform.unk60 & 1) && !(gSpellLightState & 1)) {
            DrawEnemyMarker(gPlayerState.staffTarget - 1);
        }
        if ((gPlayerState.flags & 0x20) && !(gGameState & 0x800)) {
            DrawEscapeLabel();
        }
        DrawMonsterStatusIcons(&gPlayerState.brian);
        n = gEnemyCount;
        enemy = gEnemies;
        while (n != 0) {
            if (enemy->battleData.transform.modelID != -1) {
                DrawMonsterStatusIcons(&enemy->battleData);
                n--;
            }
            enemy++;
        }
    } else if (!(gGameState & 0x87) && !(gPlayerState.brian.transform.unk60 & 1)) {
        if (gTalkTarget != 0) {
            UpdateTalkMarker();
        } else if (gSpiritTarget != 0 && !(gGameState & 8)) {
            DrawSpiritMarker();
        } else if (gObjectTarget != 0) {
            DrawObjectMarker();
        } else if (gSignTarget != 0) {
            DrawSignMarker();
        }
    }
    if (gGameState & 4) {
        DrawSpellCross(dyn, &gPlayerState);
    } else if (gGameState & 8) {
        SpiritGainWindow(dyn, &gPlayerState);
    }
    if (gSpellBannerTimer != 0) {
        DrawSpellBanner(&gPlayerState);
        gSpellBannerTimer--;
    }
}
extern u16 gHudHpBarWidth;   /* the HP bar width drawn last (eases toward the stat) */
extern u16 gHudMpBarWidth;   /* the MP bar width drawn last */
void DrawHPMPGauge(ActorTransform* brian) {
    BrianData1* stats;
    s32 bar;
    s32 d;
    s32 sq;
    u16 w;

    gScreenOffsetX = 28;
    gScreenOffsetY = 24;
    if (gHudSlideState != 0) {
        sq = gItemBarSlide * gItemBarSlide;
        gScreenOffsetX -= sq * 150 / 144;
    }
    stats = brian->base.player;
    DrawTexturePart(&gHudImage, 0, 1, 62, 17, 32, 39, 0x400, 0x400);
    DrawTexturePart(&gHudImage, 0, 18, 62, 18, 32, 56, 0x400, 0x400);
    bar = stats->currHP * 56 / stats->maxHP;
    d = bar - gHudHpBarWidth;
    if (d >= 2 || d < -1) {
        gHudHpBarWidth += d >> 1;
    } else {
        gHudHpBarWidth = bar;
    }
    w = gHudHpBarWidth;
    if (w) {
        DrawTexturePart(&gHudImage, 3, 11, w, 5, 94, 39, 0x400, 0x400);
    }
    bar = stats->currMP * 56 / stats->maxMP;
    d = bar - gHudMpBarWidth;
    if (d >= 2 || d < -1) {
        gHudMpBarWidth += d >> 1;
    } else {
        gHudMpBarWidth = bar;
    }
    w = gHudMpBarWidth;
    if (w) {
        DrawTexturePart(&gHudImage, 3, 21, w, 5, 94, 44, 0x400, 0x400);
    }
    LoadHUDTexture(&gHudImage, 32, 29, 128, 10);
    DrawNumber(3, 12, 0, stats->currHP);
    DrawNumber(1, 54, 0, stats->maxHP);
    DrawNumber(3, 12, 27, stats->currMP);
    DrawNumber(1, 54, 27, stats->maxMP);
    DrawClippedTexRect(47, 0, 8, 10, 112, 29, 0x400, 0x400);
    DrawClippedTexRect(47, 27, 8, 10, 112, 29, 0x400, 0x400);
}

extern u32 gSpiritGainElement;
extern u16 gSpiritGlowAngle;
extern u8 gElementColours[];
extern s32 gSpiritGlowBits[4];
extern Gfx gSpiritGlowGfx[];             /* the glow */
extern Gfx gSpiritFlashGfx[];             /* the flash */
void DrawSpiritCounters(mkst* dyn, BrianData1* stats) {
    s32 x;
    s32 i;
    u8* colour;
    f32 posX;
    f32 posY;

    gScreenOffsetX = 28;
    gScreenOffsetY = 198;
    if (gHudSlideState != 0) {
        i = gItemBarSlide * gItemBarSlide;
        gScreenOffsetX -= i * 200 / 144;
    }
    DrawTexturePart(&gHudImage, 0, 0, 12, 12, 32, 16, 0x400, 0x400);
    DrawTexturePart(&gHudImage, 22, 0, 12, 12, 44, 16, 0x400, 0x400);
    DrawTexturePart(&gHudImage, 44, 0, 12, 12, 56, 16, 0x400, 0x400);
    DrawTexturePart(&gHudImage, 66, 0, 12, 12, 68, 16, 0x400, 0x400);
    LoadHUDTexture(&gHudImage, 32, 29, 128, 10);
    x = 6;
    for (i = 0; i < 4; i++) {
        DrawNumber(1, x, 6, stats->spirits[i]);
        x += 22;
    }
    for (i = 0; i < 4; i++) {
        if (gSpiritGlowBits[i] & gPlayerState.flags) {
            /* QUIRK: steps one byte per element, not one RGBA entry (the flash below uses * 4): fire (255,0,0) and
               water (0,0,255) come out right, earth reads 0,0,0 and wind 0,255,200 */
            colour = &gElementColours[i];
            posX = i * 22.0f + 5.0 + (f32)gScreenOffsetX;
            posY = gScreenOffsetY + 5.0f;
            guTranslate(&dyn->unkC0[gMatrixHead], posX, posY, 0.0f);
            gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gMatrixHead++;
            gDPPipeSync(gDisplayListHead++);
            gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], (sins(gSpiritGlowAngle) >> 10) + 0x88);
            gSPDisplayList(gDisplayListHead++, gSpiritGlowGfx);
        }
    }
    gSpiritGlowAngle += 0x444;
    if (gSpiritFlashTimer != 0) {
        colour = &gElementColours[gSpiritGainElement * 4];
        posX = gSpiritGainElement * 22.0f + 5.0 + (f32)gScreenOffsetX;
        posY = gScreenOffsetY + 5.0f;
        guTranslate(&dyn->unkC0[gMatrixHead], posX, posY, 0.0f);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gDPPipeSync(gDisplayListHead++);
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], gSpiritFlashTimer * 255 / 30);
        gSPDisplayList(gDisplayListHead++, gSpiritFlashGfx);
        gSpiritFlashTimer--;
    }
}
extern Camera gCamera;
extern f32 gCompassTargetX;               /* compass target x */
extern f32 gCompassTargetZ;               /* compass target z */
extern s32 gMapPageImageId;
extern s32 gMapPageMarker;
extern Gfx gCompassDialGfx[];             /* compass dial display list */
void BuildRotationMatrix(MtxF *mtx, f32 xrot, f32 yrot, f32 zrot);
f32 Atan2(f32 x, f32 z);            /* atan2 */
void rotateCoordinatesByAngle(f32 angle, Coordinates2D* vec);
f32 sqrtf(f32 x);                   /* sqrtf */
extern f64 D_8006F538;               /* -pi (hud.c does not own its .rodata yet) */
extern f64 D_8006F540;               /* 2 pi */
extern f64 D_8006F548;               /* pi */
extern f64 D_8006F550;               /* 2 pi */
void DrawCompass(mkst* dyn) {
    f32 dx;
    f32 dz;
    f32 angle;
    f32 dist;
    MtxF mtx;

    gScreenOffsetX = 260;
    gScreenOffsetY = 24;
    BuildRotationMatrix(&mtx, 0.0f, 0.0f, gCamera.yaw);
    mtx.mf[3][0] = gScreenOffsetX + 17;
    mtx.mf[3][1] = gScreenOffsetY + 17;
    mtx.mf[3][2] = 0.0f;
    guMtxF2L(mtx.mf, &dyn->unkC0[gMatrixHead]);
    gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gMatrixHead++;
    gSPDisplayList(gDisplayListHead++, gCompassDialGfx);
    DrawTexturePart(&gHudImage, 13, 10, 11, 14, 147, 0, 0x400, 0x400);
    if (gMapPageImageId < 6 && gMapPageMarker < 0 && !(gGameState & 0x80)) {
        dx = gCompassTargetX - gPlayerState.brian.transform.pos.x;
        dz = gCompassTargetZ - gPlayerState.brian.transform.pos.z;
        dist = sqrtf(dx * dx + dz * dz);
        if (dist > 10.0) {
            angle = Atan2(dx, dz) - gCamera.yaw;
            if (angle < D_8006F538) {
                angle += D_8006F540;
            } else if (angle > D_8006F548) {
                angle -= D_8006F550;
            }
            gHudVector.x = 0.0f;
            gHudVector.y = 18.0f;
            rotateCoordinatesByAngle(angle, (Coordinates2D*)&gHudVector);
            DrawTexturePart(&gHudImage, gHudVector.x + 14.0f, gHudVector.y + 13.0f, 8, 8, 0, 50, 0x400, 0x400);
        }
    }
}
extern u8 D_8004F2CC[][4];
extern Gfx D_803AA2F8[];
extern u8 gInfoWindowPalette[];
extern HudImage gInfoWindowImage;
void DrawSpellLabel(s32 x, s32 y, u8* path, s32 presses, u16 context);
extern f32 gSpellCrossFlashPos[];             /* per element: an x, y pair, read flat */
void DrawSpellCross(mkst* dyn, PlayerState* player) {
    s32 i;
    s32 upX;
    s32 leftX;
    s32 downX;
    s32 rightX;
    u8* colour;
    s32 upY;
    s32 leftY;
    s32 downY;
    s32 rightY;
    s32 winY;
    f32 x;
    f32 y;
    u8 path[12];

    gScreenOffsetX = 34;
    gScreenOffsetY = 140;
    upX = 71;
    upY = 0;
    leftX = 0;
    leftY = 24;
    downX = 71;
    downY = 49;
    rightX = 145;
    rightY = 24;
    winY = 0;
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
    if (gSpellBannerFade && (gSpellCrossFlags & 2)) {
        i = gSpellBannerFade * gSpellBannerFade;
        upX -= i * 20 / 64;
        upY -= i * 10 / 64;
        leftX -= i * 10 / 64;
        leftY += i * 20 / 64;
        downX += i * 20 / 64;
        downY += i * 10 / 64;
        rightX += i * 10 / 64;
        rightY -= i * 20 / 64;
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255 - i * 255 / 64);
    }
    if (player->spellPresses < 3) {
        DrawTexturePart(&gInfoWindowImage, upX, upY, 110, 14, 100, 0, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, leftX, leftY, 110, 14, 100, 0, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, downX, downY, 110, 14, 100, 0, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, rightX, rightY, 110, 14, 100, 0, 0x400, 0x400);
        for (i = 0; i < player->spellPresses; i++) {
            path[i] = player->path[i];
        }
        path[i] = 0;
        DrawSpellLabel(upX, upY + 3, path, i + 1, player->stats->currMP);
        path[i] = 1;
        DrawSpellLabel(leftX, leftY + 3, path, i + 1, player->stats->currMP);
        path[i] = 2;
        DrawSpellLabel(downX, downY + 3, path, i + 1, player->stats->currMP);
        path[i] = 3;
        DrawSpellLabel(rightX, rightY + 3, path, i + 1, player->stats->currMP);
    }
    gScreenOffsetX = 105;
    gScreenOffsetY = 50;
    if (gSpellBannerFade && (gSpellCrossFlags & 2)) {
        i = gSpellBannerFade * gSpellBannerFade;
        winY = i * 40 / 64;
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255 - i * 255 / 64);
    }
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
    DrawTexturePart(&gInfoWindowImage, 0, winY, 110, 14, 100, 14, 0x400, 0x400);
    upX = 55 - player->spellPresses * 6;
    for (i = 0; i < player->spellPresses; i++) {
        upY = player->path[i];
        DrawTexturePart(&gInfoWindowImage, upX, winY + 13, 11, 11, 64, upY * 11 + 36, 0x400, 0x400);
        upX += 12;
    }
    DrawSpellLabel(0, winY + 3, player->path, player->spellPresses, player->stats->currMP);
    gScreenOffsetX = 34;
    gScreenOffsetY = 140;
    if (player->spellPresses != 3) {
        gDPPipeSync(gDisplayListHead++);
        gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(gDisplayListHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
        DrawTexturePart(&gInfoWindowImage, 109, 13, 36, 36, 64, 0, 0x400, 0x400);
    }
    if (gSpellBannerFade && (gSpellCrossFlags & 1)) {
        colour = &gElementColours[player->path[player->spellPresses - 1] * 4];
        x = gSpellCrossFlashPos[player->path[player->spellPresses - 1] * 2] + 109.0f + gScreenOffsetX;
        y = gSpellCrossFlashPos[player->path[player->spellPresses - 1] * 2 + 1] + 13.0f + gScreenOffsetY;
        guTranslate(&dyn->unkC0[gMatrixHead], x, y, 0.0f);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gDPPipeSync(gDisplayListHead++);
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], gSpellBannerFade * 255 / 8);
        gSPDisplayList(gDisplayListHead++, gSpiritFlashGfx);
    }
    if (gSpellBannerFade) {
        gSpellBannerFade--;
    }
}

void DrawSpellBanner(PlayerState* state) {
    s32 element;
    s32 index;
    s32 x;
    s32 t;
    s32 yoff;
    u8* name;

    gScreenOffsetX = 105;
    gScreenOffsetY = 50;
    element = (state->spellId >> 8) & 0xF;
    index = state->spellId & 0xFF;
    name = gSpellNames[element][index];
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
    yoff = 0;
    if (gSpellBannerFade && (gSpellCrossFlags & 2)) {
        t = gSpellBannerFade * gSpellBannerFade;
        yoff = t * 40 / 64;
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255 - t * 255 / 64);
        gSpellBannerFade--;
    }
    DrawTexturePart(&gInfoWindowImage, 0, yoff, 110, 14, 100, 14, 0x400, 0x400);
    x = 55 - GetHUDTextWidth(name) / 2;
    DrawHUDText(x, yoff + 3, 0, name);
    gDPPipeSync(gDisplayListHead++);
    gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetCombineMode(gDisplayListHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
}

extern f32 gSpiritGainFlashPos[];             /* per element: an x, y pair, read flat */
extern s32 gSpiritMaxBits[];            /* per element: a flag for the player flags (SpiritGainWindow) */
extern u8 gNumberPalette[];
extern BrianData1 gBrianStats;
extern u16 gButtonsPressed;
extern u16 gSpiritGainGlow;
void SpiritGainWindow(mkst* dyn, PlayerState* player) {
    s32 n;
    s32 slide;
    u16 alpha;
    s32 sq;
    f32 posX;
    u8* colour;
    f32 posY;

    gScreenOffsetX = 129;
    gScreenOffsetY = 72;
    slide = 0;
    if (gSpiritGainGlow != 0) {
        if (gSpiritGainFlags & 1) {
            alpha = gSpiritGainGlow * 255 / 8;
        } else {
            n = gSpiritGainGlow;
            sq = n * n;
            slide = sq * 20 / 64;
            alpha = 255 - sq * 255 / 64;
        }
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, alpha);
    }
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
    DrawTexturePart(&gInfoWindowImage, 0, slide, 64, 20, 0, 0, 0x400, 0x400);
    DrawTexturePart(&gInfoWindowImage, 0, slide + 20, 64, 20, 0, 20, 0x400, 0x400);
    DrawTexturePart(&gInfoWindowImage, 0, slide + 40, 64, 20, 0, 40, 0x400, 0x400);
    DrawTexturePart(&gInfoWindowImage, 0, slide + 60, 64, 20, 0, 60, 0x400, 0x400);
    LoadHUDTexture(&gHudImage, 32, 29, 128, 10);
    gDPLoadTLUT_pal256(gDisplayListHead++, gNumberPalette);
    DrawNumber(5, 28, slide + 36, gBrianStats.spirits[0]);
    DrawNumber(5, 10, slide + 52, gBrianStats.spirits[1]);
    DrawNumber(5, 28, slide + 67, gBrianStats.spirits[2]);
    DrawNumber(5, 46, slide + 52, gBrianStats.spirits[3]);
    if (gSpiritGainGlow != 0 && (gSpiritGainFlags & 1)) {
        colour = &gElementColours[gSpiritGainElement * 4];
        posX = gSpiritGainFlashPos[gSpiritGainElement * 2] + gScreenOffsetX;
        posY = gSpiritGainFlashPos[gSpiritGainElement * 2 + 1] + gScreenOffsetY;
        guTranslate(&dyn->unkC0[gMatrixHead], posX, posY, 0.0f);
        gSPMatrix(gDisplayListHead++, &D_2000000[gMatrixHead] + 3, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gMatrixHead++;
        gDPPipeSync(gDisplayListHead++);
        gDPSetEnvColor(gDisplayListHead++, colour[0], colour[1], colour[2], gSpiritGainGlow * 255 / 8);
        gSPDisplayList(gDisplayListHead++, gSpiritFlashGfx);
    }
    if (gSpiritGainGlow == 0) {
        if (gButtonsPressed & (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS)) {
            if (gButtonsPressed & U_CBUTTONS) {
                gSpiritGainElement = 0;
            } else if (gButtonsPressed & L_CBUTTONS) {
                gSpiritGainElement = 1;
            } else if (gButtonsPressed & D_CBUTTONS) {
                gSpiritGainElement = 2;
            } else {
                gSpiritGainElement = 3;
            }
            if (player->stats->spirits[gSpiritGainElement] < 50) {
                player->stats->spirits[gSpiritGainElement]++;
                if (player->stats->spirits[gSpiritGainElement] == 50) {
                    player->flags |= gSpiritMaxBits[gSpiritGainElement];
                    if ((player->flags & 0xF000) == 0xF000) {
                        player->flags |= 0x10000;
                    }
                }
                PlaySound(0, 4, 0xFF);
                gSpiritFlashTimer = 30;
                gSpiritGainGlow = 8;
                gSpiritGainFlags |= 1;
            }
        }
    } else {
        gSpiritGainGlow -= 1;
        if ((gSpiritGainFlags & 1) && gSpiritGainGlow == 0) {
            gGameState &= ~8;
            gSpiritGainFlags ^= 1;
        }
    }
}

extern u16 gTalkMarkerBobPhase;                /* the talk marker's bob phase */
extern s32 gMarkerScreenY;                /* the marker's screen y (0xF0 = off screen) */
extern Npc gNpcs[];         /* the map objects */
void UpdateTalkMarker(void) {
    s32 screenX;
    u16* phase;
    s32 bob;
    ActorTransform* obj;

    phase = &gTalkMarkerBobPhase;
    obj = &gNpcs[gTalkTarget - 1].transform;
    if (WorldToScreen(obj->pos.x, obj->pos.y + 14.0, obj->pos.z, &screenX, &gMarkerScreenY) != 0) {
        bob = sins(*phase) >> 13;
        if (bob < 0) {
            bob = -bob;
        }
        *phase += 0x600;
        gSPDisplayList(gDisplayListHead++, gTalkMarkerIcon);
        DrawClippedTexRect(screenX, gMarkerScreenY - bob - 30, 64, 29, 0, 0, 0x400, 0x400);
    } else {
        gMarkerScreenY = 0xF0;
    }
}
extern u16 gSpiritMarkerBobPhase;                /* the icon's bob phase */
extern Gfx gSpiritMarkerIcon[];              /* the icon (24 x 30) */
void DrawSpiritMarker(void) {
    s32 x;
    s32 y;
    u16* phase;
    s32 bob;

    phase = &gSpiritMarkerBobPhase;
    if (WorldToScreen(gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.y + 11.0, gPlayerState.brian.transform.pos.z, &x, &y) != 0) {
        bob = sins(*phase) >> 13;
        if (bob < 0) {
            bob = -bob;
        }
        *phase += 0x600;
        gSPDisplayList(gDisplayListHead++, gSpiritMarkerIcon);
        DrawClippedTexRect(x, y - bob - 30, 24, 30, 0, 0, 0x400, 0x400);
    }
}
extern u16 gMarkerBobPhase;                /* the object marker icon's bob phase */
extern Gfx gObjectMarkerIcon[];              /* the icon (16 x 30) */
void DrawObjectMarker(void) {
    s32 x;
    s32 y;
    u16* phase;
    s32 bob;

    phase = &gMarkerBobPhase;
    if (WorldToScreen(gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.y + 11.0, gPlayerState.brian.transform.pos.z, &x, &y) != 0) {
        bob = sins(*phase) >> 13;
        if (bob < 0) {
            bob = -bob;
        }
        *phase += 0x600;
        gSPDisplayList(gDisplayListHead++, gObjectMarkerIcon);
        DrawClippedTexRect(x, y - bob - 30, 16, 30, 0, 0, 0x400, 0x400);
    }
}
extern u16 gSignMarkerBobPhase;                /* the sign marker icon's bob phase */
void DrawSignMarker(void) {
    s32 x;
    s32 y;
    u16* phase;
    s32 bob;

    phase = &gSignMarkerBobPhase;
    if (WorldToScreen(gPlayerState.brian.transform.pos.x, gPlayerState.brian.transform.pos.y + 11.0, gPlayerState.brian.transform.pos.z, &x, &y) != 0) {
        bob = sins(*phase) >> 13;
        if (bob < 0) {
            bob = -bob;
        }
        *phase += 0x800;
        gSPDisplayList(gDisplayListHead++, gTalkMarkerIcon);
        DrawClippedTexRect(x, y - bob - 30, 64, 29, 0, 0, 0x400, 0x400);
    }
}
extern u16 gEnemyMarkerBobPhase;                /* the enemy marker's bob phase */
extern Gfx gEnemyMarkerIcon[];              /* the enemy marker icon (24 x 42) */
extern f64 D_8006F558;                /* 350.0 */
void DrawEnemyMarker(u16 enemy) {
    s32 screenX;
    s32 screenY;
    u16* phase;
    MonsterBattleData* m;
    f32 y;
    s32 bob;

    phase = &gEnemyMarkerBobPhase;
    m = &gEnemies[enemy].battleData;
    if (m->transform.base.monster->monsterType == FLYING) {
        y = m->stats->flyY;
    } else if (m->transform.base.monster->monsterType == MAMMON) {
        y = m->transform.pos.y + D_8006F558 * m->transform.scale;
    } else {
        y = m->transform.pos.y + m->transform.base.monster->height * m->transform.scale;
    }
    if (WorldToScreen(m->transform.pos.x, y, m->transform.pos.z, &screenX, &screenY) != 0) {
        bob = sins(*phase) >> 13;
        if (bob < 0) {
            bob = -bob;
        }
        *phase += 0x800;
        gSPDisplayList(gDisplayListHead++, gEnemyMarkerIcon);
        DrawClippedTexRect(screenX - 4, screenY - bob - 42, 24, 42, 0, 0, 0x400, 0x400);
    }
}
void DrawEscapeLabel(void) {
    s32 sx;
    s32 sy;
    f32 y;
    f32 height = gPlayerState.brian.transform.scale * gPlayerState.brian.transform.base.player->height;

    y = gPlayerState.brian.transform.pos.y + height;
    if (WorldToScreen(gPlayerState.brian.transform.pos.x, y, gPlayerState.brian.transform.pos.z, &sx, &sy) != 0) {
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, gStatusSymbolsPalette);

        DrawTexturePart(&gStatusSymbolsImage, sx - 0x17, sy - 0xF, 0x2E, 0xF, 0x51, 0x31, 0x400, 0x400);
    }
}
void UpdateStatusVisuals(MonsterBattleData* actor, u16 statusBits, u16 kind, s16 duration) {
    u32 i = 0;
    ActorStats* data = actor->stats;

    for (i = 0; i < 7; i++) {
        if (gStatusIconBits[i] & statusBits) {
            data->status[i] = kind;
            data->statusTimer[i] = duration;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        f32 effectY;
        EffectSpawnDef* model;
#ifdef MATCHING_HACK_IDO
        u16 pad; /* unused: the -g3 stack slot that puts the effect slot address spill where the original has it */
#endif

        if ((gStatusFxBits[i] & statusBits) && (data->statusFx[i] == 0)) {
            data->statusFxTimer[i] = duration;
            gHudVector.x = gStatusFxDefs[kind].sizeX;
            gHudVector.y = gStatusFxDefs[kind].sizeY;
            gHudVector.scale = 1.0f;
            model = &gStatusFxModels[gStatusFxDefs[kind].model];
            if (actor->transform.base.monster->monsterType == FLYING) {
                effectY = (gStatusFxDefs[kind].flags & 0x100) ? actor->stats->flyY : actor->stats->flyY - (actor->transform.base.monster->radius * actor->transform.scale);
            } else {
                effectY = (gStatusFxDefs[kind].flags & 0x100) ? (f32) (actor->transform.pos.y + (actor->transform.base.monster->height * 0.5 * actor->transform.scale)) : actor->transform.pos.y;
            }
            data->statusFx[i] = (StatusFxObject*) SpawnEffectObject(gStatusFxDefs[kind].color, gStatusFxDefs[kind].flags, actor->transform.pos.x, effectY,
                                                                actor->transform.pos.z, 0.0f, model, &gHudVector, &actor->transform);
            return;
        }
    }
}
typedef struct {
    s16 s;
    s16 t;
    u16 w;
    u16 h;
} IconRect;                          /* a rectangle in the icon sheet */
extern IconRect gStatusIcons[];        /* the status icons */
void DrawMonsterStatusIcons(MonsterBattleData* actor) {
    ActorStats* data;
    f32 y;
    s32 onScreen;
    s32 sx;
    s32 sy;
    s32 dx;
    s32 dy;
    s32 i;
    IconRect* icon;
    s32 s;
    s32 t;
    s32 w;
    s32 h;
    StatusFxObject* fx;

    data = actor->stats;
    if (!(gGameState & 0x800)) {
        if (actor->transform.base.monster->monsterType == FLYING) {
            y = actor->stats->flyY;
        } else {
            y = actor->transform.pos.y + actor->transform.base.monster->height * actor->transform.scale;
        }
        onScreen = WorldToScreen(actor->transform.pos.x, y, actor->transform.pos.z, &sx, &sy);
        gDPPipeSync(gDisplayListHead++);
        gDPLoadTLUT_pal256(gDisplayListHead++, gStatusSymbolsPalette);
        dy = 0;
        dx = 0;
        for (i = 0; i < 7; i++) {
            if (data->status[i] != 0) {
                icon = &gStatusIcons[data->status[i]];
                s = icon->s;
                t = icon->t;
                w = icon->w;
                h = icon->h;
                if (onScreen) {
                    DrawTexturePart(&gStatusSymbolsImage, sx + dx - (w >> 1), sy + dy - (h >> 1), w, h, s, t, 0x400, 0x400);
                }
                if (!(gStatusIconBits[i] & data->statusFlags)) {
                    data->status[i] = 0;
                } else if (data->statusTimer[i] > 0) {
                    data->statusTimer[i]--;
                    if (data->statusTimer[i] == 0) {
                        data->status[i] = 0;
                    }
                }
                dx += w >> 1;
                dy -= h >> 1;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (data->statusFx[i] != NULL) {
            fx = actor->stats->statusFx[i];
            fx->x = actor->transform.pos.x;
            fx->z = actor->transform.pos.z;
            if (actor->transform.base.monster->monsterType == FLYING) {
                if (fx->flags & 0x100) {
                    fx->y = actor->stats->flyY;
                } else {
                    fx->y = actor->stats->flyY - actor->transform.base.monster->radius * actor->transform.scale;
                }
            } else if (fx->flags & 0x100) {
                fx->y = actor->transform.pos.y + actor->transform.base.monster->height * 0.5 * actor->transform.scale;
            } else {
                fx->y = actor->transform.pos.y;
            }
            if (!(gStatusFxBits[i] & data->statusFlags)) {
                data->statusFx[i] = NULL;
                if (fx->flags & 1) {
                    fx->flags &= ~1;
                    fx->timer = 8;
                }
            } else if (data->statusFxTimer[i] > 0) {
                data->statusFxTimer[i]--;
                if (data->statusFxTimer[i] == 0) {
                    data->statusFx[i] = NULL;
                    if (fx->flags & 1) {
                        fx->flags &= ~1;
                        fx->timer = 8;
                    }
                }
            }
        }
    }
}
void ClearStatusEffects(MonsterBattleData* actor) {
    u32 i;
    ActorStats* data;
    StatusFxObject* effect;

    data = actor->stats;

    for (i = 0; i < 7; i++) {
        if (data->status[i] != 0) {
            data->status[i] = 0;
        }
    }

    for (i = 0; i < 4; i++) {
        if (data->statusFx[i] != NULL) {
            effect = data->statusFx[i];
            effect->timer = 8;
            effect->flags &= ~1;
            data->statusFx[i] = NULL;
        }
    }
}
void func_80023DEC(void) {
}
void func_80023DF4(void) {
}
void func_80023DFC(void) {
}
void func_80023E04(void) {
}
void func_80023E0C(void) {
}

extern EnemyAction* gCardEnemy;      /* the enemy on the info card: the last one hit (EnemyHitReaction) */
extern u8** gEnemyNames;              /* enemy names */
void DrawMonsterCard(void) {
    s32 slide;

    gScreenOffsetX = 181;
    gScreenOffsetY = 26;
    slide = gMonsterCardTimer - 50;
    if (slide > 0) {
        slide *= slide;
        gScreenOffsetX += slide * 120 / 100;
    }
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gInfoWindowPalette);
    if (gCardEnemy->battleData.statsBuf.statusFlags & 0x80) {
        DrawTexturePart(&gInfoWindowImage, 0, 0, 110, 16, 100, 28, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, 0, 16, 110, 16, 100, 44, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, 0, 32, 110, 16, 100, 60, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, 0, 48, 110, 4, 100, 76, 0x400, 0x400);
    } else {
        DrawTexturePart(&gInfoWindowImage, 0, 0, 110, 16, 100, 28, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, 0, 16, 110, 10, 100, 44, 0x400, 0x400);
        DrawTexturePart(&gInfoWindowImage, 0, 26, 110, 2, 100, 78, 0x400, 0x400);
    }
    DrawHUDText(4, 17, 0, gEnemyNames[gCardEnemy->monBaseData->monsterID]);
    LoadHUDTexture(&gHudImage, 32, 29, 128, 10);
    gDPLoadTLUT_pal256(gDisplayListHead++, gNumberPalette);
    if (gCardEnemy->battleData.statsBuf.statusFlags & 0x80) {
        DrawNumber(3, 16, 28, gCardEnemy->monBaseData->maxHP);
        DrawNumber(3, 16, 38, gCardEnemy->monBaseData->def);
        DrawNumber(3, 70, 28, gCardEnemy->monBaseData->atk);
        DrawNumber(3, 70, 38, gCardEnemy->monBaseData->agi);
        if (gCardEnemy->monBaseData->element != 4) {
            DrawTexturePart(&gHudImage, 91, 3, 12, 12, gCardEnemy->monBaseData->element * 12 + 32, 16, 0x400, 0x400);
        }
    }
}
void Init_MISS_Icon(void) {
    MissIcon* missIconData;
    s32 i;

    missIconData = gMISSData;
    i = 8;
    while (i > 0){
        
        missIconData->displayTimer = 0;
        missIconData++;
        i--;
    }
}
void Setup_MISS_Icon(MonsterBattleData* actor) {
    MonsterBaseData* monsterData;
    s32 i;
    MissIcon* missIconData;

    missIconData = gMISSData;
    for (i = 7; i != 0 && missIconData->displayTimer != 0; i--, missIconData++) {
        
    }
    missIconData->displayTimer = 0x2D; //length of time "MISS" appears.
    missIconData->pos = &actor->transform.pos; //pointer to the position "MISS" should appear.
    missIconData->offsetZ = 0.0f; //"MISS" location offset (yPos?)
    missIconData->offsetX = 0.0f; //"MISS" location offset (xPos?)
    monsterData = actor->transform.base.monster;
    if (monsterData->monsterType == ON_GROUND) {
        missIconData->offsetY = monsterData->height * actor->transform.scale;
    } else if (monsterData->monsterType == FLYING) {
        missIconData->offsetY = actor->stats->flyY - actor->transform.pos.y;
    } else {
        missIconData->offsetY = actor->transform.scale * D_8006F560;
    }
    missIconData->texS = 0x23; //xpos on texture for MISS icons
    missIconData->texT = 0x31; //ypos on texture for MISS icons
    missIconData->width = 0x29;
    missIconData->height = 0xE;
}
void Draw_MISS_Icon(void) {
    HudImage* image;
    f32 x;
    f32 y;
    f32 z;
    s32 sx;
    s32 sy;
    MissIcon* icon;
    s32 n;

    gDPPipeSync(gDisplayListHead++);
    gDPLoadTLUT_pal256(gDisplayListHead++, gStatusSymbolsPalette);
    image = &gStatusSymbolsImage;
    icon = gMISSData;
    n = 8;
    while (n != 0) {
        if (icon->displayTimer != 0) {
            x = icon->offsetX;
            y = icon->offsetY;
            z = icon->offsetZ;
            if (icon->pos != NULL) {
                x += icon->pos->x;
                y += icon->pos->y;
                z += icon->pos->z;
            }
            WorldToScreen(x, y, z, &sx, &sy);
            DrawTexturePart(image, sx - (icon->width >> 1), sy - icon->height, icon->width, icon->height,
                            icon->texS, icon->texT, 0x400, 0x400);
            icon->displayTimer--;
        }
        n--;
        icon++;
    }
}
extern u8 gUnlearnedSpellName[];             /* the placeholder for a spell not learned yet */
extern u8 gSpellLabelColours[];             /* label colour by availability */
s32 GetSpellFromPath(u16 presses, u8* path);
s32 GetSpellAvailability(u32 spell, u16 mp);
void DrawSpellLabel(s32 x, s32 y, u8* path, s32 presses, u16 context) {
    u32 spell;
    s32 flags;
    u8 avail;
    u8* name;
    s32 element;

    spell = GetSpellFromPath(presses, path);
    flags = GetSpellAvailability(spell, context);
    avail = flags;
    element = (spell >> 8) & 0xF;
    spell &= 0xFF;                  /* RULE-EXCEPTION R30: spell becomes the index; a separate index local loses the match */
    name = gSpellNames[element][spell];
    if (avail & 4) {
        name = gUnlearnedSpellName;
    }
    x = x - GetHUDTextWidth(name) / 2 + 0x37;
    DrawHUDText(x, y, gSpellLabelColours[avail], name);
}
extern HudImage gHudFontImage;
extern u16* gHudFontPalettes[];
void DrawHUDText(s32 x, s32 y, s32 colour, u8* text) {
    s32 row;
    s32 s;
    s32 t;                              /* the 0x80 and 0x81 glyphs reuse it from the last letter, unset before the first */

    row = 8;
    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gHudFontPalettes[colour]);
    gDPTileSync(gDisplayListHead++);
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0x0100, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
    gDPLoadSync(gDisplayListHead++);
    gDPLoadTLUTCmd(gDisplayListHead++, G_TX_LOADTILE, 15);
    gDPPipeSync(gDisplayListHead++);
    while (*text != 0xFF) {
        if (*text == 0x80) {
            DrawClippedTexRect(x - 2, y - 1, 8, 9, 0x78, t, 0x400, 0x400);
            x++;
        } else if (*text == 0x81) {
            DrawClippedTexRect(x - 2, y - 1, 8, 9, 0x80, t, 0x400, 0x400);
            x++;
        } else if (*text == 0x82) {
            x += 4;
        } else {
            s = *text & 0xF;
            t = *text & 0x70;
            s *= 8;
            t >>= 4;
            if (t != row) {
                LoadHUDTexture(&gHudFontImage, 0, t * 9, 0xA0, 9);
                row = t;
            }
            t *= 9;
            DrawClippedTexRect(x, y, 8, 9, s, t, 0x400, 0x400);
            x += 8;
        }
        text++;
    }
}

s32 GetHUDTextWidth(u8* text) {
    s32 width = 0;

    while (*text != 0xFF) {
        if (!(*text & 0x80)) {
            width += 8;
        } else if (*text == 0x80 || *text == 0x81) {
            width++;
        } else if (*text == 0x82) {
            width += 4;
        }
        text++;
    }
    return width;
}
void DrawNumber(u16 flags, s32 x, s32 y, s32 value)
{
  u8 *p;
  u8 digits[0xC];
  s32 n;

    
  p = digits;
  n = count_digits(value, digits, flags);
    for (;n != 0; n--, p++)
    {
      if (*p != ' ')
      {
        DrawClippedTexRect(x, y, 8, 0xA, (*p * 8) + 32, 0x1D, 0x400, 0x400);
        x += 7;
      } else
      if (flags & 6)
      {
        x += 7;

      }
    }
}
void func_80024A4C(void) {
}

void LoadHUDTexture(HudImage* tex, s32 s, s32 t, s32 width, s32 height) {
    if (tex->ci4 == 0) {
        /* lrs / lrt one texel past the rectangle (the CI4 path below subtracts 1): a shipped off-by-one, harmless */
        gDPLoadTextureTile(gDisplayListHead++, tex->image, G_IM_FMT_CI, G_IM_SIZ_8b, tex->width, 0, s, t, s + width,
                           t + height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                           G_TX_NOLOD, G_TX_NOLOD);
    } else {
        gDPLoadTextureTile_4b(gDisplayListHead++, tex->image, G_IM_FMT_CI, tex->width, 0, s, t, s + width - 1,
                              t + height - 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                              G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
}
void DrawClippedTexRect(s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy) {
    ulx += gScreenOffsetX;
    lrx += ulx;
    uly += gScreenOffsetY;
    if (lrx < gClipLeft) {
        return;
    }
    lry += uly;
    if (lry < gClipTop || gClipRight < ulx || gClipBottom < uly) {
        return;
    }
    if (gClipRight < lrx) {
        lrx = gClipRight;
    }
    if (gClipBottom < lry) {
        lry = gClipBottom;
    }
    if (ulx < gClipLeft) {
        s = s - ulx + gClipLeft;
        ulx = gClipLeft;
    }
    if (uly < gClipTop) {
        t = t - uly + gClipTop;
        uly = gClipTop;
    }
    gSPTextureRectangle(gDisplayListHead++, ulx << 2, uly << 2, lrx << 2, lry << 2, G_TX_RENDERTILE, s << 5, t << 5, dsdx,
                        dtdy);
}
void DrawTexturePart(HudImage* image, s32 ulx, s32 uly, s32 lrx, s32 lry, s32 s, s32 t, s32 dsdx, s32 dtdy) {
    LoadHUDTexture(image, s, t, lrx, lry);
    DrawClippedTexRect(ulx, uly, lrx, lry, s, t, dsdx, dtdy);
}
void Inventory_Init(void) {
    s32 i;
    u8* slot;


    dma_write(item_icon_palettes_ROM_START, gInventoryPalette, 0x400); //load inventory palette into RAM
    i = 8;
    slot = gVisibleInvItemIDs;
    do {
        i--;
        *slot++ = 0xFF;
    } while (i != 0);
    
    slot = gInventory;
    
    i = 151;
     while (i != 0){
        i--;
        *slot++ = 0xFF;
    }
    
    gItemScroll = 0;
    gItemCursor = 0;
    D_8008C1A4 = 0;
}
s32 CheckIfInventoryFull(void) {
    s32 full;

    if (gInventory[149] != 0xFF) {
        full = 0;
    } else {
        full = 1;
    }
    return full;
}
u32 CountItem(u8 itemID) {
    s32 i;
    s32 count;
    u8* item;

    count = 0;
    i = 150;
    for (item = gInventory; i != 0 && *item != 0xFF; i--, item++) {
        if (*item == itemID) {
            count++;
        }
    }
    return count;
}
u32 CountInventory(void) {
    s32 i;
    s32 count;
    u8* item;

    count = 0;
    i = 150;
    for (item = gInventory; i != 0 && *item != 0xFF; i--, item++) count++;
    return count;
}
void AddItemToInventory(u8 itemID) {
    s32 i = 0;

    do {
        if (gInventory[i] == 0xFF) {
            break;
        }
        i++;
    } while (i < 150);

    gInventory[i] = itemID;
}
s32 CheckIfItemUsable(u8 itemIndex) {
    s32 can_use_item;
    ItemData* item_data;
    s32 in_combat;

    can_use_item = 0;
    in_combat = gBattleState & 1;

    if (in_combat && (gItemData[itemIndex].canBeUsed & 2)) {
        can_use_item = 1;
    } else if (!in_combat && (gItemData[itemIndex].canBeUsed & 1)) {
        can_use_item = 1;
    }

    item_data = &gItemData[itemIndex];

    if (can_use_item) {
        can_use_item = gItemCheckHandlers[item_data->type](&gPlayerState.brian.transform, item_data);
    }

    if (can_use_item) {
        StartBrianUseItem(itemIndex);
    }

    return can_use_item;
}
void UseItem(u8 itemID, MonsterBattleData* brian) {
    gItemUseHandlers[gItemData[itemID].type](brian, &gItemData[itemID], itemID, brian);
}
s32 RemoveKeyItem(u16 id) {
    s32 found;
    u8* item;
    s32 i;
    ItemData* data;

    found = 0;
    item = gInventory;
    i = 150;
    while (!found && *item != 0xFF) {
        data = &gItemData[*item++];
        if (data->type == 0xF) {
            found = id == data->itemArg1;
        }
        i--;
    }
    if (found) {
        item--;
        while (i != 0) {
            *item = item[1];
            i--;
            item++;
        }
        *item = 0xFF;
    }
    return found;
}
void DrawItemBox(s32 ulx, s32 uly, s32 width, s32 height);
void StartScreenFade(s32 frames, u16 flags, void* ramp);
void DrawString(s32 x, s32 y, u8* text);
extern s8 gStickX;
extern u16 D_8004F4C0;
extern u8 D_8004F5B4[];
extern u8* gItemNamePtrs[];
extern u8* gItemDescriptionPtrs[];
extern Gfx D_803AA7C0[];
extern Gfx D_803AA940[];
extern Gfx D_803AA998[];
extern Gfx D_803AA9F0[];
extern Gfx D_803AA828[];
extern Gfx D_1000388[];
extern u8 gHudTexture[];
extern u8 D_8008C1B8[0x800];      /* the eight cached item icons, 16 x 16 CI8 (0x100 bytes each) */
void ItemMenu(void) {
    s32 i;                  /* a count down: the rows shown, the cache slots, the icons drawn, the items moved up */
    s32 k;                  /* a search's count down; then the icon's cache slot (RULE-EXCEPTION R30: one stack word for both; a separate slot local moves the frame and takes another register) */
    s32 shown;              /* the rows shown, at most 8 */
    u8 id;                  /* the row's item */
    s32 moved;              /* set when the stick moved the cursor or the list */
    s32 x;                  /* the stick's x past the dead zone; then the icons' x; then the cursor's x (RULE-EXCEPTION R30: one stack word for all three; a separate local moves the frame) */
    s32 lit;                /* the lit palette is loaded */
    u16* phase;             /* the cursor's and the arrows' bob phase */
    u8* item;               /* walks the inventory */
    u8* cache;              /* walks the cached item ids */
    u8* freeTop;            /* the top of the free slots */
    u8 free[8];             /* the cache slots no shown item uses */
    s32 bob;                /* the cursor's bobbing y; then the arrows' bob offset (RULE-EXCEPTION R30: one stack word for both; a separate local moves the frame) */

    phase = &D_8004F4C0;
    gSPDisplayList(gDisplayListHead++, D_803AA7C0);
    DrawItemBox(70, 127, 178, 22);
    DrawItemBox(72, 68, 173, 54);
    gSPDisplayList(gDisplayListHead++, D_803AA940);
    gSPDisplayList(gDisplayListHead++, D_803AA998);
    x = gStickX;
    if (x > 5) {
        x -= 5;
    } else if (x < -5) {
        x += 5;
    } else {
        x = 0;
    }
    item = &gInventory[gItemScroll];
    for (i = 8; i != 0 && *item != 0xFF; i--) {
        item++;
    }
    shown = 8 - i;
    if (shown != 0) {
        moved = 0;
        if (x != 0) {
            if (D_8008C1A4 != 0 && (x ^ D_8008C1A4) < 0) {
                D_8008C1A4 = 0;
            } else {
                D_8008C1A4 += x;
                if (D_8008C1A4 > 250) {
                    D_8008C1A4 = 0;
                    moved++;
                    gItemCursor++;
                    if (gItemCursor >= shown) {
                        gItemCursor = shown - 1;
                        if (gItemScroll < 150 - shown) {
                            gItemScroll++;
                            if (gInventory[gItemScroll + shown - 1] == 0xFF) {
                                gItemScroll--;
                                moved--;
                            }
                        }
                    }
                } else if (D_8008C1A4 < -250) {
                    D_8008C1A4 = 0;
                    moved++;
                    gItemCursor--;
                    if (gItemCursor < 0) {
                        gItemCursor = 0;
                        if (gItemScroll > 0) {
                            gItemScroll--;
                        } else {
                            moved--;
                        }
                    }
                }
                if (moved) {
                    PlaySound(0, 3, 0xFF);
                }
            }
        } else {
            D_8008C1A4 = 0;
        }
        item = &gInventory[gItemScroll];
        freeTop = free;
        cache = gVisibleInvItemIDs;
        for (i = 8; i != 0; i--) {
            for (k = shown; k != 0 && *cache != *item; k--) {
                item++;
            }
            if (k == 0) {
                *freeTop++ = 8 - i;
            }
            cache++;
            item = &gInventory[gItemScroll];
        }
        x = 107 << 2;
        i = shown;
        gSPDisplayList(gDisplayListHead++, D_803AA9F0);
        lit = FALSE;
        while (i != 0 && (id = *item) != 0xFF) {
            k = 8;
            for (cache = gVisibleInvItemIDs; k != 0 && id != *cache; k--) {
                cache++;
            }
            if (k == 0) {
                k = *--freeTop;
                gVisibleInvItemIDs[k] = id;
                dma_write(item_icon_palettes_ROM_START + (*item << 8) + 0x400, &D_8008C1B8[k << 8], 0x100);
            } else {
                k = 8 - k;
            }
            gDPLoadTextureBlock(gDisplayListHead++, &D_8008C1B8[k << 8], G_IM_FMT_CI, G_IM_SIZ_8b, 16, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            if (gItemCursor == (shown - i)) {
                gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
                gDPLoadTLUT_pal256(gDisplayListHead++, gInventoryPalette);
                lit = TRUE;
            } else if (lit) {
                gDPSetEnvColor(gDisplayListHead++, 127, 127, 127, 255);
                gDPLoadTLUT_pal256(gDisplayListHead++, &gInventoryPalette[256]);
                lit = FALSE;
            }
            gSPTextureRectangle(gDisplayListHead++, x, 131 << 2, x + (16 << 2), 147 << 2, G_TX_RENDERTILE, 0, 0,
                                1 << 10, 1 << 10);
            x += 17 << 2;
            item++;
            i--;
        }
        x = gItemCursor * (17 << 2) + (117 << 2);
        bob = (124 << 2) - ((sins(*phase & 0x7FFF) >> 11) & ~3);
        gSPDisplayList(gDisplayListHead++, D_803AA828);
        gSPTextureRectangle(gDisplayListHead++, x, bob, x + (12 << 2), bob + (12 << 2), G_TX_RENDERTILE, 78 << 5, 0,
                            1 << 10, 1 << 10);
        if (shown == 8) {
            bob = sins((*phase * 2) & 0x7FFF) >> 13;
            gDPLoadTextureTile(gDisplayListHead++, gHudTexture, G_IM_FMT_CI, G_IM_SIZ_8b, 176, 0, 90, 0, 146, 13, 0,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);
            if (gItemScroll != 0) {
                gSPTextureRectangle(gDisplayListHead++, 134 << 2, 148 << 2, 141 << 2, 155 << 2, G_TX_RENDERTILE,
                                    112 << 5, 0, 1 << 10, 1 << 10);
                gSPTextureRectangle(gDisplayListHead++, (119 - bob) << 2, 146 << 2,
                                    (133 - bob) << 2, 157 << 2, G_TX_RENDERTILE, 119 << 5, 0, 1 << 10, 1 << 10);
            }
            if (gInventory[gItemScroll + 8] != 0xFF) {
                gSPTextureRectangle(gDisplayListHead++, 210 << 2, 148 << 2, 217 << 2, 155 << 2, G_TX_RENDERTILE,
                                    112 << 5, 0, 1 << 10, 1 << 10);
                gSPTextureRectangle(gDisplayListHead++, (bob + 218) << 2, 146 << 2,
                                    (bob + 232) << 2, 157 << 2, G_TX_RENDERTILE, 133 << 5, 0, 1 << 10, 1 << 10);
            }
        }
        gSPDisplayList(gDisplayListHead++, D_1000388);
        DrawString(119, 72, gItemNamePtrs[gInventory[gItemScroll + gItemCursor]]);
        DrawString(78, 90, gItemDescriptionPtrs[gInventory[gItemScroll + gItemCursor]]);
        if ((gButtonsPressed & 0xA000) && CheckIfItemUsable(gInventory[gItemScroll + gItemCursor])) {
            item = &gInventory[gItemScroll + gItemCursor];
            i = 149 - gItemScroll - gItemCursor;
            while (i != 0) {
                *item = item[1];
                i--;
                item++;
            }
            *item = 0xFF;
            gGameState &= ~1;
            StartScreenFade(6, 4, D_8004F5B4);
            if (gInventory[gItemScroll + gItemCursor] == 0xFF) {
                if (gItemScroll > 0) {
                    gItemScroll--;
                } else if (gItemCursor > 0) {
                    gItemCursor--;
                }
            } else if (gInventory[gItemScroll + 7] == 0xFF && gItemScroll > 0) {
                gItemScroll--;
            }
        }
        *phase += 0x300;
    }
}

s32 NoEffectItem(ActorTransform* pos, ItemData* item) {
    return 0;
}
s32 CheckIfHealingNeeded(ActorTransform* brian, ItemData* item) {
    BrianData1* stats;
    s32 usable;

    usable = FALSE;
    stats = brian->base.player;
    /* BUG (as in Quest): only "not equal to the maximum" is tested, so a value above the maximum still heals */
    if ((item->itemArg1 != 0) && (stats->maxHP != stats->currHP)) {
        usable = TRUE;
    }
    if ((item->itemArg2 != 0) && (stats->maxMP != stats->currMP)) {
        usable = TRUE;
    }
    return usable;
}
s32 CheckIfTeleportPossible(ActorTransform* brian, ItemData* item) {
    s32 usable;

    if (((gMapFlags & 0xA) != 0) || ((gSubmapHeader->flags & 4) != 0)) {
        usable = FALSE;
    } else {
        usable = TRUE;
    }
    return usable;
}
u8 CheckIfSpellItemUsable(ActorTransform* brian, ItemData* item) {
    u8 usable;

    usable = TRUE;
    if ((gBattleState & 0x100) && (item->canBeUsed & 8)) {  /* 8: not in a boss battle */
        usable = FALSE;
    }
    return usable;
}
void UseNoEffectItem(MonsterBattleData* user, ItemData* item) {

}
void UseHealingItem(MonsterBattleData* BrianData, ItemData* item)
{
    u16 delta;
    BrianData1* status = BrianData->transform.base.player;
    EffectSpawnDef* temp;
    
    if (item->itemArg1 != 0) {
        delta = (status->maxHP - status->currHP);
        if (item->itemArg1 < delta) {
            delta = item->itemArg1;
        }
        
        status->currHP += delta;
        SpawnDamageNumber(BrianData, 1, delta);
    }
    if (item->itemArg2 != 0) {
        delta = (status->maxMP - status->currMP);
        if (item->itemArg2 < delta) {
            delta = item->itemArg2;
        }
        status->currMP += delta;
        SpawnDamageNumber(BrianData, 2, delta);
    }
    gItemLightRingSize.x = gBaseItemLightRingData.healItemLightWidth;
    gItemLightRingSize.y = gBaseItemLightRingData.healItemLightHeight;
    gItemLightRingSize.scale = 1.0f;
    temp = &gItemEffectModels[gBaseItemLightRingData.unk0];
    SpawnEffectObject(gBaseItemLightRingData.unk2, gBaseItemLightRingData.healItemLightColor, BrianData->transform.pos.x, BrianData->transform.pos.y, BrianData->transform.pos.z, 0.0, temp, &gItemLightRingSize, &BrianData->transform);
    PlaySound(0, 0xB, 0xFF);
}
void UseWings(ActorTransform* brian, ItemData* item) {
    EffectSpawnDef* ring;

    gGameState |= 0x80;
    gModeCountdown = 0x16;
    gPendingWarp.map = (s32)gWingData[item->itemArg1].mapID;
    gPendingWarp.submap = (s32)gWingData[item->itemArg1].submapID;
    gPendingWarp.entrance = -1;
    gBrianSpawnX = gWingData[item->itemArg1].xpos; //xpos after teleporting
    gBrianSpawnZ = gWingData[item->itemArg1].zpos; //zpos after teleporting
    gBrianSpawnYaw = gWingData[item->itemArg1].yrot; //rotation after teleporting; all three of these also apply to going through doors.
    gArrivalFlags = 0x108;
    gPlayerState.animCue = 0;
    gTransitionStyle = 1;
    gItemLightRingSize.x = gBaseItemLightRingData.wingLightWidth;
    gItemLightRingSize.y = gBaseItemLightRingData.wingLightHeight;
    gItemLightRingSize.scale = 1.0f;
    ring = &gItemEffectModels[gBaseItemLightRingData.unk10];
    SpawnEffectObject(gBaseItemLightRingData.unk12, gBaseItemLightRingData.wingLightColor, brian->pos.x, brian->pos.y, brian->pos.z, 0.0f, ring, &gItemLightRingSize, brian);
    PlaySound(0, 0x3B, 0xFF);
}
void UseSpellItem(ActorTransform* brian, ItemData* item) {
    SpawnSpell(brian, item->itemArg1, 0, 4);
}
void DrawItemBox(s32 ulx, s32 uly, s32 width, s32 height) {

    gDPPipeSync(gDisplayListHead++);
    
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    
    gDPSetRenderMode(gDisplayListHead++, G_RM_NOOP, G_RM_NOOP2);
    
    gDPSetFillColor(gDisplayListHead++, 0x00010001);
    
    gDPFillRectangle(gDisplayListHead++,  ulx, uly, (ulx + width), uly);

    gDPFillRectangle(gDisplayListHead++, ulx, (uly + height), (ulx + width) , (uly + height));

    gDPFillRectangle(gDisplayListHead++, ulx , uly, ulx, (uly + height) );

    gDPFillRectangle(gDisplayListHead++, ulx + width, uly, (ulx + width), uly + height );

    gDPFillRectangle(gDisplayListHead++ , ((ulx + 3)), ((uly + height) + 1), ((ulx + width) + 2) , ((uly + height) + 2));

    gDPFillRectangle(gDisplayListHead++, ((ulx + width) + 1), (uly + 3) , ((ulx + width) + 2), ((uly + height) + 1));

    gSPDisplayList(gDisplayListHead++, gItemBoxGfx);

    // Needs to be shifted by 2. Multiplying by 4 technically has the same effect, but doesn't match
    gSPTextureRectangle(gDisplayListHead++, ((ulx + 1) <<2) ,((uly+1)<<2), ((ulx + width) * 4), ((uly + height) * 4) , G_TX_RENDERTILE, 0, 0, qs510(1), qs510(1));

}
