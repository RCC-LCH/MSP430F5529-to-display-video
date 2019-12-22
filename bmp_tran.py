import os
from PIL import Image
import serial           #导入模块
import binascii
portx = "COM28"
bps = 184985
                    #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
timex = None
ser = serial.Serial(portx, bps, timeout=timex)
print("串口详情参数：", ser)

file_addr='C:/Users/user/Desktop/BIT_MAP'
files = os.listdir(file_addr)
#定义格式转换函数
def hex_chenge(str):
    if(len(str)%2==0):
        return str[2:]
    else:
        return '0'+str[2:]


with open('data.txt', 'w') as f:
    for nmu in range(1,1855):
        file_name = file_addr + '/'+str(nmu) +'.bmp'
        img = Image.open(file_name)
        data = img.load()
        bit_map = []
        print(img.size)
        print(nmu)
        for y in range(0, 64, 8):
            for x in range(0, 128):
                for i in range(7, -1, -1):
                    if data[x, y+i] == 0:
                        bit_map.append(0)
                    else:
                        bit_map.append(1)
        list_num = []
        for l in range(0, 8192, 8):
            binary = '0b'+str(bit_map[l])+str(bit_map[l+1])+str(bit_map[l+2])+str(bit_map[l+3])+str(bit_map[l+4])\
                    + str(bit_map[l+5])+str(bit_map[l+6])+str(bit_map[l+7])
            hexnumber = hex(int(binary, 2))
            list_num.append(hexnumber)
        for l in range(0, 1024):
            f.writelines(list_num[l] + ',' + ' ')
            ser.write(binascii.a2b_hex(hex_chenge(list_num[l])))  # Hex发送
        buff = 0
        while buff == 0:
            buff = ser.read().hex()
        print(buff)
