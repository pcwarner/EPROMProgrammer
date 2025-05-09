## EPROMProgrammer

This branch contains the designs for two EPROM programmers.

Both programmers use an Arduino Nano as the controller. The PCBs were designed using DIP trace.

The first programmer works with the 27C16 and 27C256 UV light erasable EPROMs. The programming voltage can be set using the details from the UVPROMs data sheet. Normally this is between 12 and 25 voltages. Please care to ensure the correct voltage is used and it can result in damage to the EPROM. These chips are of a very old design and as a rule you would be better using new chips, which are cheaper, faster and hold more storage. This programmer requires an external PSU of around between 6-12v with a barrel connector. 

The second programmer works with the 28C16, 28C64, 28C256, 28C512, SST39SF010, SST39SF020 and the SST39SF040. These are Electricly Erasable PROMs. The power for this progammer is obtained from the Arduino through the USB and no external PSU is required.

The PCB where created using the free version of DipTrace, which is available at: https://diptrace.com/download/download-diptrace/. The free version limits you to 300 pins and two layers, which is fine for the PCBs in this project. There are also other limitation for free version please seen their license for full details.

There firmware on the Arduino Nanos was created using the Arduino IDE, which can be downloaded at: https://www.arduino.cc/en/software/.

The controlling software that run on the PC is written in Python and should work under most implementations of this language. I use Visual Studio Code and installed Phython using that IDE.

The Arduino firmware using the serial USB interface to talk between itself and the host computer. These commands apply to both the UVEPROM and the EEPROM programs, although not all commands or all options are available in both. The commands are:

**R - Read
R,address - returns G,data**
When given an address in hexadecimal the firmware returns the data helds in that location in the EPROM. The address should be given in hexadecimal and should not be greater size of the EPROM. 

**W - Write
W,address,data - returns G**
Writes the data to the given address within the EEPROM. For UVEPROMs the programming voltage is switch on. There is then a delay to given the voltage time to stabilize, before the actual write happens. After this the programming voltage is switch off. As a result single writes can be slow. EEPROMs do not have this issue. Please see the batch mode option to able the programming voltage, after which many writes can happen. 

**U - Update 
U,address,data - returns G,data**
Writes data to EEPROM and then reads the data. The data written should be same as the data return. As a general rule it is quicker to written all the data required and then to read it all back, as the update command needs to keep swiching between modes and this takes longer. This is especially true for UVEPROMS which require programming voltage to stabilize before the data can be written.

**S - Set
S,<EPROM type> - returns G**
Sets the type of EEPROM to be used. Please see list above for the EPROMs each device supports. Most commands require that the EPROM type is set before the command will work. The EPROM type is used to define the size, the pins and the features of the EPROM. For example the pover pin is in a different location between the 28C16 and the 28C256. Should the programmer be reset or power cycled then this type is not saved. Even disconnecting and reconnected will reset the device, requiring the EPROM type to be set again. A list of support types is shown when the help command is used.

** G - Get
G - returns G,<EPROM type>**
Gets the type of EEPROM being used or a error if the type has not been set. 

**P - Protect
P,<1 or 0> - returns G**
Set or unsets write protection on some EEPROM. "1" set the protection and "0" removes it. 28C256 and 28C512 can be protected from acidential changed by setting this feature. Command is not present on the UVEPROM programmer as such EPROM require a programming voltage to be changed. 

**E - Erase
E,<ALL or address> - returns G**
The flash EEPROMs can not be updated byte at a time, they must be erased first and then written it. This command accepts the "ALL" parameter to erase the whole EEPROM or an address to erase that sector. This command only works on flash EEPROMs.

**B - Batch
B,<option>{,<data>} - returns G{,<data>}**
At present the only options supported are "1" to switch the UVEPROM programming voltage on and "0" to switch it off. This works with UVEPROMs only. Other options are planned in the future.

**T - Test
T,<option> - returns G**
The command permitts a number of test functions. "0" - Switches the programming voltage off. "1" switches the programming voltage on, which can be used to set the voltage level. "2" - Put a high to all the data lines in turn, which could be used to light as set of LEDs. "3" - Put a high to the address levels in turn. "4" - Flashes the CE# line, "5" - Flashes the OE# line and "6" flashes the WE# line. Not all EPROMs support all features.

**H - Help
H - returns <Help Message>**
This command displays the help message including the supported EPROM types.

**TODO**
1. Some EEPROMs support a "feed-back" method which lets the programmer know when the write has finished. This is not used and at present the firmware performs a delay that this the maximum length that the EEPROM is likely to need. This is clearly a slower and could be improved upon.
2. The UVPROM program has pins added that can be connected to a breadboard to support other UVEPROMs, however the firmware has not been updated to support any such additional UVEPROMs.