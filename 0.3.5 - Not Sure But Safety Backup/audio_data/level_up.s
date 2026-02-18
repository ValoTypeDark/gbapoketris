    .section .rodata
    .align 2
    .global level_up_pcm
level_up_pcm:
    .incbin "audio_pcm/level_up.pcm"
    .align 2
    .global level_up_pcm_size
level_up_pcm_size:
    .word 63322
