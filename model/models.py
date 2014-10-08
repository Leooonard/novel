#coding= utf-8

from django.db import models

# Create your models here.

class userInfo(models.Model):
	ID= models.CharField(primary_key= True, max_length= 6)
	pwd= models.CharField(max_length= 10)
	#0代表学生, 1代表老师
	userType= models.IntegerField()   


	def __unicode__(self):
		return "%s, %s"%(self.ID, self.pwd)

