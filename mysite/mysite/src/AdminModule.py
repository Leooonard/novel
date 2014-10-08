#-*-coding:utf8-*-
from django.http import HttpResponse
from django.template.loader import get_template
from django.shortcuts import render_to_response
from django.core.context_processors import csrf
from django.template import context
from django.utils import simplejson
from novel.models import *



def Admin(request):	
	explist= expinfo.objects.all()
	l= []
	for exp in explist:
		id= exp.id
		tp= exp.type
		obj= {
			"ID": id,
			"Type": tp
		}
		l.append(obj)
	JSONStr= simplejson.dumps(l)
	print type(JSONStr)	
	c= {'INITSTR': JSONStr}
	return c
