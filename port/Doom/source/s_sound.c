/**
 *  Doom Port to the nRF52840 by Nicola Wrachien (next-hack in the comments)
 *
 *  This port is based on the excellent doomhack's GBA Doom Port,
 *  with Kippykip additions.
 *  
 *  Several data structures and functions have been optimized 
 *  to fit in only 256kB RAM of nRF52840 (GBA has 384 kB RAM). 
 *  Z-Depth Light has been restored with almost no RAM consumption!
 *  Tons of speed optimizations have been done, and the game now
 *  runs extremely fast, despite the much higher 3D resolution with
 *  respect to GBA.
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright (C) 2021 Nicola Wrachien (next-hack in the comments)
 *  on nRF52840 port.
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
 * DESCRIPTION:  Platform-independent sound code
 *
 * next-hack: sounds lump are now pre-cached in immutable flash region,
 * so there is no need to find lump by name, but just using a index.
 * Tweaked some functions.
 *
 *-----------------------------------------------------------------------------*/

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "s_sound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"
#include "lprintf.h"

#include "global_data.h"
#include "pwm_audio.h"
// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200<<FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160<<FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PRIORITY 64
#define NORM_SEP      128

#define S_STEREO_SWING		(96*0x10000)

// number of channels available
static const unsigned int numChannels = MAX_CHANNELS;

//
// Internals.
//

void S_StopChannel(int cnum);

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep);

static int S_getChannel(void *origin, const sfxinfo_t *sfxinfo, int is_pickup);

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
    // next-hack preload all sound lump number
    uint32_t *soundLumps = Z_Malloc(NUMSFX * sizeof(uint32_t), PU_STATIC, NULL);
    for (int i = 0; i < NUMSFX; i++)
    {
        soundLumps[i] = W_CheckNumForName(S_sfx[i].name);
    }
    p_wad_immutable_flash_data->soundLumps = writeBufferToFlashRegion(soundLumps, NUMSFX * sizeof(uint32_t), FLASH_IMMUTABLE_REGION, true);
    Z_Free(soundLumps);
    //jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
    {

        lprintf(LO_CONFIRM, "S_Init: default sfx volume %d", sfxVolume);

        S_SetSfxVolume(sfxVolume);

        // Allocating the internal channels for mixing
        // (the maximum numer of sounds rendered
        // simultaneously) within zone memory.
        // CPhipps - calloc
        _g->channels = (channel_t*) calloc(numChannels, sizeof(channel_t));
    }

    // CPhipps - music init reformatted
    if (!nomusicparm)
    {
        S_SetMusicVolume(musicVolume);

        // no sounds are playing, and they are not mus_paused
        _g->mus_paused = 0;
    }
}

void S_Stop(void)
{
    unsigned int cnum;

    //jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
    {
        for (cnum = 0; cnum < numChannels; cnum++)
        {
            if (_g->channels[cnum].sfxinfo)
            {
                S_StopChannel(cnum);
            }
        }
        muteSound();
    }
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
    int mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)

    S_Stop();

    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    // start new music for the level
    _g->mus_paused = 0;

    if (_g->idmusnum != -1)
        mnum = _g->idmusnum; //jff 3/17/98 reload IDMUS music if not -1
    else if (_g->gamemode == commercial)
        mnum = mus_runnin + _g->gamemap - 1;
    else
    {
        static const int spmus[] =     // Song - Who? - Where?
        { mus_e3m4,     // American     e4m1
        mus_e3m2,     // Romero       e4m2
        mus_e3m3,     // Shawn        e4m3
        mus_e1m5,     // American     e4m4
        mus_e2m7,     // Tim  e4m5
        mus_e2m4,     // Romero       e4m6
        mus_e2m6,     // J.Anderson   e4m7 CHIRON.WAD
        mus_e2m5,     // Shawn        e4m8
        mus_e1m9      // Tim          e4m9
        };

        if (_g->gameepisode < 4)
            mnum = mus_e1m1 + (_g->gameepisode - 1) * 9 + _g->gamemap - 1;
        else
            mnum = spmus[_g->gamemap - 1];
    }
    S_ChangeMusic(mnum, true);
}

