
.global create_raw_sockets
type create_raw_sockets, %function

.global bind_raw_sockets
type bind_raw_sockets, %function

.global listen_to_conns
type listen_to_conns, %function

.section .data

    ; this structure should be 16 bits (2 bytes)
    csockaddr:
        .hword 2
        .hword 0x391F   ; little endian format
        .byte 127,0,0,1 ; IP Address to match (ip/port) format
        .zero 8
    
    error_mesg: .asciz "tcp stream connection failed"
    error_mesg_len = . - error_mesg

.section .bss 
    buffer 256

create_raw_sockets:

    mov x8, 198
    mov x0, 2
    mov x1, 1
    mov x2, 0

    ; programmatic guardrail here to check
    ; that the socket call returns a val
    ; equals or greater than (0)

    cbnz x0, ret 
    mov x19, x0 
    bl bind_raw_sockets

bind_raw_sockets:
    
    ; for binding raw tcp socket to a file descriptor
    ; we want to pass the fd, and then the packet structure ?
    
    mov x0, x19 
    adr x1, =csockaddr
    mov x2, 16          ; 16 bits
    mov w8, 200         ; syscall for bind
    svc 0

    cbnz .err_msg

listen_to_conns:

    mov x8, 201
    mov x0, x19 
    mov x1, 5           ; for now we could limit our listen
                        ; connections to 5 @ todo: make dynamic

    svc 0

accept_conns:
    
    ; we accept client connections at this point
    ; and before we proceed to reads & writes ...
    
    mov x8, 202
    mov x0, x19 
    adr x1, =csockaddr
    mov x2, 16

    mov x20, x0         ; the file descriptor returned from the 
                        ; accept conn call is stored in x20 register

    cbnz .err_msg

loop:

    mov x8, 63
    mov x0, x20 
    mov x1, buffer
    mov x2, 256

    cbnz err_msg
    jmp loop            ; this is like a continuous loop for reads
                        ; how do i handle writes ?

.err_msg:

    ; write the error to stderr
   
    mov x8, 64          ; sys write
    mov x0, 2           ; stderr buf
    adr x1, =error_mesg    ; error message
    mov x2, error_mesg_len 
    svc 0               ; supervisor call to trigger call
    
ret:
    ; then exit the process if failed
    mov x8, 93
    mov x0, 0
    svc 0
