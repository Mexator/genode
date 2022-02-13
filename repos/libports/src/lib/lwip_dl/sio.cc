//
// Created by anton on 13.02.2022.
//

extern "C" {
#include <lwip/sio.h>

u32_t sio_tryread(sio_fd_t, u8_t *, u32_t) {
    return 0;
}

sio_fd_t sio_open(u8_t) {
    return nullptr;
}

void sio_send(u8_t, sio_fd_t) {
}

}

