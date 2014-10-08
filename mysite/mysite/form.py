from django import forms

class testForm(forms.Form):
	name= forms.CharField()
	pwd= forms.CharField()
