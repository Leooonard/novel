from django.conf.urls import patterns, include, url
from mysite import settings
from mysite import view
# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
	('^novel/$', view.PreProcess, {'func': view.Login}),
	('^novel/admin/$', view.PreProcess, {'func': view.admin}),
	('^novel/dologin/$', view.PreProcess, {'func': view.DoLogin}),
	('^novel/Register/$', view.PreProcess, {'func': view.Register}),	
	('^novel/doregister/$', view.PreProcess, {'func': view.DoRegister}),
	('^novel/PersonalSetting/$', view.PreProcess, {'func': view.PersonalSetting}),
	('^novel/DynamicRouteExp/$', view.PreProcess, {'func': view.DynamicRouteExp}),
	('^novel/forget/$', view.PreProcess, {'func': view.changePassword}),
	('^novel/ajaxtest/$', view.PreProcess, {'func': view.Ajaxtest}),
	('^novel/ajaxload/$', view.PreProcess, {'func': view.AjaxLoad}),
	('^novel/ajaxsave/$', view.PreProcess, {'func': view.AjaxSave}),
	('^novel/ajaxdynamicrouteexp/$', view.PreProcess, {'func': view.AjaxDynamicRouteExp}),
)

urlpatterns+= patterns('',
	('^static_media/(?P<path>.*)$', 'django.views.static.serve',  
		{'document_root':settings.STATICFILES_DIRS, 'show_indexes': True}),
)
