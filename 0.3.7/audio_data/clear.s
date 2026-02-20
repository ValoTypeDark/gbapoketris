    .section .rodata
    .align 2
    .global clear_pcm
clear_pcm:
    .incbin "audio_pcm/clear.pcm"
    .align 2
    .global clear_pcm_size
clear_pcm_size:
    .word 62897
