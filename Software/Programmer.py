import time
import sys
import serial
import os.path
import re
import random

# Define the action types
ACTION_READ = 1
ACTION_WRITE = 2
ACTION_VERIFY = 3
ACTION_READBYTE = 4
ACTION_WRITEBYTE = 5
ACTION_FILL = 6
ACTION_PROTECT = 7

# Define the EPROMS we can use
EPROM_NAMES = [ "28C16", "28C64", "28C256", "28C512", "27C16", "27C256" ]
EPROM_SIZES = [    2048,    8192,    32768,    65536,    2048,    32768 ]

# Other constants
NEWLINE = 10
TIMEOUT = 5000
END_GOOD = 0
END_FAIL = -1
HASH_DISPLAY = 64 # How often to display a hash
EOL_DISPLAY = 64  # How many hashes on a line

def isHex(s):
    return re.fullmatch(r"^[0-9a-fA-F]+$", s or "") is not None

def getTime():
    return round(time.time() * 1000)

# Read a line from the serial port
def readFromSerial(serialPort):
  response = []
  lastTime = getTime()
  finished = False
  while (getTime() - lastTime) < TIMEOUT and not finished:
    line = ""
    while serialPort.inWaiting() > 0:
      x = serialPort.readline()
      if len(x) > 0:
        for c in x:
          if c > 31 and c < 126:
            line += chr(c)
          elif c == NEWLINE:
            finished = True

    if len(line) > 0:
      response.append(line)

  return str(response[0])

# Get a serial port object we can used
def getSerialPort(port):
  # Set up the serial port
  try:
    serialPort = serial.Serial(port)
  except serial.SerialException as e:
    print("ERROR: Cannot open the serial port")
    exit(END_FAIL)
  serialPort.baudrate = 115200 
  serialPort.bytesize = 8 
  serialPort.parity = 'N'
  serialPort.stopbits = 1
  time.sleep(3)
  # Wait for the starting message
  readFromSerial(serialPort)
  return serialPort

# Get the size of the EPROM from it's type
def getEpromSize(epromType):
  for i in range(0, len(EPROM_NAMES)):
    if epromType == EPROM_NAMES[i]:
      return EPROM_SIZES[i]
  return -1

def loadFile(epromType, fileName):
  epromSize = getEpromSize(epromType)
  fileContents = []
  dataSize = 0
  with open(fileName, "r") as file:
    for line in file:
        value = line.rstrip()
        if re.fullmatch(r"^[0][x][0-9A-Fa-f][0-9A-Fa-f]*$", value) or re.fullmatch(r"^[0-9A-Fa-f][0-9A-Fa-f]*$", value):
          dataSize = dataSize + 1
          if dataSize > epromSize:
            print("ERROR: The data in the file is greater than the EPROM size")
            exit(END_FAIL)
          data = int(value, 16)
          if data < 0 or data > 255:
            print("ERROR: The data value must be between 0x0 and 0xff (0 to 255 decimal)")
            exit(END_FAIL)
          fileContents.append(data)
  if dataSize < 1:
    print("ERROR: The input file contains no data")
    exit(END_FAIL)

  return fileContents

# Tell the programmer which EPROM we will be using
def setEpromToUse(serialPort, epromType):
  serialPort.write(b"S," + bytes(epromType, "utf8") + b"\n")
  response = readFromSerial(serialPort)
  if response.startswith("E"):
    print(response)
    exit(END_FAIL)

# Read a byte from the EPROM
def readByteFromEprom(serialPort, address):
  serialPort.write(b"R," + bytes(format(address, "x"), "utf8") + b"\n")
  response = readFromSerial(serialPort)
  if response.startswith("E"):
    print(response)
    exit(END_FAIL)

  return int(response[2:], 16)

# Write a byte to the EPROM
def writeByteToEprom(serialPort, address, data):
  serialPort.write(b"W," + bytes(format(address, "x") + "," + format(data, "x"), "utf8") + b"\n")
  response = readFromSerial(serialPort)
  if response.startswith("E"):
    print(response)
    exit(END_FAIL)

