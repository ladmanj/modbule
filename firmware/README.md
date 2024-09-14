
# Modbule Firmware

You need the `arm-none-eabi-gcc` toolchain version 12.3.1 or similar.

To build everything, do:

```bash
cd mblboot
make
```

The resulting file, `mblboot.elf`, should be flashed into the MCU using ST-Link, Black Magic Probe, or any other SWD adapter you prefer.

After the initial upload, the firmware can be updated via the Modbus protocol over the main RS-485 communication interface.

The binaries to be built will be located in the `firmware/modbule` directory. They are built automatically as a dependency of the boot program using the recipe above.

You can find the remote upload and configuration tools in the nearby directories.

## The Architecture

### mblboot

This is a very small and limited program that sits at the start of the flash memory. It isn't a bootloader since it can't receive updated firmware through any means. Its only function is to check the CRC checksums of the two application programs and select which one to run.

If built with the default procedure, the resulting binary includes two copies of the application, arranged like this:

| Name       | Origin             | Length            | Attributes |
|------------|--------------------|-------------------|------------|
| RAM        | 0x0000000020000000 | 0x0000000000000100 | xrw        |
| FLASH      | 0x0000000008000000 | 0x0000000000000800 | xr         |
| FLASH_APA<kbd>[^1]</kbd>  | 0x0000000008000800 | 0x0000000000007800 | xr         |
| FLASH_APB<kbd>[^2]</kbd>  | 0x0000000008008000 | 0x0000000000007800 | xr         |
| UFLASH<kbd>[^3]</kbd>     | 0x000000000800f800 | 0x0000000000000800 | rw         |

### modbule

Next in the flash memory are two copies of the main application.



| Name   | Origin             | Length            | Attributes |
|--------|--------------------|-------------------|------------|
| RAM    | 0x0000000020000100 | 0x0000000000001f00 | rw         |
| FLASH<kbd>[^1]</kbd>  | 0x0000000008000800 | 0x0000000000007800 | xr         |
| UFLASH<kbd>[^3]</kbd> | 0x000000000800f800 | 0x0000000000000800 | rw         |


| Name   | Origin             | Length            | Attributes |
|--------|--------------------|-------------------|------------|
| RAM    | 0x0000000020000100 | 0x0000000000001f00 | rw         |
| FLASH<kbd>[^2]</kbd>  | 0x0000000008008000 | 0x0000000000007800 | xr         |
| UFLASH<kbd>[^3]</kbd> | 0x000000000800f800 | 0x0000000000000800 | rw         |

[^1]: The `FLASH` corresponds to `FLASH_APA`.
[^2]: The `FLASH` corresponds to `FLASH_APB`.
[^3]: The `UFLASH` section is common to all three programs.

By default, they are practically the same, except that they are linked to run at different addresses. As they are later updated, their versions and functionality can differ.

Either of the two applications can receive and update its counterpart in flash.

The applications can also receive user-defined code to be run in RAM after a successful integrity check. This is provided as a way to perform any rescue and repair operations not known to the author at the time of writing.

#### Configuration

At the end of the flash memory, there is a data block holding a configuration:

| Name   | Origin             | Length            | Attributes |
|--------|--------------------|-------------------|------------|
| UFLASH | 0x000000000800f800 | 0x0000000000000800 | rw         |

The configuration is used by both the boot program and the two applications. It is only read by the boot program, but the applications can also modify it.

The configuration can be modified without erasing the flash, thanks to a primitive wear-leveling structure. The old records are discarded in place, and the new ones are appended after the old ones. The flash section is erased only if there is no space left for the newest configuration received.
