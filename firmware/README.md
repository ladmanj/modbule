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
is very small and very limited program which sits at the start of the flash memory.
It isn't a boot loader it can't receive a updated firmware by any means.
Everything it does is checking CRC checksums of the two application programs and selects which one has to be run.

If built with the default procedure then the resulting binary includes two copies of the application described next.

### modbule
Next in the flash memory there are two copies of the main application

By default they're practicaly the same with the exception that they are linked to run at different addresses.
As they get later updated, their versions and thus functionality can differ.

Any of the two applications can receive and update its counterpart in flash.

The aplications can also receive a user defined code to be run in ram after succesful integrity check. This is prepared as a way to do any rescue and repair operations not known to the author at the time of writing these lines.

#### Configuration
At the end of the flash memory, there is a data block holding a configuration.

The configuration is used both by the boot program and the two applications.

The configuration can be modified without erasing the flash thanks to primitive wear leveling structure. The old records are discarded in place and the new ones are appended after the old ones. The flash is erased only if there is no space left for the newest configuration received.
