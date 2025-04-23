This branch contains the designs for two EPROM programmers.

Both programmers use an Arduino Nano as the controller. The PCBs were designed using DIP trace.

The first programmer works with the 27C16 (2K) and 27C256 (32k) UV light erasable EPROMs. The programming voltage can be set using the details from the UVPROMs data sheet. Normally this is between 12 and 25 voltages. Please care to ensure the correct voltage is used and it can result damage to the EPROM. These chips are of a very old design and as a rule you would be better using new chips, which are cheaper, faster and hold more storage. This programmer requires an external PSU of around 12v with a barrel connector. 

The second programmer works with the 28C16, 28C64, 28C256, 28C512, SST39SF010, SST39SF020 and the SST39SF040. These are Electricly Erasable PROMs. The power for this progammer is obtained from the Arduino through the USB. 
