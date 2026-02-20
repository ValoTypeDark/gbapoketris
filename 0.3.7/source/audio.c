#include "audio.h"
#include <gba_types.h>
#include <gba_interrupt.h>

#define REG_SOUNDCNT_H  *((vu16*)0x04000082)
#define REG_SOUNDCNT_X  *((vu16*)0x04000084)
#define REG_FIFO_B      *((vu32*)0x040000A4)
#define REG_TM0CNT_L    *((vu16*)0x04000100)
#define REG_TM0CNT_H    *((vu16*)0x04000102)
#define REG_TM1CNT_L    *((vu16*)0x04000104)
#define REG_TM1CNT_H    *((vu16*)0x04000106)
#define REG_DMA2SAD     *((vu32*)0x040000C8)
#define REG_DMA2DAD     *((vu32*)0x040000CC)
#define REG_DMA2CNT_L   *((vu16*)0x040000D0)
#define REG_DMA2CNT_H   *((vu16*)0x040000D2)

static u32 sfx_remaining = 0;

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

void audio_init(void) {
    REG_SOUNDCNT_X = 0x80;
    REG_SOUNDCNT_H = (1<<3)    // DSB 100%
                   | (3<<12)   // DSB L+R
                   | (1<<14)   // DSB Timer1
                   | (1<<15);  // DSB reset
}

void audio_play_sfx(const s8* data, u32 size) {
    if (!data || !size) return;

    irqDisable(IRQ_TIMER1);
    REG_DMA2CNT_H   = 0;
    REG_TM1CNT_H    = 0;
    REG_SOUNDCNT_H |= (1<<15);

    sfx_remaining = size;

    // Start Timer0 (sample clock at 18000Hz)
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
