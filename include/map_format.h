#ifndef MAP_FORMAT_H
#define MAP_FORMAT_H
/* The map format's types that are not in quest_types.h. A header or unit that names one of them includes this header. The record
   writer reads the types here as it reads structs.h and quest_types.h (tools/splat_ext/anim.py, TYPE_HEADERS). */

/* the camera setups of one submap: InitRoomCamera (camera.c) reads gMapHeader->cameras[gCurrentSubmap] */
typedef struct {
    CameraSetup* setups;
    u16 count;
} MapCameras;                           /* 8 bytes: a submap's camera table (sMapData +0x24, one per submap) */

/* A submap's random encounters (encounter.c; names from vbhayden's Quest 64 scratches). For a submap whose flags have
   SUBMAP_ENCOUNTERS, InitEncounters takes the submap's EncounterZoneList (quest_types.h) from sMapData.encounters and its
   EncounterSet from sMapData.encounterSets, one of each per submap. Walking inside a zone (FindEncounterZone) can start a
   fight: CreateEncounter picks one of the zone's packs at random, and IsInEncounterCircle the spot it is fought at. */
typedef struct {
    s16 x;
    s16 z;
} EncounterCircle;                      /* 4 bytes: a spot a fight is centred on; IsInEncounterCircle picks one of the
                                           first six that lie 50 to 90 units from Brian, at random */
typedef struct {
    s32 enemy_type;                     /* SpawnEnemy's index into the monster records (baseStats) */
    s32 base_count;
    s32 count_variance;                 /* CreateEncounter adds 0 to count_variance more, at random */
} EnemySpawnData;                       /* 0xC: one enemy type of a pack and how many of it */
typedef struct {
    EnemySpawnData* spawn_data;
    u16 num_enemy_types;
    u16 position_index;                 /* the pack's first position in the set's position list */
} EncounterPack;                        /* 8 bytes: one pack; a zone's encounter table names packs by index */
typedef struct EncounterSet {     /* the tag lets sMapData (quest_types.h) point to one */
    Vec3f** position_lists;             /* CreateEncounter reads the first list only: each enemy's offset from the
                                           fight's centre, x then z in a Vec3f's x and y */
    EncounterPack* pack_list;
    EncounterCircle* circles;
    s32 circle_count;
} EncounterSet;                         /* 0x10: a submap's fight spots and packs */

#endif
