/* Copyright (C) 2006 - 2012 ScriptDev2 <http://www.scriptdev2.com/>
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

/* ScriptData
SDName: Boss_Kurinnaxx
SD%Complete: 80
SDComment: Summon Player ability NYI; Sand trap trigger NYI
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"

enum
{
    SPELL_TRASH             = 3391,
    SPELL_WIDE_SLASH        = 25814,
    SPELL_MORTAL_WOUND      = 25646,
    SPELL_SANDTRAP          = 25648,        // summons gameobject 180647
    SPELL_ENRAGE            = 26527,
    SPELL_SUMMON_PLAYER     = 26446,
};

struct MANGOS_DLL_DECL boss_kurinnaxxAI : public ScriptedAI
{
    boss_kurinnaxxAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiMortalWoundTimer;
    uint32 m_uiSandTrapTimer;
    uint32 m_uiTrashTimer;
    uint32 m_uiWideSlashTimer;
    bool m_bEnraged;

    void Reset()
    {
        m_bEnraged = false;

        m_uiMortalWoundTimer = urand(8000, 10000);
        m_uiSandTrapTimer    = urand(5000, 10000);
        m_uiTrashTimer       = urand(1000, 5000);
        m_uiWideSlashTimer   = urand(10000, 15000);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // If we are belowe 30% HP cast enrage
        if (!m_bEnraged && m_creature->GetHealthPercent() <= 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                m_bEnraged = true;
        }

        // Mortal Wound
        if (m_uiMortalWoundTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_WOUND) == CAST_OK)
                m_uiMortalWoundTimer = urand(8000, 10000);
        }
        else
            m_uiMortalWoundTimer -= uiDiff;

        // Sand Trap
        if (m_uiSandTrapTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1);
            if (!pTarget)
                pTarget = m_creature->getVictim();

            pTarget->CastSpell(pTarget, SPELL_SANDTRAP, true, NULL, NULL, m_creature->GetObjectGuid());
            m_uiSandTrapTimer = urand(10000, 15000);
        }
        else
            m_uiSandTrapTimer -= uiDiff;

        // Wide Slash
        if (m_uiWideSlashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WIDE_SLASH) == CAST_OK)
                m_uiWideSlashTimer = urand(12000, 15000);
        }
        else
            m_uiWideSlashTimer -= uiDiff;

        // Trash
        if (m_uiTrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH) == CAST_OK)
                m_uiTrashTimer = urand(12000, 17000);
        }
        else
            m_uiTrashTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kurinnaxx(Creature* pCreature)
{
    return new boss_kurinnaxxAI(pCreature);
}

void AddSC_boss_kurinnaxx()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_kurinnaxx";
    pNewScript->GetAI = &GetAI_boss_kurinnaxx;
    pNewScript->RegisterSelf();
}
