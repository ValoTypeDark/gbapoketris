    .section .rodata
    .align 2
    .global bg_gameplay_pcm
bg_gameplay_pcm:
    .incbin "audio_pcm/bg_gameplay.pcm"
    .align 2
    .global bg_gameplay_pcm_size
bg_gameplay_pcm_size:
    .word 3340800
