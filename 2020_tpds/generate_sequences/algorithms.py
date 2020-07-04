from copy import *
from cmath import *
import scipy
import os
from numpy import *
import settings

import distri
from tools import *

# rc('text', usetex=True)





####### LIST OF ALGORITHMS HERE:
### ALL-CKPT
### MEME-ALL-CKPT












########################## START Algorithms ##########################


### distrib = [[vi,...],[fi...]]
def compute_fk(distrib):
	s = len(distrib[1])
	res = [0] * (s+1)
	for i in range(s-1, -1, -1):
		res[i] = distrib[1][i] + res[i+1]
	return res



#distrib = [[],[]] list of v_i and list of fi
def Eckpt(expt_t, opt_j, opt_delta_j, ic, il, distrib, distrib_name, fk, numberChunks, waitingTimeFunction):
	min_ = float('inf')
	j= il+1
	delta_j = 0
	for j_ in range(il+1,numberChunks+1): # for j in [il+1,numberChunks]
		fk_ =  fk[j_]
		# for i in range(j_,numberChunks): # summation of the fi for k=j+1 to n 
		# 	fk_+=distrib[1][i]
		
		tmp,candidate=0,0
		##### if (ic!=0)
		if (ic):
			# if delta_j_=0
			tmp =  expt_t[ic][j_] + (waitingTimeFunction(settings.R+(distrib[0][j_-1]-distrib[0][ic-1] ))+getattr(settings, "beta")*settings.R)*fk[il] + getattr(settings, "beta")*(distrib[0][j_-1]-distrib[0][ic-1] )*fk_
			# if delta_j_=1
			candidate =  expt_t[j_][j_] + (waitingTimeFunction(settings.R+(distrib[0][j_-1]-distrib[0][ic-1] )+settings.C)+getattr(settings, "beta")*settings.R)*fk[il] +getattr(settings, "beta")*settings.C*fk_
		##### if (ic>0)
		else:
			# if delta_j_=0
			tmp = expt_t[0][j_] + waitingTimeFunction(distrib[0][j_-1])*fk[il] + settings.beta*(distrib[0][j_-1])*fk_
			# if delta_j_=1
			candidate = expt_t[j_][j_] + waitingTimeFunction(distrib[0][j_-1]+settings.C)*fk[il] + settings.beta*settings.C*fk_
		# guess best expectation is when j candidate is checkpointed
		delta_j_ = 1
		# we look if the non checkpointed cost for j is better than the checkpointed one
		if (tmp<candidate):
			candidate=deepcopy(tmp)
			delta_j_=0
		# look for global minimum over the different j
		if (candidate<min_):
			j = deepcopy(j_)
			min_= deepcopy(candidate)
			delta_j = deepcopy(delta_j_)
	assert(min_!= float('inf'))
	expt_t[ic][il] = deepcopy(min_)
	opt_j[ic][il] = deepcopy(j)
	opt_delta_j[ic][il] = deepcopy(int(delta_j))
	return expt_t,opt_j,opt_delta_j



#distrib = [[],[]] list of v_i and list of fi
### ALL-CKPT algorithm
def dynamic_programming_discrete_allcheckpoint(distrib, distrib_name, waitingTimeFunction, numberChunks):
	min_distrib = getattr(distri, "min_"+distrib_name.lower())#globals()["min_"+distrib.lower()]  
	mean_distrib = getattr(distri, "mean_"+distrib_name.lower())
	

	expt_t = []
	assert( len(distrib[0])==numberChunks and len(distrib[1])==numberChunks)
	assert(numberChunks)
	
	n = numberChunks

	# compute all the values of fk
	fk = compute_fk(distrib) 

	# tables to save expectation and indexes for backtracking
	expt_t = zeros(numberChunks+1)
	jstar = zeros(numberChunks+1)

	
	# initialization
	tim = settings.beta*mean_distrib
	expt_t[n] = tim

	# main loop
	for i in range(n-1,-1,-1): 
		R_ = settings.R
		if (i==0):
			R_=0

		min_=float('inf')
		j = -1
		for j_ in range (i+1,n+1):
			vj = distrib[0][j_-1]
			vi = 0
			if (i!=0):
				vi = distrib[0][i-1]
			C_ = settings.C
			if (j_==numberChunks):
				C_ = 0
			candidate = expt_t[j_] + settings.beta*settings.C*fk[j_-1] + (waitingTimeFunction(R_ + (vj-vi)+C_) + settings.beta*R_)*fk[i]
			if (candidate<min_):
				min_=deepcopy(candidate)
				j = deepcopy(j_)
		expt_t[i]=deepcopy(min_)
		jstar[i]=deepcopy(j)
		assert(j>=0)
		assert(min_!=float('inf'))
	

	# backtracking
	liste = []
	current_ind = int(jstar[0])
	while(current_ind<n):
		liste.append(current_ind)
		current_ind = int(jstar[current_ind])
	liste.append(n)

	#print([distrib[0][i] for i in liste])
	
	result = [[min_distrib,0]]
	for i in liste:
		result.append([distrib[0][i-1],1])
	result[len(result)-1][1]=0
	#print(result)
	# return expt_t[0],result
	return result


#distrib = [[],[]] list of v_i and list of fi
### MEM-ALL_CKPT algorithm
def dynamic_programming_discrete_allcheckpoint_dynamic(distrib, ckpt_trace, distrib_name, waitingTimeFunction, numberChunks):
	min_distrib = getattr(distri, "min_"+distrib_name.lower())#globals()["min_"+distrib.lower()]  
	mean_distrib = getattr(distri, "mean_"+distrib_name.lower())
	

	expt_t = []
	assert( len(distrib[0])==numberChunks and len(distrib[1])==numberChunks)
	assert(numberChunks)
	
	n = numberChunks

	# compute all the values of fk
	fk = compute_fk(distrib) 

	# tables to save expectation and indexes for backtracking
	expt_t = zeros(numberChunks+1)
	jstar = zeros(numberChunks+1)

	
	# initialization
	tim = 0
	expt_t[n] = tim

	# main loop
	for i in range(n-1,-1,-1): 
		R_ = settings.R
		if (i==0):
			R_=0

		min_=float('inf')
		j = -1
		for j_ in range (i+1,n+1):
			vj = distrib[0][j_-1]
			vi = 0
			if (i!=0):
				vi = distrib[0][i-1]
			C_ = ckpt_trace[i]
			if (j_==numberChunks):
				C_ = 0
			candidate = expt_t[j_] + (R_ + vj-vi+C_)*fk[i]
			if (candidate<min_):
				min_=deepcopy(candidate)
				j = deepcopy(j_)
		expt_t[i]=deepcopy(min_)
		jstar[i]=deepcopy(j)
		assert(j>=0)
		assert(min_!=float('inf'))
	

	# backtracking
	liste = []
	current_ind = int(jstar[0])
	while(current_ind<n):
		liste.append(current_ind)
		current_ind = int(jstar[current_ind])
	liste.append(n)

	#print([distrib[0][i] for i in liste])
	
	result = [[min_distrib,0]]
	for i in liste:
		result.append([distrib[0][i-1],ckpt_trace[i-1]])
	result[len(result)-1][1]=0
	#print(result)
	# return expt_t[0],result
	return result














