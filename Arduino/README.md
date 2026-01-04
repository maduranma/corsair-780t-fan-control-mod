# Arduino code
So this is the Arduino part of the project, I've used an Arduino Pro Micro with USB-C.

## boards.txt
Insert the following on your `boards.txt` file (use the VID/PID that you want).

The VID/PID used by default is FFFF/0001, as FFFF is abandoned as a vendor.

In the future I plan to ask for a free VID/PID from [pid.codes](https://pid.codes) if possible, as this is open source.

```txt
corsairfanmod.name=Corsair Fan Controller Mod
corsairfanmod.vid.0=0xFFFF
corsairfanmod.pid.0=0x0001
corsairfanmod.vid.1=0xFFFF
corsairfanmod.pid.1=0x0001
corsairfanmod.vid.2=0xFFFF
corsairfanmod.pid.2=0x0001
corsairfanmod.vid.3=0xFFFF
corsairfanmod.pid.3=0x0001
corsairfanmod.upload_port.0.vid=0xFFFF
corsairfanmod.upload_port.0.pid=0x0001
corsairfanmod.upload_port.1.vid=0xFFFF
corsairfanmod.upload_port.1.pid=0x0001
corsairfanmod.upload_port.2.vid=0xFFFF
corsairfanmod.upload_port.2.pid=0x0001
corsairfanmod.upload_port.3.vid=0xFFFF
corsairfanmod.upload_port.3.pid=0x0001
corsairfanmod.upload_port.4.board=leonardo

corsairfanmod.upload.tool=avrdude
corsairfanmod.upload.tool.default=avrdude
corsairfanmod.upload.tool.network=arduino_ota
corsairfanmod.upload.protocol=avr109
corsairfanmod.upload.maximum_size=28672
corsairfanmod.upload.maximum_data_size=2560
corsairfanmod.upload.speed=57600
corsairfanmod.upload.disable_flushing=true
corsairfanmod.upload.use_1200bps_touch=true
corsairfanmod.upload.wait_for_upload_port=true

corsairfanmod.bootloader.tool=avrdude
corsairfanmod.bootloader.tool.default=avrdude
corsairfanmod.bootloader.low_fuses=0xff
corsairfanmod.bootloader.high_fuses=0xd8
corsairfanmod.bootloader.extended_fuses=0xcb
corsairfanmod.bootloader.file=caterina/Caterina-Leonardo.hex
corsairfanmod.bootloader.unlock_bits=0x3F
corsairfanmod.bootloader.lock_bits=0x2F

corsairfanmod.build.mcu=atmega32u4
corsairfanmod.build.f_cpu=16000000L
corsairfanmod.build.vid=0xFFFF
corsairfanmod.build.pid=0x0001
corsairfanmod.build.usb_product="Corsair Fan Controller Mod"
corsairfanmod.build.board=AVR_LEONARDO
corsairfanmod.build.core=arduino
corsairfanmod.build.variant=leonardo
corsairfanmod.build.extra_flags={build.usb_flags}
```