
.global create_tcp_sockets
type create_tcp_sockets, %function

.global bind_tcp_sockets
type bind_tcp_sockets, %function

.global listen_to_conns
type listen_to_conns, %function

.section .data

    ; this structure should be 16 bits (2 bytes)
    csockaddr:
        .hword 2
        .hword 0x1F39   ; big endian format
        .byte 127,0,0,1 ; IP Address to match (ip/port) format
        .zero 8
    
    error_mesg: .asciz "tcp stream connection failed"
    error_mesg_len = . - error_mesg

.section .bss 
    buffer .skip 255    ; static allocation of 255 bytes of mem

create_tcp_sockets:

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

bind_tcp_sockets:
    
    ; for binding raw tcp socket to a file descriptor
    ; we want to pass the fd, and then the packet structure ?
    
    mov x0, x19 
    adr x1, =csockaddr
    mov x2, 16          ; 16 bytes
    mov w8, 200         ; syscall for bind
    svc 0

    cmp x0, 0
    blt err_msg 
    ret

listen_to_conns:

    mov x8, 201
    mov x0, x19 
    mov x1, 5           ; for now we could limit our listen
                        ; connections to 5 @ todo: make dynamic

    svc 0

    cmp x0, 0
    blt err_msg
    ret

accept_conns:
    
    ; we accept client connections at this point
    ; and before we proceed to reads & writes ...
    
    mov x8, 202
    mov x0, x19 
    adr x1, =csockaddr
    mov x2, 16
    svc 0

    cmp x0,0
    mov x20, x0         ; the file descriptor returned from the 
                        ; accept conn call is stored in x20 register
    blt err_msg
    ret 

loop:

    mov x8, 63
    mov x0, x20 
    adr x1, buffer
    mov x2, 255
    svc 0

    mov x2, x0          ; the current N bytes from the read operation
    mov x8, 64 
    mov x0, x20 
    adr x1, buffer 
    svc 0
    
    b loop              ; this is like a continuous loop for reads
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
