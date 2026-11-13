#ifndef GAME_SIGNS_H
#define GAME_SIGNS_H

/* The functions of src/game/signs.c, declared once for every file that calls them. */

#include "common.h"

void LoadRoomSigns(void);
SignSlot* TryReadSign(ActorTransform* brian);
void LoadSignText(SignRef* sign);

#endif