void S_StartSoundAtVolume(mobj_t *origin, int sfx_id, int volume)
{
    int /*priority, */cnum, is_pickup;
    const sfxinfo_t *sfx;

    int sep = NORM_SEP;
    if (origin) { debugi("%s origin x:%d y:%d\r\n", __func__, origin->x, origin->y); }
    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    is_pickup = (sfx_id & PICKUP_SOUND) || sfx_id == sfx_oof || (sfx_id == sfx_noway); // killough 4/25/98
    sfx_id &= ~PICKUP_SOUND;

    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
        I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

    sfx = &S_sfx[sfx_id];

    // Initialize sound parameters
    if (sfx->link)
    {
//        priority = sfx->priority;
        volume += sfx->volume;

        if (volume < 1)
            return;

        if (volume > _g->snd_SfxVolume)
            volume = _g->snd_SfxVolume;
    }
    else
    {
//        priority = NORM_PRIORITY;
    }

    // Check to see if it is audible, modify the params
    // killough 3/7/98, 4/25/98: code rearranged slightly

    if (!origin || origin == _g->player.mo)
    {
        volume *= 8;
    }
    else
    {
        if (!S_AdjustSoundParams(_g->player.mo, origin, &volume, &sep))
        {
            return;
        }
    }
    debugi("%s sfx: name:%s prio:%d singulatiry:%d ticks:%d vol:%d\r\n", __func__, sfx->name, sfx->priority, sfx->singularity, sfx->ticks, sfx->volume);
    for (int i = 0; i < numChannels; i++)
    {
        debugi("%s g.channels[%d]: handle:%d is_pick:%d sfx- name:%s prio:%d sing:%d ticks:%d vol:%d tickend:%d\r\n",
               __func__,
               i,
               _g->channels[i].handle,
               _g->channels[i].is_pickup,
                _g->channels[i].sfxinfo ? _g->channels[i].sfxinfo->name ? _g->channels[i].sfxinfo->name : "(null)" : "(null)",
                _g->channels[i].sfxinfo ? _g->channels[i].sfxinfo->priority : 0,
                _g->channels[i].sfxinfo ? _g->channels[i].sfxinfo->singularity: 0,
                _g->channels[i].sfxinfo ? _g->channels[i].sfxinfo->ticks : 0,
                _g->channels[i].sfxinfo ? _g->channels[i].sfxinfo->volume : 0,
               _g->channels[i].tickend);
    }

    // kill old sound
    for (cnum = 0; cnum < numChannels; cnum++)
    {
        if (_g->channels[cnum].sfxinfo && _g->channels[cnum].origin == origin && (_g->channels[cnum].is_pickup == is_pickup))
        {

            S_StopChannel(cnum);
            break;
        }
    }
    // try to find a channel
    cnum = S_getChannel(origin, sfx, is_pickup);

    if (cnum < 0)
    {
        return;
    }
    int h = I_StartSound(sfx_id, cnum, volume, sep);
    debugi("%s h:%d sfx_id:%d cnum:%d vol:%d sep:%d\r\n", __func__, h, sfx_id, cnum, volume, sep);
    if (h != -1)
    {
        _g->channels[cnum].handle = h;
        _g->channels[cnum].tickend = (_g->gametic + sfx->ticks);
    }

}

void S_StartSound(mobj_t *origin, int sfx_id)
{
    S_StartSoundAtVolume(origin, sfx_id, _g->snd_SfxVolume);
}

void S_StartSound2(degenmobj_t *origin, int sfx_id)
{
    //Look at this mess.

    //Originally, the degenmobj_t had
    //a thinker_t at the start of the struct
    //so that it could be passed around and
    //cast to a mobj_t* in the sound code
    //for non-mobj sound makers like doors.

    //This also meant that each and every sector_t
    //struct has 24 bytes wasted. I can't afford
    //to waste memory like that so we have a seperate
    //function for these cases which cobbles toget a temp
    //mobj_t-like struct to pass to the sound code.

    volatile static uint8_t index = 0;
    volatile static static_mobj_t tmpMobj[MAX_CHANNELS];
// next-hack: due to mobj_t member reordering, this stuff cannot be used. We can use a static_mobj_t instead,
// which is smaller than a regular mobj_t
    /*   struct fake_mobj
     {
     thinker_t ununsed;
     degenmobj_t origin;
     } fm;*/

    /* 
    gez: 
    originally there was a single tmpMobj on the stack but pointers are later
    assigned to this, so eventually they'll point to invalid places in RAM regarding
    sound origin coordinates, consequently sound should have had randodm interruptions.
    this static vars fix sadly wastes around 350 bytes.
    */
    index %= MAX_CHANNELS;
    tmpMobj[index].x = origin->x;
    tmpMobj[index].y = origin->y;
    S_StartSoundAtVolume((mobj_t*) &tmpMobj[index], sfx_id, _g->snd_SfxVolume);
    index++;
}

