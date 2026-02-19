
.section .text
.global _start

_start:

    mov x8, 198      ; socket call
    mov x0, 2        ; ipv4 family
    mov x1, 2        ; sock_dgram for udp conns
    mov x2, 0        ; use default protocols   

    svc 0

    mov x19, x0     ; callee saved register
                    ; to save the fd inside x19 register

    mov x8, 206     ; send_to kernel call
    mov x0, x19     ; from the x19 register pass the fd into the call
    mov x1, msg 
    mov x2, msg_len 
    mov x3, 0       ; apply no flags & use default flags for packet buf
    
    mov x4, remote_addr 
    mov x5, 16      ; 16 because the sockaddr_in structure is 16 bits

    svc 0

    mov x8, 93      ; exit the entire process after sys calls
    mov x0, 0
    svc 0


.section data
    msg: .ascii "simulated encrypted data"
    msg_len = . - msg

    remote_addr:
        .hword 2            ; af_inet family
        .hword 0x391F       ; port 8000 in little Endian
        .byte  127,0,0,1    ; address 
        .zero. 8            ; padding ...