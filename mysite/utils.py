#coding= utf-8

"""
    这个模块用来放置一些平常经常会使用的函数.
"""


from django.template.loader import get_template
from django.template import TemplateDoesNotExist, RequestContext
from django.shortcuts import render_to_response
from django.http import HttpResponse, HttpResponseRedirect, HttpRequest
from exceptions import NameError, KeyError, UnicodeDecodeError
import os

defaultPage= ""
errorPage= ""

# 防止模板名错误引起的问题, 错误后导向主页.
def RenderResponse(templateName, context= {}):
    try:
        return render_to_response(templateName, context)
    except TemplateDoesNotExist, e:
        if defaultPage:
            return render_to_response(defaultPage, {})
        else:
            return None

def RenderPaginatedResponse(templateName, request, context= {}):
    try:
        return render_to_response(templateName, context, context_instance= RequestContext(request))
    except TemplateDoesNotExist, e:
        return render_to_response('main.html', {})

def RenderRedirect(path):
    return HttpResponseRedirect(path)

def RenderErrorHTML(text= {}):
    try:
        context= {
            'title': text['title'],
            'content': text['content'],
            'href': text['href']
        }
    except(NameError,KeyError), e:
        context= {
            'title': '内部错误',
            'content': '服务器发生小错误啦 %>_<%,  5秒后重定向至我们主页! (⊙o⊙)~~~',
            'href': '/main/'
        }
    finally:
        if errorPage:
            return RenderResponse(errorPage, context)
        else:
            return None

def RenderTipHTML(text= {}):
    try:
        context= {
            'title': text['title'],
            'content': text['content'],
            'href': text['href']
        }
    # 多个异常时的写法要注意!!!
    except (NameError, KeyError), e:
        context= {
            'title': '内部错误',
            'content': '服务器发生小错误啦 %>_<%,  5秒后重定向至我们主页! (⊙o⊙)~~~',
            'href': '/main/'
        }
    finally:
        return RenderResponse('tip.html', context)

def CompareDate(date1, date2):
    if date1< date2:
        return -1
    elif date1== date2:
        return 0
    elif date1> date2:
        return 1

def PrintArrived(*args):
    print ''
    print '-'* 20+ 'Arrived!!!'+ '-'* 20
    print ''
    for item in args:
        print item
        print '-'* 50

def GetFileRealPath(file, relativePath):
    return os.path.join(os.path.split(os.path.realpath(file))[0], relativePath).replace('\\', '/')

try:
    import json
    def WriteObjectToJsonString(obj):
        try:
            return json.write(obj)
        except UnicodeDecodeError, e:
            for item in obj:
                try:
                    obj[item]= obj[item].decode('utf-8')
                except Exception, e:
                    pass
            return json.write(obj)


    def ReadObjectFromJsonString(string):
        return json.read(string)
except:
    pass

