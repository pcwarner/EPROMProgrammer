# EPROMProgrammer

This branch contains the designs for two EPROM programmers.

Both programmers use an Arduino Nano as the controller. The PCBs were designed using DIP trace.

The first programmer works with the 27C16 and 27C256 UV light erasable EPROMs. The programming voltage can be set using the details from the UVPROMs data sheet. Normally this is between 12 and 25 voltages. Please care to ensure the correct voltage is used, An incorrect voltage can result in damage to the EPROM. UVEPROMs are of a very old design and as a rule you would be better using new chips, which are cheaper, faster and hold more storage. This programmer requires an external PSU of around between 6-12v with a barrel connector. The programming voltagew is created using a on board boost convertor. Please use the following link to a YouTube video showing this device: https://youtu.be/ZAfaKeMcMLA?si=_kUlqI8FRL1CYst8

The second programmer works with the 28C16, 28C64, 28C256, 28C512, SST39SF010, SST39SF020 and the SST39SF040. These are Electricly Erasable PROMs. The power for this progammer is obtained from the Arduino through the USB and no external PSU is required. The YouTube video this device is: https://youtu.be/WWmPPYQPKCU?si=6Kh_ERBs9z3F51hZ

The PCB where created using the free version of DipTrace, which is available at: https://diptrace.com/download/download-diptrace/. The free version limits you to 300 pins and two layers, which is fine for the PCBs in this project. There are also other limitation for free version please seen their license for full details.

There firmware on the Arduino Nanos was created using the Arduino IDE, which can be downloaded at: https://www.arduino.cc/en/software/.

The controlling software that run on the PC is written in Python and should work under most implementations of this language. I use Visual Studio Code and installed Phython using that IDE.

# Firmware Interface

The Arduino firmware using the serial USB interface to talk between itself and the host computer. These commands apply to both the UVEPROM and the EEPROM programs, although not all commands or all options are available in both. All commands return "G", which may be followed by other parameters. If there is a error then the firmware will return "E" followed by a text description of the error.

The commands are:

## R - Read

**R,\<address\> - returns G,\<data\>**

When given an address in hexadecimal the firmware returns the data helds in that location in the EPROM. The address should be given in hexadecimal and should not be greater size of the EPROM. 

## W - Write

**W,\<address\>,\<data\> - returns G**

Writes the data to the given address within the EEPROM. For UVEPROMs the programming voltage is switch on. There is then a delay to given the voltage time to stabilize, before the actual write happens. After this the programming voltage is switch off. As a result single writes can be slow. EEPROMs do not have this issue. Please see the batch mode option to able the programming voltage, after which many writes can happen. 

## U - Update

**U,\<address\>,\<data\> - returns G,\<data\>**

Writes data to EEPROM and then reads the data. The data written should be same as the data return. As a general rule it is quicker to written all the data required and then to read it all back, as the update command needs to keep swiching between modes and this takes longer. This is especially true for UVEPROMS which require the programming voltage to stabilize before the data can be written.

## S - Set

**S,\<EPROM type\> - returns G**

Sets the type of EEPROM to be used. Please see list above for the EPROMs each device supports. Most commands require that the EPROM type is set before the command will work. The EPROM type is used to define the size, the pins and the features of the EPROM. For example the power pin is in a different location between the 28C16 and the 28C256. Should the programmer be reset or power cycled then this type is not saved. Even disconnecting and reconnected will reset the device, requiring the EPROM type to be set again. A list of support types is shown when the help command is used.

## G - Get

**G - returns G,\<EPROM type\>**

Gets the type of EEPROM being used or returns an error if the type has not been set. 

## P - Protect

**P,\<1 or 0\> - returns G**

Set or unsets write protection on some EEPROMs. "1" set the protection and "0" removes it. For example 28C256 and 28C512 support this feature that protects against accidentially  changing the contents. This command is not present on the UVEPROM programmer as such EPROM require a programming voltage to be changed. 

## E - Erase

**E,\<ALL or Address\> - returns G**

