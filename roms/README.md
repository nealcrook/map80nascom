The roms folder is used to keep all the Z80 code needed to run the emulator.

These .nas and .rom files are Nascom ascii format files.

When loading .rom files they are loaded into "ROM" space and therefore will not be paged out or written to. If you want them loaded into "RAM" then change their names to .nas.

There are 2 files that are automatically loaded into their own "ROM" space at start up.

* **nassys3.nas** NASSYS3.  Extracted from my system. 

    This is loaded into address 0 if the -b options is not used.

* **vfcrom0.nas** The boot rom for the vfc card.

    The VFC vfcrom0.nas was extracted from my system.
    This is loaded into the MAP80 VFC card rom area and activated if the -b option is used.
    
The are optional files that can be specified on the command line to load.

* **basic.rom** Nascom2 Basic. Thanks to http://www.nascomhomepage.com/ 

* **PolyDos_2_Boot_ROM.rom** The boot rom for PolyDos 2.
    Thanks to Neal Crook - https://github.com/nealcrook/nascom/tree/master/PolyDos/rom



