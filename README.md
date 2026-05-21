# Booting FreeBSD on POWER8/POWER9

This is a simple program to deal with a regression in the last version
of the POWER8/POWER9 OpenPower (kexec-lite).

## What happened?

There was a regression in kexec-lite . The fix is here at
[https://github.com/antonblanchard/kexec-lite/pull/9](https://github.com/antonblanchard/kexec-lite/pull/9) .
However, the fix never made it into a subsequent boot firmware release.
This means that the latest boot firmware has a regression that prevents
FreeBSD from booting.

## How do I know if I need this?

If you boot a 15.0 or 16.x snapshot image and it reboots after kexec
rather than starting the kernel, chances are you need this.

(Yes, TODO item here is to get the known good and known not good firmware
versions; I'll do that when I reboot a broken firmware box.)

## What's the cause?

The diff explains it all - the entry point math is just plain wrong,
and it works on Linux because of their choice of entry address versus
virtual address.

A normal ppc64 kernel (May 2026) via elfdump -a /boot/kernel/kernel :

```
elf header:

        e_ident: ELFCLASS64 ELFDATA2LSB ELFOSABI_FREEBSD
...
        e_entry: 0x101540
...

program header:

entry: 0
        p_type: PT_LOAD
        p_offset: 65536
        p_vaddr: 0x100000
        p_paddr: 0x100000
```

## What's the fix?

The fix is to offset the entry address by the virtual address.
It's dirty, but it results in a booting kernel.

## Should I run this regardless of POWER8/POWER9 ?

No! Only run this for kernels that don't boot on the last firmware
version on the OpenPower POWER8/POWER9 boxes.  The earlier firmware
works fine and the Raptor Engineering POWER9 boot firmware works
fine.  If you run this tool to adjust the kernel entry point address
on working firmware it will /also stop working/ . You've been warned!

## How do you use this tool?

The tool expects to be built on little endian machine and be fed a little
endian powerpc64 kernel to fix.  Sorry I haven't yet fixed it to be
endian agnostic / configurable.

Building:

```
cc main.c -o main
```

Running:

```
# cp /boot/kernel/kernel .
# chmod 644 kernel
# ./main ./kernel
# cp -f ./kernel /boot/kernel/kernel
# chmod 444 /boot/kernel/kernel
```

Note that the kernel image is by default read only and the tool will
not write to the kernel if it's read only.

A 'fixed' kernel image will have a modified entry point but the same
vaddr/paddr in the PT_LOAD header.

```
elf header:
...
        e_entry: 0x1540
...

program header:

entry: 0
        p_type: PT_LOAD
        p_offset: 65536
        p_vaddr: 0x100000
        p_paddr: 0x100000

```

