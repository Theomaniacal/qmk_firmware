# Curie

Insert Photo Here

Insert Description Here


* Keyboard Maintainer: [Newman Liu](https://github.com/theomaniacal)
* Hardware Supported: Curie PCB for Curie Keyboard
* Hardware Availability: [thockpop.com](https://thockpop.com)

Getting the board into bootloader mode:

To flash firmware onto this board, you'll need to bring the PCB into bootloader mode. To enter bootloader mode, use tweezers or a paperclip to short the two reset pins. The reset pins are located next to the Caps Lock switch footprint. While the reset pins are shorted, connect the USB cable.

Make example for this keyboard (after setting up your build environment):

    make thockpop/curie:default

Flashing example for this keyboard:

    make thockpop/curie:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).