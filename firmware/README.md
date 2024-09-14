# Modbule firmware
You need arm-none-eabi-gcc toolchain version 12.3.1 or similar.

To build everything do:

```
cd mblboot
make
```
The resulting file mblboot.elf should be flashed into the MCU by st-link, black magic probe or whatever SWD adapter you do prefer.

Since the first upload the firmware should have be possible to update via the modbus protocol over the main RS-485 communication interface.

The particular binaries are (to be built) in firmware/modbule directory.
It is been built automaticaly as dependency of the boot program by the recipe above.


You can find the remote upload and configuration tools in its directories nearby.

## The architecture
### mblboot
It's very small and very limited program which sits at the start of the flash memory.
It isn't a boot loader it can't receive a updated firmware by any means.
Everything it does is checking CRC checksums of the two application programs and selects which one has to be run.
If built with the default procedure then the resulting binary includes two copies of the application like this
```
Name             Origin             Length             Attributes
RAM              0x0000000020000000 0x0000000000000100 xrw
FLASH            0x0000000008000000 0x0000000000000800 xr
FLASH_APA        0x0000000008000800 0x0000000000007800 xr
FLASH_APB        0x0000000008008000 0x0000000000007800 xr
UFLASH           0x000000000800f800 0x0000000000000800 rw
```
### modbule
Next in the flash memory there are two copies of the main application.

FLASH corresponds to the FLASH_APA from above
```
Name             Origin             Length             Attributes
RAM              0x0000000020000100 0x0000000000001f00 rw
FLASH            0x0000000008000800 0x0000000000007800 xr
UFLASH           0x000000000800f800 0x0000000000000800 rw
```
FLASH corresponds to the FLASH_APB from above
```
Name             Origin             Length             Attributes
RAM              0x0000000020000100 0x0000000000001f00 rw
FLASH            0x0000000008008000 0x0000000000007800 xr
UFLASH           0x000000000800f800 0x0000000000000800 rw

```
The UFLASH section is common for all the three programs.

By default they're practicaly the same with the exception that they are linked to run at different addresses.
As they get later updated, their versions and thus functionality can differ.

Any of the two applications can receive and update its counterpart in flash.

The aplications can also receive a user defined code to be run in ram after succesful integrity check. This is prepared as a way to do any rescue and repair operations not known to the author at the time of writing these lines.

#### Configuration
At the end of the flash memory, there is a data block holding a configuration.
```
UFLASH           0x000000000800f800 0x0000000000000800 rw
```
The configuration is used both by the boot program and the two applications.
It is only read by the boot program, but the applications can also modify it.

The configuration can be modified without erasing the flash thanks to primitive wear leveling structure. The old records are discarded in place and the new ones are appended after the old ones. The flash is erased only if there is no space left for the newest configuration received.
