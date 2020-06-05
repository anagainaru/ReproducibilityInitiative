import distri
from distri import *
import random


def init_settings():
    global R,C,alpha,beta,gamma_
    R = 0
    C = 0
    alpha = 1.0 #0.95 #1.0
    gamma_ = 0.0 #3771.84 #0.1
    beta = 0.0 # Cost function: WT(T)+ beta.min(X,T)
    
    global pthres, max_truncatednormal
    pthres = 0.9999999 ## threshold pmax value for discretization (the upperbound where we truncate)
    max_truncatednormal = distri.b_truncatednormal
   

    


########################## START Waiting Time Functions ##########################


############### Linear ###############

# WT(t) = t --> alpha = 1.0, gamma = 0.0
def linear(time):
	return alpha*time + gamma_