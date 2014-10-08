from ctypes import *
import ctypes
bin= cdll.LoadLibrary('/home/mymmoondt/mysite/mysite/lab-ping.so')
rtval= bin.ping('/home/mymmoondt/mysite/mysite/data')
bin= cdll.LoadLibrary('/home/mymmoondt/mysite/mysite/read.so')
rtval= bin.checkPCAP(ctypes.string_at(rtval))
fd= open('/home/mymmoondt/repos/ns-3-allinone/ns-3-dev/scratch/Event', 'w')
fd.write(ctypes.string_at(rtval))
fd.close()
