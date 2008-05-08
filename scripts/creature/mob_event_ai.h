/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "sc_creature.h"
#define MAX_ACTIONS     3

enum Event_Types
{
    EVENT_T_TIMER_REPEAT            = 0,    //Time, Initial, Random
    EVENT_T_TIMER_SINGLE            = 1,    //Time
    EVENT_T_TIMER_OOC_REPEAT        = 2,    //Time, Initial, Random
    EVENT_T_TIMER_OOC_SINGLE        = 3,    //Time
    EVENT_T_HP                      = 4,    //HPMax%, HPMin%, TimeUntilRepeat
    EVENT_T_MANA                    = 5,    //ManaMax%,ManaMin% TimeUntilRepeat
    EVENT_T_AGGRO                   = 6,    //NONE
    EVENT_T_KILL                    = 7,    //TimeUntilRepeat
    EVENT_T_DEATH                   = 8,    //NONE
    EVENT_T_EVADE                   = 9,    //NONE
    EVENT_T_SPELLHIT                = 10,   //SpellID, School, TimeUntilRepeat
    EVENT_T_RANGE                   = 11,   //MinDist, MaxDist, TimeUnitlRepeat
    EVENT_T_OOC_LOS                 = 12,   //NoHostile, NoFriendly, TimeUntilRepeat
    EVENT_T_SPAWNED                 = 13,   //NONE
    EVENT_T_TARGET_HP               = 14,   //HPMax%, HPMin%, TimeUntilRepeat
    EVENT_T_TARGET_CASTING          = 15,   //TimeUntilRepeat
    
    EVENT_T_END,
};

enum Action_Types
{
    ACTION_T_NONE                   = 0,    //No action
    ACTION_T_SAY                    = 1,    //TextId
    ACTION_T_YELL                   = 2,    //TextId
    ACTION_T_TEXTEMOTE              = 3,    //TextId
    ACTION_T_SOUND                  = 4,    //SoundId
    ACTION_T_EMOTE                  = 5,    //EmoteId
    ACTION_T_RANDOM_SAY             = 6,    //TextId1, TextId2, TextId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_YELL            = 7,    //TextId1, TextId2, TextId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_TEXTEMOTE       = 8,    //TextId1, TextId2, TextId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_SOUND           = 9,    //SoundId1, SoundId2, SoundId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_EMOTE           = 10,   //EmoteId1, EmoteId2, EmoteId3 (-1 in any field means no output if randomed that field)
    ACTION_T_CAST                   = 11,   //SpellId, Target, CastFlags
    ACTION_T_SUMMON                 = 12,   //CreatureID, Target, Duration in ms
    ACTION_T_THREAT_SINGLE_PCT      = 13,   //Threat%, Target
    ACTION_T_THREAT_ALL_PCT         = 14,   //Threat%
    ACTION_T_QUEST_EVENT            = 15,   //QuestID, Target
    ACTION_T_CASTCREATUREGO         = 16,   //QuestID, SpellId, Target
    ACTION_T_SET_UNIT_FIELD         = 17,   //Field_Number, Value, Target
    ACTION_T_SET_UNIT_FLAG          = 18,   //Flags (may be more than one field OR'd together), Target
    ACTION_T_REMOVE_UNIT_FLAG       = 19,   //Flags (may be more than one field OR'd together), Target
    ACTION_T_AUTO_ATTACK            = 20,   //AllowAttackState (0 = stop attack, anything else means continue attacking)
    ACTION_T_COMBAT_MOVEMENT        = 21,   //AllowCombatMovement (0 = stop combat based movement, anything else continue attacking)
    ACTION_T_SET_PHASE              = 22,   //Phase
    ACTION_T_INC_PHASE              = 23,   //Value (may be negative to decrement phase, should not be 0)
    ACTION_T_EVADE                  = 24,   //No Params
    ACTION_T_FLEE                   = 25,   //No Params
    ACTION_T_QUEST_EVENT_ALL        = 26,   //QuestID
    ACTION_T_CASTCREATUREGO_ALL     = 27,   //QuestId, SpellId
    ACTION_T_REMOVEAURASFROMSPELL   = 28,   //Target, Spellid
    ACTION_T_RANGED_MOVEMENT        = 29,   //Distance, Angle
    ACTION_T_RANDOM_PHASE           = 30,   //PhaseId1, PhaseId2, PhaseId3
    ACTION_T_RANDOM_PHASE_RANGE     = 31,   //PhaseMin, PhaseMax
    ACTION_T_SUMMON_ID              = 32,   //CreatureId, Target, SpawnId
    ACTION_T_KILLED_MONSTER         = 33,   //CreatureId, Target
    ACTION_T_SET_INST_DATA          = 34,   //Field, Data
    ACTION_T_SET_INST_DATA64        = 35,   //Field, Target

    ACTION_T_END,
};

enum Target
{
    TARGET_T_SELF = 0,                  //Self cast
    TARGET_T_HOSTILE,                   //Our current target (ie: highest aggro)
    TARGET_T_HOSTILE_SECOND_AGGRO,      //Second highest aggro (generaly used for cleaves and some special attacks)
    TARGET_T_HOSTILE_LAST_AGGRO,        //Dead last on aggro (no idea what this could be used for)
    TARGET_T_HOSTILE_RANDOM,            //Just any random target on our threat list
    TARGET_T_HOSTILE_RANDOM_NOT_TOP,    //Any random target except top threat
    TARGET_T_ACTION_INVOKER,            //Unit who caused this Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS)

    TARGET_T_END
    //CAST_FRIENDLY_RANDOM,             //NOT YET IMPLEMENTED
};

enum CastFlags
{
    CAST_INTURRUPT_PREVIOUS     = 0x01,     //Interrupt any spell casting
    CAST_TRIGGERED              = 0x02,     //Triggered (this makes spell cost zero mana and have no cast time)
    CAST_FORCE_CAST             = 0x04,     //Forces cast even if creature is out of mana or out of range
    CAST_NO_MELEE_IF_OOM        = 0x08,     //Prevents creature from entering melee if out of mana or out of range
    CAST_FORCE_TARGET_SELF      = 0x10,     //Forces the target to cast this spell on itself
};

struct EventAI_Event
{
    uint32 creature_id;

    uint16 event_type;
    uint32 event_inverse_phase_mask;
    uint8 event_chance;
    union
    {
        uint32 event_param1;
        int32 event_param1_s;
    };
    union
    {
        uint32 event_param2;
        int32 event_param2_s;
    };
    union
    {
        uint32 event_param3;
        int32 event_param3_s;
    };

    struct _action
    {
        uint16 type;
        union
        {
            uint32 param1;
            int32 param1_s;
        };
        union
        {
            uint32 param2;
            int32 param2_s;
        };
        union
        {
            uint32 param3;
            int32 param3_s;
        };
    }action[MAX_ACTIONS];
};

extern HM_NAMESPACE::hash_map<uint32, EventAI_Event> Event_Map;

struct EventAI_Summon
{
    uint32 id;

    float position_x;
    float position_y;
    float position_z;
    float orientation;
    uint32 SpawnTimeSecs;
};

extern HM_NAMESPACE::hash_map<uint32, EventAI_Summon> EventSummon_Map;


