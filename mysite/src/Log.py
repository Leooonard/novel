#coding= utf-8
import os
import fcntl
import time
import StringIO

"""
	该模块专用于记录程序执行过程中的信息至文件.
	支持文件锁. 防止文件操作.
	各记录文件已定义. 可改动.
"""


errorLogPath= os.path.join(os.path.split(os.path.realpath(__file__))[0], '../ErrorLog').replace('\\', '/') #在Log.py文件上一级目录
logPath= os.path.join(os.path.split(os.path.realpath(__file__))[0], '../Log').replace('\\', '/')


def ErrorLog(funcName, errorInfo):
	"""
		函数用于记录错误信息至文件errorLog.txt.
		在打开文件时要注意, 文件是否正在被使用.
		文件中记录的错误条目格式为:
		时间. 函数名. 具体错误信息.
	"""
	file= open(errorLogPath, "a")
	fcntl.flock(file.fileno(), fcntl.LOCK_EX)  #获取文件锁
	errorTime= time.strftime("%Y-%m-%d %H:%M:%S. ", time.localtime(time.time())) #发生错误的时间. 时间格式为2014-10-7 22:02:00.
	funcName= funcName+ '. ' #发生错误的函数名.
	errorInfo= errorInfo+ '\n' #具体的错误信息
	file.write(errorTime+ funcName+ errorInfo)
	file.close() #关闭文件. 同时失去文件锁.

def Log(funcName, message):
	"""
		函数用于记录某些调试信息至文件Log.txt
		在打开文件时要注意, 文件是否正在被使用.
		文件中记录的错误条目格式为:
		时间. 函数名. 具体错误信息.
	"""
	file= open(logPath, "a")
	fcntl.flock(file.fileno(), fcntl.LOCK_EX)  #获取文件锁
	messageTime= time.strftime("%Y-%m-%d %H:%M:%S. ", time.localtime(time.time())) #发生错误的时间. 时间格式为2014-10-7 22:02:00.
	funcName= funcName+ '. ' #发生错误的函数名.
	message= message+ '\n' #具体的错误信息
	file.write(messageTime+ funcName+ message)
	file.close() #关闭文件. 同时失去文件锁.


def GetErrorMessage(traceback):
	"""
		函数用于从traceback中获取相应的错误信息.
	"""
	fp= StringIO.StringIO()
	traceback.print_exc(file= fp)
	message= fp.getvalue()
	return message