void S_StopSound(void *origin)
{
    int cnum;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {
        if (_g->channels[cnum].sfxinfo && _g->channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
    }
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (_g->mus_playing && !_g->mus_paused)
    {
        I_PauseSong(0);
        _g->mus_paused = true;
    }
}

void S_ResumeSound(void)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (_g->mus_playing && _g->mus_paused)
    {
        I_ResumeSong(0);
        _g->mus_paused = false;
    }
}

static boolean S_SoundIsPlaying(int cnum)
{
    const channel_t *channel = &_g->channels[cnum];

    if (channel->sfxinfo)
    {
        // next-hack: warning this is very bugged, because it does not count overflow.
        // On the other side, I don't think anyone is playing the same level for
        // 61,356,675 seconds (two years).
        int ticknow = _g->gametic;
        return (channel->tickend >= ticknow);   // this was bugged anyway, was <
    }

    return false;
}

//
// Updates music & sounds
//
void S_UpdateSounds(void *listener_p)
{
    //mobj_t *listener = (mobj_t*) listener_p;
    int cnum;
    //int sep = NORM_SEP;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    for (cnum = 0; cnum < numChannels; cnum++)
    {
        const sfxinfo_t *sfx;
        channel_t *c = &_g->channels[cnum];

        if ((sfx = c->sfxinfo))
        {
            if (S_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                int volume = _g->snd_SfxVolume;

                if (sfx->link)
                {
                    volume += sfx->volume;

                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                    else
                    {
                        if (volume > _g->snd_SfxVolume)
                            volume = _g->snd_SfxVolume;
                    }
                }
            }
            else   // if channel is allocated but sound has stopped, free it
            {
                S_StopChannel(cnum);
            }
        }
    }
}

void S_SetMusicVolume(int volume)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;
    if (volume < 0 || volume > 15)
        I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);
    I_SetMusicVolume(volume);
    _g->snd_MusicVolume = volume;
}

void S_SetSfxVolume(int volume)
{
    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;
    if (volume < 0 || volume > 127)
        I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);
    _g->snd_SfxVolume = volume;
}

// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;
    S_ChangeMusic(m_id, false);
}

void S_ChangeMusic(int musicnum, int looping)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (musicnum <= mus_None || musicnum >= NUMMUSIC)
        I_Error("S_ChangeMusic: Bad music number %d", musicnum);

    if (_g->mus_playing == musicnum)
        return;

    // shutdown old music
    S_StopMusic();

    // play it
    I_PlaySong(musicnum, looping);

    _g->mus_playing = musicnum;
}

void S_StopMusic(void)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (_g->mus_playing)
    {
        if (_g->mus_paused)
            I_ResumeSong(0);

        I_StopSong(0);

        _g->mus_playing = 0;
    }
}

