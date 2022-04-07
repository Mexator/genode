# Repository with test implementation of persistent networking stack

- `net.sh`
Script that creates virtual bridge interface onto which qemu VMs would be
connected. Should be launched with sudo.

- `src/test/ptcp_test`
Scenario that uses ptcp library to test and demonstrate its work.