import serial as serial
import constants as CONSTANTS


#
# Initalize serial
#
def init():

  # Open the serial port for SMS and GPS devices
  serialPort = serial.Serial(
    port=CONSTANTS.SERIAL_DEVICE,
    baudrate=CONSTANTS.SERIAL_BAUD,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
  )
  return serialPort


