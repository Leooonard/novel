#-*-coding:utf8-*-


from ctypes import *
from django.utils import simplejson
from xml.etree.ElementTree import ElementTree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import dump
from xml.etree.ElementTree import Comment
from xml.etree.ElementTree import tostring
import os, ctypes, MySQLdb
import string
from .. import utils
import Log
import time, os, re, random, shutil, stat


def DynamicRouteExp(data, path):
	JSONObj= simplejson.loads(data)
	xmlPath= ConvertJSONtoXML(JSONObj, path)
	#修改了！！！！
	libPath= utils.GetFileRealPath(__file__, '../lab-ping.so')
	libping= cdll.LoadLibrary(libPath)
	Log.Log('DynamicRouteExp', 'xmlPath is '+ xmlPath)
	rtVal= libping.ping(xmlPath)
	Log.Log('DynamicRouteExp', 'rtval is '+ ctypes.string_at(rtVal))
	libPath= utils.GetFileRealPath(__file__, '../read.so')
	libping= cdll.LoadLibrary(libPath)
	rtVal= libping.checkPCAP(ctypes.string_at(rtVal))
	JSONStr= ctypes.string_at(rtVal)
	JSONObj= simplejson.loads(JSONStr)
	for seg in JSONObj['ARRAY']:
		if(string.atoi(seg["NODE"]))> 1:
			seg["NODE"]= 'r'+ str(string.atoi(seg["NODE"]))	
		else:
			seg["NODE"]= 'h'+ seg["NODE"]
	JSONStr= simplejson.dumps(JSONObj)	
	return JSONStr

def CreateUserFolder(acc, everytime= False):
	"""
		帮每个用户创建一个属于自己的文件夹.
		everytime为True则每次都创建一个全新的文件夹. 文件夹名为time.time()+ acc+ 8位随机数
		everytime为False则只创建一次文件夹. 文件夹名为acc. 并且若已经存在, 将删除之前的文件夹,
		重新建立新同名文件夹.

		若文件夹已存在, 直接返回路径. 否则创建文件夹后返回路径.
	"""
	if everytime:
		currentTime= str(int(time.time()))
		folderName= currentTime+ acc+ str(random.randint(10000000, 99999999))
	else:
		folderName= acc
	folderPath= utils.GetFileRealPath(__file__, '../ExpResult/'+ folderName)
	if not everytime and os.path.isdir(folderPath):
		shutil.rmtree(folderPath, True)
	if os.path.isdir(folderPath):
		return folderPath
	else:
		os.mkdir(folderPath)
		os.chmod(folderPath, stat.S_IRWXU+ stat.S_IRWXG+ stat.S_IRWXO) #修改权限
		return folderPath

def ConvertJSONtoXML(JSONObj, path):
		xmlTree= ElementTree()
		root= Element('root')
		xmlTree._setroot(root)
		ExpType= Element('experitype')
		ExpType.text= 'ROUTER'
		root.append(ExpType)
		NodeCount= Element('nodecount')
		NodeCount.text= str(JSONObj['NodeCount'])
		root.append(NodeCount)
		SegCount= Element('segmentcount')
		SegCount.text= str(len(JSONObj['DATA']))
		root.append(SegCount)
		for seg in JSONObj['DATA']:
			for key in seg:
				regx= re.compile('^ConObj\d$')
				if regx.match(key)!= None:
					if(seg[key]['Type']== 'Route'):
						if(seg[key]['RouteInfo']['HostDev']):
							hNode= Element('local')
							hNode.set('ID', seg[key]['RouteID'][1: len(seg[key]['RouteID'])])
							hNode.set('type', 'router')
							root.append(hNode)
						if(seg[key]['RouteInfo']['TargetDev']):
							tNode= Element('remote')
							tNode.set('ID', seg[key]['RouteID'][1: len(seg[key]['RouteID'])])
							tNode.set('type', 'router')
							root.append(tNode)
					else:
						if(seg[key]['HostInfo']['HostDev']):
							hNode= Element('local')
							hNode.set('ID', seg[key]['HostID'][1: len(seg[key]['HostID'])])
							hNode.set('type', 'host')
							root.append(hNode)
						if(seg[key]['HostInfo']['TargetDev']):
							tNode= Element('remote')
							tNode.set('ID', seg[key]['HostID'][1: len(seg[key]['HostID'])])
							tNode.set('type', 'host')
							root.append(tNode)
		for seg in JSONObj['DATA']:
			segEle= Element('segment')
			segEle.set('ID', seg['segID'][1: len(seg['segID'])])
			for key in seg:
				regx= re.compile('^ConObj\d$')
				if regx.match(key)!= None:
					devEle= Element('node')
					if(seg[key]['Type']== 'Host'):
						devEle.set('ID', seg[key]['HostID'][1: len(seg[key]['HostID'])])
						devEle.set('type', 'host')
						devIPEle= Element('ipaddress')
						devIPEle.text= seg[key]['HostInfo']['IP']
						devMaskIPEle= Element('mask')
						devMaskIPEle.text= seg[key]['HostInfo']['MaskIP']
						devEle.append(devIPEle)
						devEle.append(devMaskIPEle)
						segEle.append(devEle)
					elif(seg[key]['Type']== 'Route'):
						devEle.set('ID', seg[key]['RouteID'][1: len(seg[key]['RouteID'])])
						devEle.set('type', 'router')
						devIPEle= Element('ipaddress')
						devIPEle.text= seg[key]['RouteInfo']['IP']
						devMaskIPEle= Element('mask')
						devMaskIPEle.text= seg[key]['RouteInfo']['MaskIP']
						devEle.append(devIPEle)
						devEle.append(devMaskIPEle)
						devRouteTable= Element('routertable')
						rowCount= 0;
						for row in seg[key]['RouteInfo']['RouteTable']:
							rowEle= Element('item')
							rowEle.set('ID', str(rowCount))
							rowCount= rowCount+ 1
							rowIPEle= Element('destaddress')
							rowIPEle.text= row['IP']
							rowNextSkipEle= Element('nexthop')
							rowNextSkipEle.text= row['NextSkip']
							rowMaskIPEle= Element('mask')
							rowMaskIPEle.text= row['MaskIP']
							rowEle.append(rowIPEle)
							rowEle.append(rowMaskIPEle)
							rowEle.append(rowNextSkipEle)
							devRouteTable.append(rowEle)
						devEle.append(devRouteTable)
						segEle.append(devEle)
			root.append(segEle)
		xmlTree.write(os.path.join(path, 'data.xml'), 'utf8')	
		return 	os.path.join(path, 'data')






