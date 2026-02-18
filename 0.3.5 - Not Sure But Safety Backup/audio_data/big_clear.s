    .section .rodata
    .align 2
    .global big_clear_pcm
big_clear_pcm:
    .incbin "audio_pcm/big_clear.pcm"
    .align 2
    .global big_clear_pcm_size
big_clear_pcm_size:
    .word 63109
