from django.conf.urls import patterns, include, url
from mysite import settings

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('mysite.view',
	('^mysite/admin/$', 'admin'),
	('^mysite/$', 'Login'),
	# ('^mysite/checkLogInfo/$', 'checkLogInfo'),
	('^mysite/dologin/$', 'DoLogin'),
	('^mysite/Register/$', 'Register'),	
	('^mysite/doregister/$', 'DoRegister'),
	('^mysite/PingExp/$', 'PingExp'),
	('^mysite/RouteExp/$', 'RouteExp'),
	('^mysite/DynamicRouteExp/$', 'DynamicRouteExp'),
	('^mysite/PingHostConfDialog/$', 'PingHostConfDialog'),
	('^mysite/PingTarConfDialog/$', 'PingTarConfDialog'),
	('^mysite/RunPingExp/$', 'RunPingExp'),
	('^mysite/test/$', 'test'),
	('^mysite/testDialog/$', 'testDialog'),
	('^mysite/PingRouteConfDialog/$', 'PingRouteConfDialog'),
	('^mysite/RegisterProc/$', 'RegisterProc'),
	('^mysite/PersonalSetting/$', 'PersonalSetting'),
	('^mysite/TeacherSetting/$', 'TeacherSetting'),
	('^mysite/ResultDialog/$', 'ResultDialog'),
	('^mysite/forget/$', 'changePassword'),
	('^mysite/ajaxtest/$', 'Ajaxtest'),
	('^mysite/ajaxload/$', 'AjaxLoad'),
	('^mysite/ajaxsave/$', 'AjaxSave'),
	('^mysite/ajaxdynamicrouteexp/$', 'AjaxDynamicRouteExp'),
)

urlpatterns+= patterns('',
	('^static_media/(?P<path>.*)$', 'django.views.static.serve',  
		{'document_root':settings.STATICFILES_DIRS, 'show_indexes': True}),
)

urlpatterns+= patterns('',
	(r'^admin/', include(admin.site.urls)),
)
