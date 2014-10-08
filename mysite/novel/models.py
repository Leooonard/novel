#coding= utf-8

from django.db import models

# Create your models here.

class student(models.Model):
	id= models.CharField(primary_key= True, max_length= 6)
	pwd= models.CharField(max_length= 10)  

class expinfo(models.Model):
	id= models.CharField(primary_key= True, max_length= 6)
	type= models.CharField(max_length= 10, null= True)

class stu_exp_info(models.Model):
	stu= models.ForeignKey(student)
	exp= models.ForeignKey(expinfo)
	grade= models.IntegerField(null= True)
	content= models.TextField()