# Read the contents of the EPROM and write it to a file
def read(epromType, port, fileName):
  epromSize = getEpromSize(epromType)
  fileContents = [0] * epromSize
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
 
  j = 0
  for i in range(0, epromSize):
    if (i % HASH_DISPLAY) == 0:
      print("#", end="")
      j = j + 1
      if j >= EOL_DISPLAY:
        print("")
        j = 0
  
    fileContents[i] = readByteFromEprom(serialPort, i)

  print("")
  print(str(epromSize) + " bytes read")  
  serialPort.close()

  with open(fileName, "w") as file:
    file.write("v2.0 raw\n")
    for i in range(0, epromSize):
      file.write(hex(fileContents[i]) + "\n")

# Write the contents of the file to the EPROM
def write(epromType, port, fileName):
  fileContents = loadFile(epromType, fileName)
  dataSize = len(fileContents)
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
  
  j = 0
  for i in range(0, dataSize):
    if (i % HASH_DISPLAY) == 0:
      print("#", end="")
      j = j + 1
      if j >= EOL_DISPLAY:
        print("")
        j = 0

    writeByteToEprom(serialPort, i, fileContents[i])

  print("")
  print(str(dataSize) + " bytes written")
  serialPort.close()

# Verify the contents of the EPROM against the file
def verify(epromType, port, fileName):
  fileContents = loadFile(epromType, fileName)
  dataSize = len(fileContents)

  serialPort = getSerialPort(port)

  setEpromToUse(serialPort, epromType)
  
  j = 0
  for i in range(0, dataSize):
    if (i % HASH_DISPLAY) == 0:
      print("#", end="")
      j = j + 1
      if j >= EOL_DISPLAY:
        print("")
        j = 0
    
    data = readByteFromEprom(serialPort, i)
    if data != fileContents[i]:
      print("")
      print("ERROR: The data in the EPROM (" + hex(data) + ") does not match the data in the file (" + hex(fileContents[i]) + ") at address " + hex(i))
      exit(END_FAIL)

  print("")
  print(str(dataSize) + " bytes verified")
  serialPort.close()

# Fill EPROM with random data and write the same to the file
def fill(epromType, port, fileName):
  epromSize = getEpromSize(epromType)
  fileContents = [0] * epromSize
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
 
  j = 0
  for i in range(0, epromSize):
    if (i % HASH_DISPLAY) == 0:
      print("#", end="")
      j = j + 1
      if j >= EOL_DISPLAY:
        print("")
        j = 0

    fileContents[i] = random.randint(0, 254)
    writeByteToEprom(serialPort, i, fileContents[i])

  with open(fileName, "w") as file:
    file.write("v2.0 raw\n")
    for i in range(0, epromSize):
      file.write(hex(fileContents[i]) + "\n")

  print("")
  print(str(epromSize) + " bytes written")
  serialPort.close()  

# Read a single byte from the EPROM
def readByte(epromType, port, address):
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
  data = readByteFromEprom(serialPort, address)
  serialPort.close()
  print(hex(data))

# Write a single byte to the EPROM
def writeByte(epromType, port, address, data):
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
  writeByteToEprom(serialPort, address, data)
  serialPort.close()

# Protect or Unprotect the EPROM from chnages
def protect(epromType, port, protectionMode):
  serialPort = getSerialPort(port)
  setEpromToUse(serialPort, epromType)
  if protectionMode:
    serialPort.write(b"P,1\n")
  else:
    serialPort.write(b"P,0\n")
  
  response = readFromSerial(serialPort)
  if response.startswith("E"):
    print(response)
    exit(END_FAIL)
 
  if protectionMode:
    print("Protection set on EPROM")
  else:
    print("Protection removed from EPROM")

