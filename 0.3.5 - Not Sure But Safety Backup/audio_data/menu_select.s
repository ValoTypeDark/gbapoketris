    .section .rodata
    .align 2
    .global menu_select_pcm
menu_select_pcm:
    .incbin "audio_pcm/menu_select.pcm"
    .align 2
    .global menu_select_pcm_size
menu_select_pcm_size:
    .word 15970
