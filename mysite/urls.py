from django.conf.urls import patterns, include, url
from mysite import settings

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('mysite.view',
	('^novel/$', 'Login'),
	('^novel/admin/$', 'admin'),
	('^novel/dologin/$', 'DoLogin'),
	('^novel/Register/$', 'Register'),	
	('^novel/doregister/$', 'DoRegister'),
	('^novel/PersonalSetting/$', 'PersonalSetting'),
	('^novel/DynamicRouteExp$', 'DynamicRouteExp'),
	('^novel/forget/$', 'changePassword'),
	('^novel/ajaxtest/$', 'Ajaxtest'),
	('^novel/ajaxload/$', 'AjaxLoad'),
	('^novel/ajaxsave/$', 'AjaxSave'),
	('^novel/ajaxdynamicrouteexp/$', 'AjaxDynamicRouteExp'),
)

urlpatterns+= patterns('',
	('^static_media/(?P<path>.*)$', 'django.views.static.serve',  
		{'document_root':settings.STATICFILES_DIRS, 'show_indexes': True}),
)