# The main methos
if __name__ == "__main__":

  print("")

  # The first parameter is the EPROM type
  i = 1
  epromType = sys.argv[i].upper()
  if getEpromSize(epromType) < 0:
    print("ERROR: The EPROM type is not one that is supported")
    exit(END_FAIL)

  # The second parameter is the port
  i = i + 1
  port = sys.argv[i].upper()
  if not port.startswith("COM"):
    print("ERROR: The COM port should be COM followed by a number (E.g. COM4)")
    exit(END_FAIL)

  # The third parameter is the action
  i = i + 1
  actionString = sys.argv[i].upper()
  match actionString:
    case "READ":
      action = ACTION_READ
    case "WRITE":
      action = ACTION_WRITE
    case "VERIFY":
      action = ACTION_VERIFY
    case "READBYTE":
      action = ACTION_READBYTE
    case "WRITEBYTE":
      action = ACTION_WRITEBYTE
    case "FILL":
      action = ACTION_FILL
    case "PROTECT":
      action = ACTION_PROTECT
    case _:
      print("ERROR: Action is not a valid type")
      exit(END_FAIL)

  # WRITE, READ, VERIFY and FILL should be followed by a file name
  fileName = ""
  if action == ACTION_READ or action == ACTION_WRITE or action == ACTION_VERIFY or action == ACTION_FILL:
    i = i + 1
    fileName = sys.argv[i]
    if fileName == "":
      print("ERROR: file name is missing")
      exit(END_FAIL)
    elif (action == ACTION_READ or action == ACTION_FILL) and os.path.isfile(fileName):
      print("ERROR: A file with the name " + fileName + " already exists")
      exit(END_FAIL)
    elif (action == ACTION_WRITE or action == ACTION_VERIFY) and not os.path.isfile(fileName):
      print("ERROR: There is no file with the name " + fileName)
      exit(END_FAIL)

  # READBYTE and WRITEBYTE need to be followed by an address
  address = 0
  if action == ACTION_READBYTE or action == ACTION_WRITEBYTE:
    i = i + 1; 
    addressString = sys.argv[i]
    if addressString.upper().startswith("0X"):
      addressString = addressString[2:]
    if not isHex(addressString):
      print("ERROR: The address is not a valid Hex value")
      exit(END_FAIL)
    address = int(addressString, 16)
    if address < 0 or address > getEpromSize(epromType):
      print("ERROR: The address must be between 0x0 and " + hex(getEpromSize(epromType)) + " (Which is 0 and " + getEpromSize(epromType) + " in decimal)")
      exit(END_FAIL)

  # WRITEBYTE is then followed by a data value
  data = 0
  if action == ACTION_WRITEBYTE:
    i = i + 1; 
    dataString = sys.argv[i]
    if dataString.upper().startswith("0X"):
      dataString = dataString[2:]
    if not isHex(dataString):
      print("ERROR: The data is not a valid Hex value")
      exit(END_FAIL)
    data = int(dataString, 16)
    if data < 0 or data > 255:
      print("ERROR: The data must be between 0x0 and 0xFF (Which is 0 and 255 in decimal)")
      exit(END_FAIL)

  # PROTECT is then followed by either a "0" for unprotect or "1" to protect
  protectionMode = False
  if action == ACTION_PROTECT:
    i = i + 1
    if sys.argv[i] == "0":
      protectionMode = False
    elif sys.argv[i] == "1":
      protectionMode = True
    else:
      print("ERROR: The protection mode must be either 0 for unprotect or 1 for protect.")
      exit(END_FAIL)

  # Take the action requird
  if action == ACTION_READ:
    read(epromType, port, fileName)
  elif action == ACTION_WRITE:
    write(epromType, port, fileName)
  elif action == ACTION_VERIFY:
    verify(epromType, port, fileName)
  elif action ==  ACTION_READBYTE:
    readByte(epromType, port, address)
  elif action == ACTION_WRITEBYTE:
    writeByte(epromType, port, address, data)
  elif action == ACTION_FILL:
    fill(epromType, port, fileName)
  elif action == ACTION_PROTECT:
    protect(epromType, port, protectionMode)

  exit(END_GOOD)
