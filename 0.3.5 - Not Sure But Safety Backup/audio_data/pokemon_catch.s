    .section .rodata
    .align 2
    .global pokemon_catch_pcm
pokemon_catch_pcm:
    .incbin "audio_pcm/pokemon_catch.pcm"
    .align 2
    .global pokemon_catch_pcm_size
pokemon_catch_pcm_size:
    .word 62854
