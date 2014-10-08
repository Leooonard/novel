from ctypes import *
import ctypes


libping= cdll.LoadLibrary('/home/mymmoondt/mysite/mysite/libpcap.so')
result= ctypes.string_at(libping.checkPCAP('/home/mymmoondt/mysite/mysite/data-0-1.pcap'))
if(len(result)== 0):
	print "null"
print result
	
