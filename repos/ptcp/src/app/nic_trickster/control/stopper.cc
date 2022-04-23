#include <nic_trickster/control/stopper.h>

Genode::Mutex Stopper::mutex;

void Stopper::suspend() {
    mutex.acquire();
}

void Stopper::resume() {
    mutex.release();
}