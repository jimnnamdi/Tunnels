
.global connect_tcp
type connect_tcp, %function 

.section .data
    serv:
        .hword 2            ; AF_INET
        .hword 0x1F39       ; port No
        .byte  127, 0, 0, 1 ; ip Addr
        .zero  8            ; padding for the ip/port sync

.section .bss 
    buffer: .skip 255

tcp_init:
   
    mov x8, 198
    mov x0, 2
    mov x1, 1
    mov x2, 0

    ; programmatic guardrail here to check
    ; that the socket call returns a val
    ; equals or greater than (0)

    svc 0
    cmp x0, 0
    blt err_msg

    mov x19, x0 
    ret

connect_tcp:
    mov x8, 203
    mov x0, x19 
    adr, serv
    mov x1, 16
    svc 0 

    cmp x0, 0
    blt err_msg
    ret 
