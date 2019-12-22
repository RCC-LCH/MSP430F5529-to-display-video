# 图片二值化
import os
from PIL import Image
file_addr='C:/Users/user/Desktop/.BMP'
file_sto='C:/Users/user/Desktop/BIT_MAP'
files = os.listdir(file_addr)
cnt=0
for name in files :
    cnt=cnt+1
    print(name)
    file_name=file_addr + '/'+name
    file_sto_name=file_sto + '/' + str(cnt) +'.bmp'
    img = Image.open(file_name)


    Img = img.convert('L')

    threshold = 220
    table = []
    for i in range(256):
        if i < threshold:
            table.append(0)
        else:
            table.append(1)
    # 图片二值化
    photo = Img.point(table, '1')
    photo.save(file_sto_name)
