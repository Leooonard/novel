#-*-coding:utf8-*-
from django.http import HttpResponse
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

def PersonalSetting(request):
	Acc= request.POST.get('ACC')
	Pwd= request.POST.get('PWD')
	CHPWDFLAG= request.POST.get('CHPWDFLAG', False)  #改密码
	conn= MySQLdb.connect(host= 'localhost', user= 'root', passwd= '', db= 'LabOL')
	cur= conn.cursor()
	cur.execute('select * from stu_exp_info where ID=%s', (Acc))
	result= cur.fetchall()
	count= 0
	JSONArray= []
	while count< len(result):
		exp_type= result[count][1]
		exp_content= result[count][2]
		exp_report= result[count][3]
		exp_grade= result[count][4]
		exp_id= result[count][5]
		temp= dict(TYPE= exp_type, CONTENT= exp_content, REPORT= exp_report, GRADE= exp_grade, ID= exp_id)
		JSONArray.append(temp)
		count= count+ 1
	
	JSONObj= dict(JSONArray)
	JSONStr= simplejson.dumps(JSONObj)
	if(CHPWDFLAG):
		cur.execute('update user_info set pwd=%s where ID=%s', (Pwd, Acc))
		conn.commit()
		cur.close()
		conn.close()
		c= {'UPDATE': 1, 'ACC': Acc, 'PWD': Pwd, 'JSONStr': JSONStr}
		c.update(csrf(request))
		return c
	else:
		c= {'ACC': Acc, 'PWD': Pwd, 'JSONStr': JSONStr}
		c.update(csrf(request))
		cur.close()
		conn.close()
		return c