void S_StopChannel(int cnum)
{
    int i;
    channel_t *c = &_g->channels[cnum];

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    if (c->sfxinfo)
    {
        // check to see
        //  if other channels are playing the sound
        for (i = 0; i < numChannels; i++)
        {
            // volatile mobj_t *check1_org = (mobj_t *)c->origin;
            // volatile mobj_t *check2_org = (mobj_t *)_g->channels[i].origin;
            // volatile const sfxinfo_t *check1_sfx = c->sfxinfo;
            // volatile const sfxinfo_t *check2_sfx = _g->channels[i].sfxinfo;
            // debugi("%s check1 cnum:%d handle:%d ispick:%d tickend:%d\r\n",
            //        __func__,
            //        cnum,
            //        c->handle,
            //        c->is_pickup,
            //        c->tickend);
            // debugi("%s check1 cnum:%d sfxinfo: name:%s prio:%d sing:%d ticks:%d vol:%d\r\n",
            //        __func__,
            //        cnum,
            //        check1_sfx ? check1_sfx->name ? check1_sfx->name : "(null)" : "(null)",
            //        check1_sfx ? check1_sfx->priority : 0,
            //        check1_sfx ? check1_sfx->singularity : 0,
            //        check1_sfx ? check1_sfx->ticks : 0,
            //        check1_sfx ? check1_sfx->volume : 0);
            // debugi("%s check1 cnum:%d org: x:%d y:%d\r\n",
            //        __func__,
            //        cnum,
            //        check1_org ? check1_org->x : 0,
            //        check1_org ? check1_org->y : 0);


            // debugi("%s check2 indx:%d handle:%d ispick:%d tickend:%d\r\n",
            //        __func__,
            //        i,
            //        _g->channels[i].handle,
            //        _g->channels[i].is_pickup,
            //        _g->channels[i].tickend);
            // debugi("%s check2 indx:%d sfxinfo: name:%s prio:%d sing:%d ticks:%d vol:%d\r\n",
            //        __func__,
            //        i,
            //        check2_sfx ? check2_sfx->name ? check2_sfx->name : "(null)" : "(null)",
            //        check2_sfx ? check2_sfx->priority : 0,
            //        check2_sfx ? check2_sfx->singularity : 0,
            //        check2_sfx ? check2_sfx->ticks : 0,
            //        check2_sfx ? check2_sfx->volume : 0);
            // debugi("%s check2 indx:%d org: x:%d y:%d\r\n",
            //        __func__,
            //        i,
            //        check2_org ? check2_org->x : 0,
            //        check2_org ? check2_org->y : 0);

            if (cnum != i && c->sfxinfo == _g->channels[i].sfxinfo)
                break;
        }

        // degrade usefulness of sound data
        c->sfxinfo = 0;
        c->tickend = 0;
    }
    debugi("%s killing cnum:%d with id:%d vol:%d\r\n", __func__, cnum, soundChannels[cnum].sfxIdx, soundChannels[cnum].volume);
    soundChannels[cnum].sfxIdx = 0;
    soundChannels[cnum].volume = 0;
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source, int *vol, int *sep)
{
    fixed_t adx, ady, approx_dist;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return 0;

    // e6y
    // Fix crash when the program wants to S_AdjustSoundParams() for player
    // which is not displayplayer and displayplayer was not spawned at the moment.
    // It happens in multiplayer demos only.
    //
    // Stack trace is:
    // P_SetupLevel() - P_LoadThings() - P_SpawnMapThing() - P_SpawnPlayer(players[0]) -
    // P_SetupPsprites() - P_BringUpWeapon() - S_StartSound(players[0]->mo, sfx_sawup) -
    // S_StartSoundAtVolume() - S_AdjustSoundParams(players[displayplayer]->mo, ...);
    // players[displayplayer]->mo is NULL
    //
    // There is no more crash on e1cmnet3.lmp between e1m2 and e1m3
    // http://competn.doom2.net/pub/compet-n/doom/coop/movies/e1cmnet3.zip

    if (!listener)
        return 0;

    // calculate the distance to sound origin
    // and clip it if necessary
    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);

    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

    if (!approx_dist)  // killough 11/98: handle zero-distance as special case
    {
        *vol = _g->snd_SfxVolume;
        return *vol > 0;
    }

    if (approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle_t angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

    if (angle <= listener->angle)
        angle += 0xffffffff;

    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
        *vol = _g->snd_SfxVolume * 8;
    else
        // distance effect
        *vol = (_g->snd_SfxVolume * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS) * 8) / S_ATTENUATOR;

    return (*vol > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
// killough 4/25/98: made static, added is_pickup argument

static int S_getChannel(void *origin, const sfxinfo_t *sfxinfo, int is_pickup)
{
    // channel number to use
    int cnum;
    channel_t *c;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return -1;

    // Find an open channel
    for (cnum = 0; cnum < numChannels && _g->channels[cnum].sfxinfo; cnum++)
    {
        debugi("%s finding cnum:%d sfxinfo: name:%s prio:%d sing:%d ticks:%d vol:%d\r\n",
               __func__,
               cnum,
               _g->channels[cnum].sfxinfo->name ? _g->channels[cnum].sfxinfo->name : "(null)",
               _g->channels[cnum].sfxinfo->priority,
               _g->channels[cnum].sfxinfo->singularity,
               _g->channels[cnum].sfxinfo->ticks,
               _g->channels[cnum].sfxinfo->volume);

        if (origin && _g->channels[cnum].origin)
        {
            debugi("%s comparing origin x:%d y:%d to g_origin[%d]: x:%d y:%d\r\n",
                   __func__,
                   ((mobj_t *)origin)->x,
                   ((mobj_t *)origin)->y,
                   cnum,
                   ((mobj_t *)_g->channels[cnum].origin)->x,
                   ((mobj_t *)_g->channels[cnum].origin)->y);
        }
        if (origin && _g->channels[cnum].origin == origin && _g->channels[cnum].is_pickup == is_pickup)
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == numChannels)
    {      // Look for lower priority
        for (cnum = 0; cnum < numChannels; cnum++)
            if (_g->channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;
        if (cnum == numChannels)
            return -1;                  // No lower priority.  Sorry, Charlie.
        else
        {
            S_StopChannel(cnum);        // Otherwise, kick out lower priority.
        }
    }

    c = &_g->channels[cnum];              // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;
    c->is_pickup = is_pickup;         // killough 4/25/98
    return cnum;
}

