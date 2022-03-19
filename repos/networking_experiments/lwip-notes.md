## Some notes I want not to forget

Lwip options cat be modified at `repos/libports/include/lwip/lwipopts.h`

To make symbols from lwip transitively visible for libraries that use lwip_vfs 
you need to add needed symbols to `global` section of `repos/libports/src/lib/vfs/lwip/symbol.map`

Also there are warnings about missing `lwip_strerr()` and redefined `htons`. I
don't know now how to fix that. Future Anton, that job is for you.

Also for the program to compile I added 
```
INC_DIR += /genode/contrib/lwip-6e0661b21fde397041389d5d8db906b5a6543700/include/lwip
INC_DIR += /genode/repos/libports/include/lwip/
```
to `target.mk` of `ptcp_test`. Try to do it with `port_dir`.

Don't forget about these macros. Code that is dependent of lwip won't compile
without them:
```
#define LWIP_NO_STDINT_H 0
#define LWIP_NO_UNISTD_H 0
#define SSIZE_MAX
```

Below run file for gdb on nova with ptcp

```expect
build {
    test/ptcp_test lib/vfs_ptcp lib/vfs/lwip
    app/gdb_monitor
    lib/gdbserver_platform-nova
    drivers/uart
    lib/vfs/pipe
}
create_boot_directory

import_from_depot [depot_user]/src/[base_src] \
                  [depot_user]/pkg/[drivers_nic_pkg] \
                  [depot_user]/src/init \
                  [depot_user]/src/nic_router

install_config {
<config verbose="yes" prio_levels="2">
    <parent-provides>
		<service name="ROM"/>
		<service name="IRQ"/>
		<service name="IO_MEM"/>
		<service name="IO_PORT"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="CPU"/>
		<service name="LOG"/>
	</parent-provides>

    <default-route>
        <any-service> <parent/> <any-child/> </any-service>
    </default-route>

    <default caps="100"/>

	<start name="timer">
		<resource name="RAM" quantum="2M"/>
		<provides> <service name="Timer"/> </provides>
	</start>
    <start name="pc_uart_drv">
        <resource name="RAM" quantum="2M"/>
        <provides>
            <service name="Terminal"/>
            <service name="Uart"/>
        </provides>
        <config>
            <policy label_prefix="gdb_monitor" uart="1"/>
        </config>
    </start>
	<start name="drivers" caps="1000" managing_system="yes" priority="-1">
		<resource name="RAM" quantum="320M"/>
		<binary name="init"/>
		<route>
			<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
			<service name="Timer"> <child name="timer"/> </service>
			<service name="Uplink"> <child name="nic_router"/> </service>
			<any-service> <parent/> </any-service>
		</route>
	</start>

    <start name="nic_router" caps="200">
		<resource name="RAM" quantum="100M"/>
		<provides>
			<service name="Nic"/>
			<service name="Uplink"/>
		</provides>
		<config dhcp_discover_timeout_sec="1">

			<policy label_prefix="ptcp_test" domain="downlink"/>
			<policy label_prefix="drivers"           domain="uplink"/>

			<domain name="uplink">

				<nat domain="downlink"
				     tcp-ports="16384"
				     udp-ports="16384"
				     icmp-ids="16384"/>

				<tcp-forward port="80"  domain="downlink" to="10.0.3.2"/>

			</domain>

			<domain name="downlink" interface="10.0.3.1/24">

				<dhcp-server ip_first="10.0.3.2"
				             ip_last="10.0.3.2"
				             dns_config_from="uplink"/>

				<tcp dst="0.0.0.0/0"><permit-any domain="uplink" /></tcp>
				<udp dst="0.0.0.0/0"><permit-any domain="uplink" /></udp>
				<icmp dst="0.0.0.0/0" domain="uplink"/>

			</domain>

		</config>
	</start>

    <start name="gdb_monitor" caps="2000">
        <resource name="RAM" quantum="30M"/>
        <route>
        	<service name="Nic"> <child name="nic_router"/> </service>
        	<any-service> <parent/> <any-child/> </any-service>
        </route>
        <config>
        <preserve name="RAM" quantum="15M"/>
        <target name="ptcp_test" caps="100">
            <config  verbose="yes" ld_verbose="yes" >
                <vfs>
            	    <dir name="dev">    <log/>             </dir>
            		<dir name="socket"> <lwip dhcp="yes"/> </dir>
            	</vfs>
            	<libc stdout="/dev/log" stderr="/dev/log" socket="/socket"/>
            </config>
        </target>
        <vfs>
            <dir name="dev">
                <log/>
                <terminal raw="yes"/>
                <inline name="rtc">2021-01-01 00:01</inline>
            </dir>
            <dir name="pipe"> <pipe/> </dir>
        </vfs>
        <libc stdout="/dev/log" stderr="/dev/log" pipe="/pipe" rtc="/dev/rtc"/>
    </config>
    </start>

</config>
}
# evaluated by the run tool
proc binary_name_gdbserver_platform_lib_so { } {
        return "gdbserver_platform-$::env(KERNEL).lib.so"
}

build_boot_image {
    libc.lib.so libm.lib.so vfs.lib.so stdcxx.lib.so
    pc_uart_drv vfs_pipe.lib.so
    gdb_monitor gdbserver_platform.lib.so

    ptcp_test

    vfs_ptcp.lib.so vfs_lwip.lib.so
}

```
