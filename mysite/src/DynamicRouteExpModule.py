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
import time, os, re, random, shutil, stat, zipfile

ERROR_ARRAY= ['XML文件读取错误.', 'XML文件错误, 缺少root元素.', 'XML文件错误, 缺少root元素.', 
				'XML文件错误, 缺少实验类型.', 'XML文件错误, 缺少节点数量.', 
				'节点数量超过范围, (节点数量应在0-100之间)', 'XML文件错误, 缺少网段数量.',
				'XML文件错误, 网段数量错误.']

def DynamicRouteExp(data, path):
	"""
		进行实验.
		data为具体配置数据. path为生成文件要写入的目录路径.
	"""
	Log.Log('DynamicRouteExp', 'data is '+ data)
	JSONObj= simplejson.loads(data)
	xmlPath= ConvertJSONtoXML(JSONObj, path)
	#修改了！！！！
	libPath= utils.GetFileRealPath(__file__, '../lab-ping.so')
	libping= cdll.LoadLibrary(libPath)
	rtVal= libping.ping(str(xmlPath)) #str()很关键！！！
	rtVal= string_at(rtVal)
	try:
		rtVal= int(rtVal) #尝试将字符串转型成数字.
		Log.Log("DynamicRouteExp", "rtVal is "+ str(rtVal))
	except:
		#转型失败说明实验产生结果. 可以进行数据包分析.
		rtVal= str(string_at(rtVal))[0: -1] #去除最后那个0
	else:
		#转型成功, 说明实验并未产生结果, 异常退出, 需要返回错误.
		return 'failed, '+ ERROR_ARRAY[rtVal]
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

	JSONObj['DOWNLOADPATH']= CreatePCAPZip(path) or '#' #如果返回false， 则赋值#.

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


def CreatePCAPZip(path):
	"""
		该函数用于将实验生成的所有PCAP数据包打包成ZIP文件(不压缩). 
		ZIP文件将放置于PCAP数据包同目录下. ZIP文件名将使用PCAP包的前缀名.
		输入为PCAP所在文件夹路径. 
		输出为生成的ZIP文件路径.
	"""
	regx= re.compile("\w+\.pcap$")
	filelist= []

	if not os.path.isdir(path):
		#传入的路径错误.
		return False

	for filename in os.listdir(path): #获取文件夹下所有文件的文件名.
		if re.search(regx, filename)!= None:
			#如果是pcap包则把名字加入数组.
			filelist.append(filename)

	if len(filelist)== 0:
		return False #文件夹下不存在pcap包.

	#首先获取前缀
	regx= re.compile("^\w+-")
	filename= filelist[0]
	prefix= re.search(regx, filename)
	if prefix== None:
		#pcap包文件名错误.
		return False
	prefix= prefix.group()[0: -1]

	#获取前缀后开始创建压缩文件
	zipPath= os.path.join(path, prefix+ '.zip')
	zipTool= zipfile.ZipFile(zipPath, 'w', zipfile.ZIP_STORED)
	for filename in filelist:
		zipTool.write(os.path.join(path, filename))
	zipTool.close()
	return os.path.join(path, prefix+ '.zip')


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






