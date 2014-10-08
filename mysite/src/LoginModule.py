#-*-coding:utf8-*-
from django.http import HttpResponse, HttpResponseRedirect
from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.core.context_processors import csrf
from django.template import context
from ctypes import *
from django.utils import simplejson
from xml.etree.ElementTree import ElementTree
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import dump
from xml.etree.ElementTree import Comment
from xml.etree.ElementTree import tostring
import os, ctypes, MySQLdb
from novel.models import student
import  random
import re


def Login(request):
	"""if request.session.get('log', False):
		return HttpResponseRedirect('/admin/')
	else:"""
	return checkLogInfo(request)

def checkLogInfo(request):
	idregx= re.compile('^[a-zA-Z0-9]{0,6}$')
	pwdregx= re.compile('^[a-zA-Z0-9]{0,10}$')
	id= request.POST.get('acc')
	pwd= request.POST.get('pwd')
	if not id or not pwd:
		c= {'RESULT': '0'}   #有空字段
		return render_to_response('Login.html', c)
	if not re.match(idregx, id) or not re.match(pwdregx, pwd):
		c= {'RESULT': '1'}   #格式错误
		return render_to_response('Login.html', c)
	try:
		stu= student.objects.get(id= id)
	except student.DoesNotExist:
		c= {'RESULT': '2'}    #不存在该帐号
		return render_to_response('Login.html', c)
	else:
		if stu.pwd!= pwd:
			c= {'RESULT': '3'}    #错误密码
			return render_to_response('Login.html', c)
		else:
			stuinfo= {
				'id': id,
				'pwd': pwd
			}
			request.session['stuinfo']= stuinfo
			return HttpResponseRedirect('../admin/')

def changePassword(request):
	id= request.POST.get('acc')
	pwd= request.POST.get('pwd')
	if not id or not pwd:
		c= {'RESULT': '0'}
		return render_to_response('forget.html', c)	
	idregx= re.compile('^[a-zA-Z0-9]{0,6}$')
	pwdregx= re.compile('^[a-zA-Z0-9]{0,10}$')
	if not re.match(idregx, id) or not re.match(pwdregx, pwd):
		c= {'RESULT': '1'}   #格式错误
		return render_to_response('Login.html', c)
	try:
		stu= student.objects.get(id= id)
	except student.DoesNotExist:
		c= {'RESULT': '2'}
		return render_to_response('forget.html', c)
	else:
		stu.pwd= pwd
		stu.save()
		return HttpResponseRedirect('/mysite/')


		
