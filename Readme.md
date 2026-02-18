# ARM64-TCP-Network

A minimalist, high-performance TCP Server written entirely in **ARM64 (AArch64) Assembly** for Linux. This project bypasses standard C libraries (libc) to interact directly with the kernel via software interrupts.

### Overview

This project demonstrates low-level socket programming using the Linux syscall interface. It implements the standard Berkeley Sockets lifecycle:

1. **Socket Creation** (`sys_socket`)
2. **Address Binding** (`sys_bind`)
3. **Passive Listening** (`sys_listen`)
4. **Connection Acceptance** (`sys_accept`)
5. **I/O Loop** (`sys_read` & `sys_write`)

### Technical Specifications

* **Architecture:** ARM64 (AArch64)
* **Platform:** Linux
* **Default Port:** 8089 (Configurable in `.data` section)
* **Memory Footprint:** < 2KB
* **Dependencies:** None (No `libc` required)

### Syscall Table Reference

| Function | Syscall ID (x8) | Purpose |
| :--- | :--- | :--- |
| `socket` | 198 | Initialize the IPv4 TCP socket |
| `bind` | 200 | Bind to 127.0.0.1:port |
| `listen` | 201 | Set backlog queue depth |
| `accept` | 202 | Block until a client connects |
| `read` | 63 | Ingest data into the BSS buffer |
| `write` | 64 | Echo data back to the client |

### Build & Run

To assemble and link this project, you will need `as` (GNU Assembler) and `ld` (GNU Linker).

```bash
# 1. Assemble the source
as -o listener.o listener.s

# 2. Link the object file
ld -o listener listener.o

# 3. Execute
./listener