    .section .rodata
    .align 2
    .global bg_pokedex_pcm
bg_pokedex_pcm:
    .incbin "audio_pcm/bg_pokedex.pcm"
    .align 2
    .global bg_pokedex_pcm_size
bg_pokedex_pcm_size:
    .word 2583360
