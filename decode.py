import struct
import csv
import io
BLOCK_SIZE = 18
filename = './LOG0.BIN'
with open("LOG0.csv",'w') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=['time','accx','accy','accz','gyrox','gyroy','gyroz','temp'])
    writer.writeheader()

    with open(filename, "rb") as file:
        while(1):
            data_block = file.read(512)
            if data_block == b'':
                break
            bt = io.BytesIO(data_block)
            count = struct.unpack('h',bt.read(2))
            if(count == 0):
                break
            overruns = struct.unpack('h',bt.read(2))
            for i in range(1,count[0]):
                data = struct.unpack('Ihhhhhhh',bt.read(18))
                print(data)
                writer.writerow({
                'time':data[0],
                'accx':data[1],
                'accy':data[2],
                'accz':data[3],
                'gyrox':data[4],
                'gyroy':data[5],
                'gyroz':data[6],
                'temp':data[7]})