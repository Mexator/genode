# Why virsh

Recently, as I needed to test work of checkpointing and restoring with TCP
connection, I have switched from bare `qemu` to more advanced mechanism, that
involves libvirt. 

## TODO: Research what libvirt does in more detail

libvirt allows to checkpoint and restore virtual machines. My roadmap with it 
is following:

-[x] Run any ISO image created by Genode build system.
  
  No adjustments were needed. Running it works perfectly fine, if done from
  virsh. If it is done from Virtual Machine Manager GUI, there is no console
  output visible. 

  I created an XML VM config file, that is located at `stuff/vm1.xml` (Name 
  will be changed later).

  Steps to run:
    - Open the XML file with editor and find line that defines CDROM.
    - Add `<source file='path/to/iso'>` tag inside of `<disk ... cdrom'>`
    - Change MAC address to be unique if you want proper networking
    - Open virsh as superuser
    - Run `create path/to/xml --console`

  To shutdown the VM use `destroy` command. For whatever reason Genode does not
  reacts to `shutdown` command with any arguments.

- [x] Successfully Run the lwip_hello scenario.
  
  ISSUES:
    - platform-drv reports error not seen previously: 

```
[init -> drivers -> platform_drv] Error: ACPI report parsing error.
[init -> drivers -> platform_drv] Error: Uncaught exception of type 'int'
```

    - Host should have static IP, but it is not pingable, with ICMP and with 
    telnet. Seems like newly created interfaces does not attch to br0.
    
    **SOLVED: migrating VM to older x86_64 arch fixed the issues: Now server
    properly listens and the error is gone**
