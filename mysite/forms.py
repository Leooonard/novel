from django import forms

class LoginForm(forms.Form):
	userName= forms.CharField(max_length= 30)
	password= forms.CharField(max_length= 10)
	def clean_password(self):
		raise forms.ValidationError('everything is wrong!')