# -*- coding: UTF-8 -*-

from aip import AipFace

# 定义常量
APP_ID = '10312161'
API_KEY = '5DrFz7umPy9oBHwu1qgAzxOr'
SECRET_KEY = 'tGQoG0cchrhutKjemnPvIaP4Mpyh5r07'

# 初始化AipFace对象
aipFace = AipFace(APP_ID, API_KEY, SECRET_KEY);

# 读取图片
filePath = "timg.jpeg"
def get_file_content(filePath):
	with open(filePath, 'rb') as fp:
		return fp.read()

# 定义参数变量
options={
	'max_face_num':1,
	'face_fields':"age,beauty,expression,faceshape",
}

#调用人脸属性检测接口
result = aipFace.detect(get_file_content(filePath), options)

print(result)
print(type(result))