Flash EEPROMs can not be reprogrammed directly, instead they must first be erased first and then written to. This command accepts the "ALL" parameter to erase the whole EEPROM or an address of a sector to erase. This command only works on flash EEPROMs. The sector size depends on the EEPROM. For example the SST39SF0x0 devices supported by this EEPROM programmer used a 4K sector size. 

## B - Batch

**B,\<option\>{,\<data\>} - returns G{,\<data\>}**

At present the only options supported are "1" to switch on the UVEPROM programming voltage on and "0" to switch it off. This option only works with UVEPROMs only. Other options are planned in the future.

## T - Test

**T,\<option\> - returns G**

The command permitts a number of test functions.

 - "0" - Switches the programming voltage off.
 - "1" - Switches the programming voltage on, which can then be used to set the voltage level. 
 - "2" - Put a high to all the data lines in turn, which could be used to light a set of LEDs. 
 - "3" - Put a high to the address levels in turn, which again could light LEDs. 
 - "4" - Flashes the CE# line.
 - "5" - Flashes the OE# line.
 - "6" - Flashes the WE# line.

 Not all EPROMs support all features.

## H - Help

**H - returns \<Help Message\>**
This command displays the help message including a list of the supported EPROM types.

# Software

Also included with this project is a controlling program that is written in Python. It uses a command line interface. 

## READ

**python programmer READ \<Serial Port\> \<EPROM Type\> \<File Name{.hex}\>**

The READ command reads EPROM and writes it to a file. The file should have the suffix of ".hex". The "Serial Port" parameter should be the port the programmer is connected to. For example: "COM6". The EPROM type should be a valid EPROM type code that the programmer supports. And the file name should be a valid file that does not exist. The resulting file will been the size about six times the size of the EPROM being read and contains one line per byte with the value in hexadecimal. 

## WRITE

**python programmer WRITE \<Serial Port\> \<EPROM Type\> \<File Name{.hex}\>**

The write command takes the contents of a hexadecimal file and write it to the EPROM. If the file is smaller than the size the EPROM can take then only the first part of the EPROM is used. If the file is larger than the EPROM then an error will be reported.

## VERIFY

**python programmer VERIFY \<Serial Port\> \<EPROM Type\> \<File Name{.hex}\>**

The verify command read the contents of the EPROM and compares the data to that in the EPROM, reporting any errors.

## READBYTE

**python programmer READBYTE \<Serial Port\> \<EPROM Type\> \<Address\>**

This command reads a byte from the EPROM and displays it.

## READBYTE

**python programmer WRITEBYTE \<Serial Port\> \<EPROM Type\> \<Address\> \<Data\>**

This command writes a single byte to the EPROM. Not really that useful, but is included to be complete.

## PROTECT

**python programmer PROTECT \<Serial Port\> \<EPROM Type\> \<1 or 0\>**

Switches the protection mode on or off. This command requires that the EPROM supports the function. If it doesn't then a error will be reported.

## FILL

**python programmer FILL \<Serial Port\> \<EPROM Type\> \<File Name{.hex}\>**

Fills the EPROM with randon data and alo writes the data to a file. This command was really only added so I could test the programmers, however it has been left in as it could be used to security wipe an EPROM. Writing zeros to the EEPROM, does always clear the contents, whereas performing a few fills is more secure. 
 

# TODO

1. Some EEPROMs support a "feed-back" method which lets the programmer know when the write has finished. At present this feature is not used and the firmware simply performs a delay which is likely to be greater than the maximum the EEPROM  will need. This is clearly a slower method and could be improved upon. When written the "feed-back" method will be on by default for the EPROMS that support it, however it will be possible to switch back to the "delay" method using a "B" (Batch) command.
2. The UVEPROM programmer has pins added that can be connected to a breadboard in order to support other UVEPROMs, however at present the firmware has not been updated to support any such additional UVEPROMs.
3. In a similar way the EEPROM programmer at present doesn't have such pins and it would be useful to add these.
