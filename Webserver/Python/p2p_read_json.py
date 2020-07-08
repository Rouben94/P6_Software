import serial, multiprocessing, time, re, json

serialPort = serial.Serial(port="COM9", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
serialString = ""

bm_message_list = []
bm_time_list = []

def bm_start(time):
    serial_command = "benchmark_start {}\n"
    serialPort.write(bytes(serial_command.format(time).encode('Ascii')))

bm_time = float(input("Benchmark Zeit:"))
close_time = time.time() + bm_time

while(1):
    close_time2 = time.time() + 12.0
    bm_start(10000)

    while(1):
        # Wait until there is data waiting in the serial buffer
        if(serialPort.in_waiting > 0):
            # Read data out of the buffer until a carraige return / new line is found
            serialString = serialPort.readline().decode('Ascii')

            # Print the contents of the serial data
            if "ID" in serialString:
                result = re.findall(r"\d+", serialString)
                if result[0] in bm_message_list:
                    bm_time_list.append(abs(int(result[1])-int(bm_message_list[bm_message_list.index(result[0])+1])))
                else:
                    bm_message_list.append(result[0])
                    bm_message_list.append(result[1])
        
        if time.time()>close_time2:
            bm_message_list.clear()
            print(bm_time_list)
            break
    
    if time.time()>close_time:
        bm_time_list.clear
        break