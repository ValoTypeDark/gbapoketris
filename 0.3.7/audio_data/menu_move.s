    .section .rodata
    .align 2
    .global menu_move_pcm
menu_move_pcm:
    .incbin "audio_pcm/menu_move.pcm"
    .align 2
    .global menu_move_pcm_size
menu_move_pcm_size:
    .word 15970
