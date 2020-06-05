# import matplotlib.pyplot as plt
# import matplotlib.mlab as mlab
# from matplotlib import rc
from copy import *
from cmath import *
import scipy
import os


import distri
from distri import *
#from settings import *
import settings



class NotStableException(Exception):
	pass



########################## START Application Time ##########################


# Get recovery time of reservation k in schedule
def Rk(schedule,k):
	# if (k==0):
	# 	return 0
	if(k<=1):
		return 0
	tmp = 1
	for i in range(1,k):
		tmp = tmp * (1-schedule[i][1])
	return ((1-tmp)*settings.R)
	


# Get actual length of reservation k in schedule
def Tk(schedule,k):
	tk = schedule[k][0]
	if (k==0):
		return schedule[k][0]
	max_ti = schedule[0][0]
	for i in range(1,k):
		if (schedule[i][1]):
			max_ti = schedule[i][0]
	return tk - max_ti



# Get checkpointing time of reservation k in schedule
def Ck(schedule,k):
	assert(schedule[k][1]==1 or schedule[k][1]==0)
	if (k==0 or k==(len(schedule)-1)):
		return 0
	return schedule[k][1]*getattr(settings,"C")



# Get the actual cost of reservation k in schedule
def Wk(schedule,k):
	if(k==0):
		return 0
	return (Rk(schedule,k) + Tk(schedule,k) + Ck(schedule,k))










########################## TOOLS TO DISCRETIZE A CONTINUOUS PROBABILITY DISTRIB ##########################





# Generate a liste of (ti,pi) following Equal-time scheme
def discretization_time(distrib, numberChunks):
	result = [[],[]] # list of ti, list of pi
	
	tmax = getattr(settings, "max_"+distrib.lower()) #globals()["max_"+distrib.lower()]
	tmin = getattr(distri, "min_"+distrib.lower())#globals()["min_"+distrib.lower()]  
  
	for i in range(1,numberChunks+1):
		tmp = tmin +(i*((tmax-tmin)/numberChunks))
		result[0].append(tmp)

	result[1].append((globals()["CDF_"+distrib.lower()](result[0][0])-globals()["CDF_"+distrib.lower()](tmin))/globals()["CDF_"+distrib.lower()](tmax))
	
	for i in range(1,numberChunks):
		result[1].append((globals()["CDF_"+distrib.lower()](result[0][i])-globals()["CDF_"+distrib.lower()](result[0][i-1]))/globals()["CDF_"+distrib.lower()](tmax))
	
	# cpt = 0.0
	# for i in range(0,len(result[1])):
	# 	cpt+=result[1][i]
	# print("sum proba _proba = "+str(cpt))
	return result



########################## END TOOLS TO DISCRETIZE A CONTINUOUS PROBABILITY DISTRIB ##########################