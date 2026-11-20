#ifndef DIALOGUE_H
#define DIALOGUE_H

/* The dialogue bank's strings by name. StartNPCText reads an NPC's string (at most MESSAGE_TEXT_SIZE bytes) at
 * dialogue_bank_ROM_START plus the u16 offset in NpcRecord.message or .altMessage; LoadSignText reads a sign's name
 * (0x20 bytes) and text (0x200 bytes) at D_D37700, the sign part's start, plus the u16 offsets in the sign record.
 * Each name below is such an offset: DLG_NPC_<offset in the original> from the bank's start, DLG_SIGN_<offset in the
 * original> from the sign part's start. The build writes them (tools/text_build.py, from the bank's source text,
 * assets/dialogue_bank/<part>.dialogue.txt) into build/include/dialogue/, so a string that changes length moves the
 * strings after it and every record that names one follows. */
#include "dialogue/npc.h"
#include "dialogue/signs.h"

#endif
