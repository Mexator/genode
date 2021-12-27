This is a simple client-server scenario, where both hosts located at the same
subnet, server has static IP address, and client receives one from external 
(non-Genode) DHCP server.

At first, server creates a listening socket, then it binds address to it and 
starts listening.

Sockets are managed with Genode VFS plugin. When socket is created, Genode 
checks `<libc/>` attributes for `socket` path in VFS. In case of success, it
tries to open virtual file `[sock dir]/[tcp|udp]/new_socket`. `open()` returns 
a unique file descriptor that corresponds to newly created socket. Genode 
remembers all opened files by process and holds them in 
`File_descriptor_allocator` singleton.

After socket creation it is granted an IP address with `bind()`, which opens
virtual file `[socket_path]/bind`, converts `sockaddr` to string and writes
the string to the file.

The `listen()` call, similarly, opens `[socket_path]/listen` and writes there
size of clients queue (`backlog` parameter of `listen`). 

Accept call simply does an infinite loop that tries to read from 
`[socket_path]/accept` until data will appear there. When data appears there,
that means new client is ready to be accepted. After that a new socket is 
created and is read as a file with `recv()` or `read()`.

From the network point of view process looks like this:
Client sends a SYN packet;
Server sends ACK and SYN;
Client sends an empty ACK packet;
Client sends ACK packet with payload;
When client sent amount of bytes equal to window size, it sets PUSH flag on data packet;
Server replies with empty ACK;
When all data sent and ACKed, client sends segment with FIN and ACK set;
Server ACKs this packet;
Both sockets are destroyed.
