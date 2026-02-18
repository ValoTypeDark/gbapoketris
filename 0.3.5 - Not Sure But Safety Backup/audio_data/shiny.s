    .section .rodata
    .align 2
    .global shiny_pcm
shiny_pcm:
    .incbin "audio_pcm/shiny.pcm"
    .align 2
    .global shiny_pcm_size
shiny_pcm_size:
    .word 24948
