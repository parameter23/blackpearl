#include "music.h"
#include "sfx.h"

uint8_t music_enabled = 0;

typedef struct {
    uint16_t freq;
    uint16_t duration;
} MusicNote;

/* ---------------------------------------------------------
 * Dark Synthwave Bassline (ca. 25 Sekunden Loop)
 * --------------------------------------------------------- */

static const MusicNote music_track[] = {
    { 55, 400 }, { 0, 100 },
    { 65, 400 }, { 0, 100 },
    { 73, 400 }, { 0, 100 },
    { 82, 600 }, { 0, 200 },

    { 55, 400 }, { 0, 100 },
    { 65, 400 }, { 0, 100 },
    { 73, 400 }, { 0, 100 },
    { 92, 600 }, { 0, 200 },

    { 55, 300 }, { 65, 300 }, { 73, 300 }, { 82, 300 },
    { 92, 300 }, { 110, 300 }, { 92, 300 }, { 82, 300 },

    { 0, 500 },
};

static const uint16_t music_length = sizeof(music_track) / sizeof(MusicNote);

static uint16_t note_timer = 0;
static uint16_t note_index = 0;

/* ---------------------------------------------------------
 * Musik starten
 * --------------------------------------------------------- */
void music_play(void)
{
    music_enabled = 1;
    note_index = 0;
    note_timer = music_track[0].duration;
}

/* ---------------------------------------------------------
 * Musik stoppen
 * --------------------------------------------------------- */
void music_stop(void)
{
    music_enabled = 0;
    sfx_stop();
}

/* ---------------------------------------------------------
 * Initialisierung
 * --------------------------------------------------------- */
void music_init(void)
{
    music_enabled = 0;
    note_index = 0;
    note_timer = 0;
}

/* ---------------------------------------------------------
 * Wird im SysTick (1 ms) aufgerufen
 * --------------------------------------------------------- */
void music_update_1ms(void)
{
    if (!music_enabled)
        return;

    if (note_timer > 0) {
        note_timer--;
        return;
    }

    note_index++;
    if (note_index >= music_length)
        note_index = 0;

    uint16_t freq = music_track[note_index].freq;
    uint16_t dur  = music_track[note_index].duration;

    note_timer = dur;

    if (freq == 0) {
        sfx_stop();
    } else {
        sfx_play_freq(freq);
    }
}
