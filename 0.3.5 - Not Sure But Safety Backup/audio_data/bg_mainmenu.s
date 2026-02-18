    .section .rodata
    .align 2
    .global bg_mainmenu_pcm
bg_mainmenu_pcm:
    .incbin "audio_pcm/bg_mainmenu.pcm"
    .align 2
    .global bg_mainmenu_pcm_size
bg_mainmenu_pcm_size:
    .word 2609280
