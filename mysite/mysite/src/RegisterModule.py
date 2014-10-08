#coding= utf-8

from django.http import *
from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.core.context_processors import csrf
from django.template import context
import os, ctypes, MySQLdb
from novel.models import *
import re

def Register(request):
	return checkRegInfo(request)

def checkRegInfo(request):
	if (not request.POST.get('accInfo', None))or (not request.POST.get('pwdInfo', None)):
		c= {'RESULT': '1'}
		return render_to_response('Register.html', c)
	else:
		id= request.POST.get('accInfo')
		pwd= request.POST.get('pwdInfo')
		idregx= re.compile('^[a-zA-Z0-9]{0,6}$')
		pwdregx= re.compile('^[a-zA-Z0-9]{0,10}$')
		if not re.match(idregx, id) or not re.match(pwdregx, pwd):
			c= {'RESULT': '1'}   #格式错误
			return render_to_response('Login.html', c)
		try:
			student.objects.get(id= id)
		except student.DoesNotExist:
			newUser= student(id= id, pwd= pwd)  #0 is the student and 1 is the teacher
			newUser.save()
			c= {'RESULT': '3'}
		else:
			c= {'RESULT': '2'}
		return render_to_response('Register.html', c)