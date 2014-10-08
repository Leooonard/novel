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

def DynamicRouteExp(data):
	JSONObj= simplejson.loads(data)
	xmlPath= JSON2XML(JSONObj)
	libPath= os.path.join(os.path.split(os.path.realpath(__file__))[0], '../lab-ping.so').replace('\\', '/')
	libping= cdll.LoadLibrary(libPath)
	rtVal= libping.ping(xmlPath)
	libPath= os.path.join(os.path.split(os.path.realpath(__file__))[0], '../read.so').replace('\\', '/')
	libping= cdll.LoadLibrary(libPath)
	rtVal= libping.checkPCAP(ctypes.string_at(rtVal))
	JSONStr= ctypes.string_at(rtVal)
	filePath= os.path.join(os.path.split(os.path.realpath(__file__))[0], '../data').replace('\\', '/')
	fd= open(filePath, 'w')
	fd.write(JSONStr)
	fd.close()
	JSONObj= simplejson.loads(JSONStr)
	for seg in JSONObj['ARRAY']:
		if(string.atoi(seg["NODE"]))> 1:
			seg["NODE"]= 'r'+ str(string.atoi(seg["NODE"]))	
		else:
			seg["NODE"]= 'h'+ seg["NODE"]
	JSONStr= simplejson.dumps(JSONObj)	
	return JSONStr

def JSON2XML(JSONObj):
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
			if(seg['ConObj1']['Type']== 'Route'):
				if(seg['ConObj1']['RouteInfo']['HostDev']):
					hNode= Element('local')
					hNode.set('ID', seg['ConObj1']['RouteID'][1: len(seg['ConObj1']['RouteID'])])
					hNode.set('type', 'router')
					root.append(hNode)
				if(seg['ConObj1']['RouteInfo']['TargetDev']):
					tNode= Element('remote')
					tNode.set('ID', seg['ConObj1']['RouteID'][1: len(seg['ConObj1']['RouteID'])])
					tNode.set('type', 'router')
					root.append(tNode)
			else:
				if(seg['ConObj1']['HostInfo']['HostDev']):
					hNode= Element('local')
					hNode.set('ID', seg['ConObj1']['HostID'][1: len(seg['ConObj1']['HostID'])])
					hNode.set('type', 'host')
					root.append(hNode)
				if(seg['ConObj1']['HostInfo']['TargetDev']):
					tNode= Element('remote')
					tNode.set('ID', seg['ConObj1']['HostID'][1: len(seg['ConObj1']['HostID'])])
					tNode.set('type', 'host')
					root.append(tNode)
			if(seg['ConObj2']['Type']== 'Route'):
				if(seg['ConObj2']['RouteInfo']['HostDev']):
					hNode= Element('local')
					hNode.set('ID', seg['ConObj2']['RouteID'][1: len(seg['ConObj2']['RouteID'])])
					hNode.set('type', 'router')
					root.append(hNode)
				if(seg['ConObj2']['RouteInfo']['TargetDev']):
					tNode= Element('remote')
					tNode.set('ID', seg['ConObj2']['RouteID'][1: len(seg['ConObj2']['RouteID'])])
					tNode.set('type', 'router')
					root.append(tNode)
			else:
				if(seg['ConObj2']['HostInfo']['HostDev']):
					hNode= Element('local')
					hNode.set('ID', seg['ConObj2']['HostID'][1: len(seg['ConObj2']['HostID'])])
					hNode.set('type', 'host')
					root.append(hNode)
				if(seg['ConObj2']['HostInfo']['TargetDev']):
					tNode= Element('remote')
					tNode.set('ID', seg['ConObj2']['HostID'][1: len(seg['ConObj2']['HostID'])])
					tNode.set('type', 'host')
					root.append(tNode)
		for seg in JSONObj['DATA']:
			segEle= Element('segment')
			segEle.set('ID', seg['segID'][1: len(seg['segID'])])
			devEle= Element('node')
			if(seg['ConObj1']['Type']== 'Host'):
				devEle.set('ID', seg['ConObj1']['HostID'][1: len(seg['ConObj1']['HostID'])])
				devEle.set('type', 'host')
				devIPEle= Element('ipaddress')
				devIPEle.text= seg['ConObj1']['HostInfo']['IP']
				devMaskIPEle= Element('mask')
				devMaskIPEle.text= seg['ConObj1']['HostInfo']['MaskIP']
				devEle.append(devIPEle)
				devEle.append(devMaskIPEle)
				segEle.append(devEle)
			elif(seg['ConObj1']['Type']== 'Route'):
				devEle.set('ID', seg['ConObj1']['RouteID'][1: len(seg['ConObj1']['RouteID'])])
				devEle.set('type', 'router')
				devIPEle= Element('ipaddress')
				devIPEle.text= seg['ConObj1']['RouteInfo']['IP']
				devMaskIPEle= Element('mask')
				devMaskIPEle.text= seg['ConObj1']['RouteInfo']['MaskIP']
				devEle.append(devIPEle)
				devEle.append(devMaskIPEle)
				devRouteTable= Element('routertable')
				rowCount= 0;
				for row in seg['ConObj1']['RouteInfo']['RouteTable']:
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
			devEle= Element('node')
			if(seg['ConObj2']['Type']== 'Host'):
				devEle.set('ID', seg['ConObj2']['HostID'][1: len(seg['ConObj2']['HostID'])])
				devEle.set('type', 'host')
				devIPEle= Element('ipaddress')
				devIPEle.text= seg['ConObj2']['HostInfo']['IP']
				devMaskIPEle= Element('mask')
				devMaskIPEle.text= seg['ConObj2']['HostInfo']['MaskIP']
				devEle.append(devIPEle)
				devEle.append(devMaskIPEle)
				segEle.append(devEle)
			elif(seg['ConObj2']['Type']== 'Route'):
				devEle.set('ID', seg['ConObj2']['RouteID'][1: len(seg['ConObj2']['RouteID'])])
				devEle.set('type', 'router')
				devIPEle= Element('ipaddress')
				devIPEle.text= seg['ConObj2']['RouteInfo']['IP']
				devMaskIPEle= Element('mask')
				devMaskIPEle.text= seg['ConObj2']['RouteInfo']['MaskIP']
				devEle.append(devIPEle)
				devEle.append(devMaskIPEle)
				devRouteTable= Element('routertable')
				rowCount= 0;
				for row in seg['ConObj2']['RouteInfo']['RouteTable']:
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
		xmlTree.write('/home/mymmoondt/mysite/mysite/data.xml', 'utf8')	
		return 	'/home/mymmoondt/mysite/mysite/data'






