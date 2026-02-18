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

// Music: DMA1 + Timer0 (FIFO A) - loops forever
// SFX:   DMA2 + Timer1 (FIFO B) - stops after playing once

static u32 sfx_remaining = 0;

static void sfx_irq(void) {
    if (sfx_remaining > 0xFFFF) {
        sfx_remaining -= 0xFFFF;
        REG_TM1CNT_H = 0;
        REG_TM1CNT_L = 1;
        REG_TM1CNT_H = (1<<2) | (1<<6) | (1<<7);
    } else {
        // Stop DMA2 and Timer1, but DON'T stop Timer0 (music needs it)
        REG_DMA2CNT_H   = 0;
        REG_TM1CNT_H    = 0;
        REG_SOUNDCNT_H |= (1<<15);  // reset FIFO B only
        irqDisable(IRQ_TIMER1);
        sfx_remaining = 0;
    }
}

void audio_init(void) {
    REG_SOUNDCNT_X = 0x80;
    // Channel A (music): 100% vol, L+R, Timer0
    // Channel B (sfx):   100% vol, L+R, Timer0 (shared timer)
    REG_SOUNDCNT_H = (1<<2)    // DSA 100%
                   | (3<<8)    // DSA L+R
                   | (0<<10)   // DSA Timer0
                   | (1<<11)   // DSA reset
                   | (1<<3)    // DSB 100%
                   | (3<<12)   // DSB L+R
                   | (0<<14)   // DSB Timer0
                   | (1<<15);  // DSB reset
}

void audio_play_music(const s8* data, u32 size) {
    // Stop any existing music
    REG_DMA1CNT_H = 0;
    REG_SOUNDCNT_H |= (1<<11);  // reset FIFO A

    // Music uses DMA repeat and loops forever
    REG_DMA1SAD   = (u32)data;
    REG_DMA1DAD   = (u32)&REG_FIFO_A;
    REG_DMA1CNT_L = 0;
    REG_DMA1CNT_H = (1<<15)   // enable
                  | (3<<12)   // FIFO timing
                  | (1<<10)   // 32-bit
                  | (1<<9)    // repeat
                  | (2<<5);   // dst fixed

    // Start Timer0 if not already running (shared with SFX)
    if (!(REG_TM0CNT_H & (1<<7))) {
        REG_TM0CNT_L = (u16)(65536 - (16777216 / 18000));
        REG_TM0CNT_H = (1<<7);  // enable
    }
}

void audio_stop_music(void) {
    REG_DMA1CNT_H = 0;
    REG_SOUNDCNT_H |= (1<<11);
}

void audio_play_sfx(const s8* data, u32 size) {
    irqDisable(IRQ_TIMER1);
    REG_DMA2CNT_H   = 0;
    REG_TM1CNT_H    = 0;
    REG_SOUNDCNT_H |= (1<<15);

    sfx_remaining = size;

    // Start Timer0 if not already running (shared with music)
    if (!(REG_TM0CNT_H & (1<<7))) {
        REG_TM0CNT_L = (u16)(65536 - (16777216 / 18000));
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
