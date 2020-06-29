import serial, time

serialPort = serial.Serial(port="/dev/ttyACM0", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

serialString = ""

# Open the srial port ttyACM0
serialPort.close()

# Send state command to the master node
serialPort.write(b"benchmark_start 1000")
serialPort.write(b"\r\n")

while(1):
    # Wait until there is data waiting in the serial buffer
    if(serialPort.in_waiting > 0):

        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()

        # Print the contents of the serial data
        print(serialString.decode('Ascii'))

        break