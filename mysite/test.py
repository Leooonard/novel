from ctypes import *
import ctypes
bin= cdll.LoadLibrary('/home/sse-server-04/git/novel/mysite/lab-ping.so')
rtval= bin.ping('/home/sse-server-04/git/novel/mysite/ExpResult/092912/data')

