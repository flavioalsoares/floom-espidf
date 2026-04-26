/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_ENEMY__
#define __P_ENEMY__

#include "p_mobj.h"

void P_NoiseAlert (mobj_t *target, mobj_t *emmiter);
void P_SpawnBrainTargets(void); /* killough 3/26/98: spawn icon landings */

extern struct brain_s {         /* killough 3/26/98: global state of boss brain */
  int easy, targeton;
} brain;

// ********************************************************************
// Function addresses or Code Pointers
// ********************************************************************
// These function addresses are the Code Pointers that have been
// modified for years by Dehacked enthusiasts.  The new BEX format
// allows more extensive changes (see d_deh.c)

struct mobj_s;
struct player_s;
typedef struct pspdef_s pspdef_t;

void A_Explode(struct mobj_s *actor);
void A_Pain(struct mobj_s *actor);
void A_PlayerScream(struct mobj_s *actor);
void A_Fall(struct mobj_s *actor);
void A_XScream(struct mobj_s *actor);
void A_Look(struct mobj_s *actor);
void A_Chase(struct mobj_s *actor);
void A_FaceTarget(struct mobj_s *actor);
void A_PosAttack(struct mobj_s *actor);
void A_Scream(struct mobj_s *actor);
void A_SPosAttack(struct mobj_s *actor);
void A_VileChase(struct mobj_s *actor);
void A_VileStart(struct mobj_s *actor);
void A_VileTarget(struct mobj_s *actor);
void A_VileAttack(struct mobj_s *actor);
void A_StartFire(struct mobj_s *actor);
void A_Fire(struct mobj_s *actor);
void A_FireCrackle(struct mobj_s *actor);
void A_Tracer(struct mobj_s *actor);
void A_SkelWhoosh(struct mobj_s *actor);
void A_SkelFist(struct mobj_s *actor);
void A_SkelMissile(struct mobj_s *actor);
void A_FatRaise(struct mobj_s *actor);
void A_FatAttack1(struct mobj_s *actor);
void A_FatAttack2(struct mobj_s *actor);
void A_FatAttack3(struct mobj_s *actor);
void A_BossDeath(struct mobj_s *actor);
void A_CPosAttack(struct mobj_s *actor);
void A_CPosRefire(struct mobj_s *actor);
void A_TroopAttack(struct mobj_s *actor);
void A_SargAttack(struct mobj_s *actor);
void A_HeadAttack(struct mobj_s *actor);
void A_BruisAttack(struct mobj_s *actor);
void A_SkullAttack(struct mobj_s *actor);
void A_Metal(struct mobj_s *actor);
void A_SpidRefire(struct mobj_s *actor);
void A_BabyMetal(struct mobj_s *actor);
void A_BspiAttack(struct mobj_s *actor);
void A_Hoof(struct mobj_s *actor);
void A_CyberAttack(struct mobj_s *actor);
void A_PainAttack(struct mobj_s *actor);
void A_PainDie(struct mobj_s *actor);
void A_KeenDie(struct mobj_s *actor);
void A_BrainPain(struct mobj_s *actor);
void A_BrainScream(struct mobj_s *actor);
void A_BrainDie(struct mobj_s *actor);
void A_BrainAwake(struct mobj_s *actor);
void A_BrainSpit(struct mobj_s *actor);
void A_SpawnSound(struct mobj_s *actor);
void A_SpawnFly(struct mobj_s *actor);
void A_BrainExplode(struct mobj_s *actor);
void A_Die(struct mobj_s *actor);
void A_Detonate(struct mobj_s *actor);
void A_Mushroom(struct mobj_s *actor);
void A_Spawn(struct mobj_s *actor);
void A_Turn(struct mobj_s *actor);
void A_Face(struct mobj_s *actor);
void A_Scratch(struct mobj_s *actor);
void A_PlaySound(struct mobj_s *actor);
void A_RandomJump(struct mobj_s *actor);
void A_LineEffect(struct mobj_s *actor);
void A_OpenShotgun2(struct player_s *player, pspdef_t *psp);
void A_LoadShotgun2(struct player_s *player, pspdef_t *psp);
void A_CloseShotgun2(struct player_s *player, pspdef_t *psp);

#endif // __P_ENEMY__
