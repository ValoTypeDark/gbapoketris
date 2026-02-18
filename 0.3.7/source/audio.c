#include "audio.h"
#include <gba_types.h>
#include <gba_interrupt.h>

#define REG_SOUNDCNT_H  *((vu16*)0x04000082)
#define REG_SOUNDCNT_X  *((vu16*)0x04000084)
#define REG_FIFO_A      *((vu32*)0x040000A0)
#define REG_FIFO_B      *((vu32*)0x040000A4)
#define REG_TM0CNT_L    *((vu16*)0x04000100)
#define REG_TM0CNT_H    *((vu16*)0x04000102)
#define REG_TM1CNT_L    *((vu16*)0x04000104)
#define REG_TM1CNT_H    *((vu16*)0x04000106)
#define REG_DMA1SAD     *((vu32*)0x040000BC)
#define REG_DMA1DAD     *((vu32*)0x040000C0)
#define REG_DMA1CNT_L   *((vu16*)0x040000C4)
#define REG_DMA1CNT_H   *((vu16*)0x040000C6)
#define REG_DMA2SAD     *((vu32*)0x040000C8)
#define REG_DMA2DAD     *((vu32*)0x040000CC)
#define REG_DMA2CNT_L   *((vu16*)0x040000D0)
#define REG_DMA2CNT_H   *((vu16*)0x040000D2)

// ---------------------------------------------------------------------------
// Music: DMA1 + Timer0 (FIFO A), repeat mode — runs forever.
// Looping: audio_update() counts samples each VBlank. When the track is about
// to end, it restarts DMA1 from the beginning BEFORE the source pointer
// escapes the buffer. We restart one full frame early so there is always a
// safe margin.
//
// SFX: DMA2 + Timer1 (FIFO B), stopped by Timer1 IRQ after one play.
// ---------------------------------------------------------------------------

// Tracks are 181200 Hz mono 8-bit PCM. At 60 fps: 181200/60 = 3020 samples/frame.
#define SAMPLE_RATE        181200
#define SAMPLES_PER_FRAME  3020

// DMA1 control word for music (repeat FIFO mode — the only mode that works)
#define MUSIC_DMA_CTRL  ((1<<15) | (3<<12) | (1<<10) | (1<<9) | (2<<5))
//                        enable   FIFO-tim   32-bit   repeat   dst-fixed

static const s8* music_data    = 0;
static u32       music_size    = 0;   // bytes
static u32       music_played  = 0;   // samples counted this pass

static u32 sfx_remaining = 0;

// ── SFX IRQ ───────────────────────────────────────────────────────────────────

static void sfx_irq(void) {
    if (sfx_remaining > 0xFFFF) {
        sfx_remaining -= 0xFFFF;
        REG_TM1CNT_H = 0;
        REG_TM1CNT_L = 1;
        REG_TM1CNT_H = (1<<2) | (1<<6) | (1<<7);
    } else {
        REG_DMA2CNT_H   = 0;
        REG_TM1CNT_H    = 0;
        REG_SOUNDCNT_H |= (1<<15);
        irqDisable(IRQ_TIMER1);
        sfx_remaining = 0;
    }
}

// ── Internal helpers ──────────────────────────────────────────────────────────

static void restart_music_dma(void) {
    // Disable DMA, reset FIFO, restart from beginning of track
    REG_DMA1CNT_H   = 0;
    REG_SOUNDCNT_H |= (1<<11);

    REG_DMA1SAD   = (u32)music_data;
    REG_DMA1DAD   = (u32)&REG_FIFO_A;
    REG_DMA1CNT_L = 0;
    REG_DMA1CNT_H = MUSIC_DMA_CTRL;
}

// ── Public API ────────────────────────────────────────────────────────────────

void audio_init(void) {
    REG_SOUNDCNT_X = 0x80;
    REG_SOUNDCNT_H = (1<<2)  | (3<<8)  | (0<<10) | (1<<11)   // DSA: 100%, L+R, Timer0, reset
                   | (1<<3)  | (3<<12) | (0<<14) | (1<<15);   // DSB: 100%, L+R, Timer0, reset
}

void audio_play_music(const s8* data, u32 size) {
    if (!data || !size) return;

    music_data   = data;
    music_size   = size;
    music_played = 0;

    restart_music_dma();

    // Start Timer0 (sample clock)
    REG_TM0CNT_H = 0;
    REG_TM0CNT_L = (u16)(65536 - (16777216 / SAMPLE_RATE));
    REG_TM0CNT_H = (1<<7);
}

void audio_stop_music(void) {
    REG_DMA1CNT_H   = 0;
    REG_SOUNDCNT_H |= (1<<11);
    music_data   = 0;
    music_size   = 0;
    music_played = 0;
}

// Call once per VBlank. Restarts DMA with a safety margin before the buffer
// ends so the source pointer never escapes into adjacent ROM data.
// The DMA runs freely between VBlanks and can consume up to ~302 bytes in a
// single frame, so we restart 10 frames early (~3020 samples, ~0.17s).
// This tiny clip at the loop point is completely inaudible.
#define LOOP_SAFETY_FRAMES  10

void audio_update(void) {
    if (!music_data || !music_size) return;

    music_played += SAMPLES_PER_FRAME;

    if (music_played + (SAMPLES_PER_FRAME * LOOP_SAFETY_FRAMES) >= music_size) {
        music_played = 0;
        restart_music_dma();
    }
}

void audio_play_sfx(const s8* data, u32 size) {
    if (!data || !size) return;

    irqDisable(IRQ_TIMER1);
    REG_DMA2CNT_H   = 0;
    REG_TM1CNT_H    = 0;
    REG_SOUNDCNT_H |= (1<<15);

    sfx_remaining = size;

    if (!(REG_TM0CNT_H & (1<<7))) {
        REG_TM0CNT_L = (u16)(65536 - (16777216 / SAMPLE_RATE));
        REG_TM0CNT_H = (1<<7);
    }

    u16 t1_reload = (size > 0xFFFF) ? 1 : (u16)(65536 - size);
    if (size > 0xFFFF) sfx_remaining -= 0xFFFF;
    else               sfx_remaining  = 0;

    irqSet(IRQ_TIMER1, sfx_irq);
    irqEnable(IRQ_TIMER1);
    REG_TM1CNT_L = t1_reload;
    REG_TM1CNT_H = (1<<2) | (1<<6) | (1<<7);

    REG_DMA2SAD   = (u32)data;
    REG_DMA2DAD   = (u32)&REG_FIFO_B;
    REG_DMA2CNT_L = 0;
    REG_DMA2CNT_H = (1<<15) | (3<<12) | (1<<10) | (1<<9) | (2<<5);
}
