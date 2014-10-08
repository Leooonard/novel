#coding= utf-8

from ctypes import *
from django.utils import simplejson
from xml.etree.ElementTree import ElementTree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import dump
from xml.etree.ElementTree import Comment
from xml.etree.ElementTree import tostring
import os, ctypes, MySQLdb
from mysite.form import testForm
from novel.models import *
from src import LoginModule, AdminModule, DynamicRouteExpModule, Log, RegisterModule, PersonalSettingModule
from utils import *


class pingpac:
	dataNum= 'null'
	Length= 'null'
	proto= 'null'
	srcIP= 'null'
	dstIP= 'null'
	info= 'null'


def admin(request):
	c= AdminModule.Admin(request)
	return RenderResponse('admin.html', c)

def Login(request):
	return RenderResponse('Login.html')

def DoLogin(request):
	return LoginModule.Login(request)

def changePassword(request):
	return LoginModule.changePassword(request)

def Register(request):
	return RenderResponse('Register.html')

def DoRegister(request):
	return RegisterModule.Register(request)

def PersonalSetting(request):
	c= PersonalSettingModule.PersonalSetting(request)
	return RenderResponse('PersonalSetting.html', c)

def TeacherSetting(request):
	return RenderResponse('TeacherSetting.html')	

def readPCAP(trace):
	libping= cdll.LoadLibrary('/home/mymmoondt/mysite/mysite/libpcap.so')
	result= ctypes.string_at(libping.checkPCAP(trace))
	# print result
	pcapArr= []
	findex= 0
	lindex= 0
	while result[lindex]!= '$':
		count= 0
		x= pingpac()
		while count< 6:
			if(count== 0):
				if(result[lindex]== '|'):
					x.dataNum= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
				else:
					lindex= lindex+ 1
			elif(count== 1):
				if(result[lindex]== '|'):
					x.Length= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
				else:
					lindex= lindex+ 1
			elif(count== 2):
				if(result[lindex]== '|'):
					x.proto= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
					if(x.proto== "ARP"):
						x.srcIP= "null"
						x.dstIP= "null"
						x.info= "null"
						count= 6
				else:
					lindex= lindex+ 1
			elif(count== 3):
				if(result[lindex]== '|'):
					x.srcIP= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
				else:
					lindex= lindex+ 1
			elif(count== 4):
				if(result[lindex]== '|'):
					x.dstIP= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
				else:
					lindex= lindex+ 1
			elif(count== 5):
				if(result[lindex]== '|'):
					x.info= result[findex: lindex]
					findex= lindex+ 1
					lindex= lindex+ 1
					count= count+ 1
				else:
					lindex= lindex+ 1
		pcapArr.append(x)
	return pcapArr

def PingExp(request):
	showTable= request.POST.get('sExp', 'false')
	if(showTable== 'true'):
		libping= cdll.LoadLibrary('/home/mymmoondt/mysite/mysite/libping.so')
		result= ctypes.string_at(libping.checkPCAP('/home/mymmoondt/mysite/mysite/ping.pcap'))
		pcapArr= []
		findex= 0
		lindex= 0
		while result[lindex]!= '$':
			count= 0
			x= pingpac()
			while count< 6:
				if(count== 0):
					if(result[lindex]== '|'):
						x.dataNum= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
					else:
						lindex= lindex+ 1
				elif(count== 1):
					if(result[lindex]== '|'):
						x.Length= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
					else:
						lindex= lindex+ 1
				elif(count== 2):
					if(result[lindex]== '|'):
						x.proto= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
						if(x.proto== "ARP"):
							x.srcIP= "null"
							x.dstIP= "null"
							x.info= "null"
							count= 6
					else:
						lindex= lindex+ 1
				elif(count== 3):
					if(result[lindex]== '|'):
						x.srcIP= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
					else:
						lindex= lindex+ 1
				elif(count== 4):
					if(result[lindex]== '|'):
						x.dstIP= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
					else:
						lindex= lindex+ 1
				elif(count== 5):
					if(result[lindex]== '|'):
						x.info= result[findex: lindex]
						findex= lindex+ 1
						lindex= lindex+ 1
						count= count+ 1
					else:
						lindex= lindex+ 1
			pcapArr.append(x)
		context= {'showTable': 1, 'pcapPack': pcapArr}
		context.update(csrf(request))		
		return render_to_response('PingExp.html', context)
	elif(showTable== 'false'):
		context= {}
		context.update(csrf(request))		
		return render_to_response('PingExp.html', context)

def RouteExp(request):
	fd= open('/home/mymmoondt/mysite/templates/RouteExp.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def DynamicRouteExp(request):
	context= {
		"ID": request.POST.get("ID", "None")
	}
	return render_to_response('DynamicRouteExp.html', context)

def ResultDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/ResultDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def DynamicRouteConfDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/DynamicRouteConfDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def DynamicHostConfDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/DynamicHostConfDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def PingHostConfDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/PingHostConfDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def PingTarConfDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/PingTarConfDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def PingRouteConfDialog(request):
	fd= open('/home/mymmoondt/mysite/templates/PingRouteConfDialog.html')
	html= fd.read()
	fd.close()
	return HttpResponse(html)

def RunPingExp(request):
	c= {}
	c.update(csrf(request))
	return render_to_response('PingHostConfDialog.html', c)

def test(request):
	"""form= testForm(request.POST)
	if form.is_valid():
		return HttpResponse(form.cleaned_data)"""

	c= {}
	c.update(csrf(request))
	return render_to_response('test.html', c)

def testDialog(request):
	c= {}
	c.update(csrf(request))
	return render_to_response('testDialog.html', c)
	

def RegisterProc(request):
	c= {}
	c.update(csrf(request))
	return render_to_response('Register.html', c)


def Ajaxtest(request):
	pass


def AjaxLoad(request):
	try:
		id= request.session['stuinfo'].get('id')
		stu= student.objects.get(id= id)
		expid= request.GET.get('id')
		exp= expinfo.objects.get(id= expid)
		se= stu_exp_info.objects.get(stu= stu, exp= exp)
		content= se.content
		return HttpResponse(content)
	except Exception, e:
		#保证一定会有返回值, 不至于产生500错误
		return HttpResponse('failed')


def AjaxSave(request):
	try:
		id= request.session['stuinfo'].get('id')
		stu= student.objects.get(id= id)
		expid= request.GET.get('id')
		exp= expinfo.objects.get(id= expid)
		try:
			se= stu_exp_info.objects.get(stu= stu, exp= exp)
		except stu_exp_info.DoesNotExist:
			data= request.GET.get('data')
			se= stu_exp_info(stu= stu, exp= exp, content= data)
			se.save()
		else:
			data= request.GET.get('data')
			se.content= data
			se.save()
		return HttpResponse('success')
	except:
		print "not catch!"
		return HttpResponse('failed')

def AjaxDynamicRouteExp(request):
	import traceback
	try:
		data= request.GET.get('data')
		JSONStr= DynamicRouteExpModule.DynamicRouteExp(data)
		Log.ErrorLog(AjaxDynamicRouteExp.__name__, JSONStr)
		return HttpResponse(JSONStr)
	except:
		msg= Log.GetErrorMessage(traceback)
		Log.ErrorLog(AjaxDynamicRouteExp.__name__, msg)
		return HttpResponse('failed')




