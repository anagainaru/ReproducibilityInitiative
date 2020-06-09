
from numpy import *
from scipy.stats import *
from scipy.special import *
from math import *



global list_distrib

list_distrib=["TruncatedNormal"]

def phi(x):
	return ((1.0/sqrt(2.0*pi))*exp(-0.5*pow(x,2)))
def Phi(x):
	return (0.5*(1+erf(x/sqrt(2))))

# print(Phi(float('inf')))
# exit()




############################# DISTRIBUTIONS PARAMETERS & MEAN/VARIANCE #############################

def init_distributions(mean,std,a,b):
	
	global mu_truncatednormal, sigma2_truncatednormal, a_truncatednormal, b_truncatednormal, alpha_truncatednormal, beta_truncatednormal, mean_truncatednormal, variance_truncatednormal, min_truncatednormal
	# truncatednormal ; t in [a;b]
	mu_truncatednormal = mean # in R
	sigma2_truncatednormal = pow(std,2) # >= 1
	a_truncatednormal = a # >=0
	b_truncatednormal = b # > a

	alpha_truncatednormal = (a_truncatednormal-mu_truncatednormal)/sqrt(sigma2_truncatednormal)
	beta_truncatednormal = (b_truncatednormal-mu_truncatednormal)/sqrt(sigma2_truncatednormal)
	mean_truncatednormal = mu_truncatednormal + sqrt(sigma2_truncatednormal)* ((phi(alpha_truncatednormal)-phi(beta_truncatednormal)) / (Phi(beta_truncatednormal)-Phi(alpha_truncatednormal)))
	variance_truncatednormal = sigma2_truncatednormal * (1 + ((alpha_truncatednormal*phi(alpha_truncatednormal)-beta_truncatednormal*phi(beta_truncatednormal)) / (Phi(beta_truncatednormal)-Phi(alpha_truncatednormal))) - pow((phi(alpha_truncatednormal)-phi(beta_truncatednormal)) / (Phi(beta_truncatednormal)-Phi(alpha_truncatednormal)),2) )
	#variance_truncatednormal = sigma2_truncatednormal * (1 + ((alpha_truncatednormal*phi(alpha_truncatednormal))/(1-Phi(alpha_truncatednormal))) - pow(phi(alpha_truncatednormal)/(1-phi(alpha_truncatednormal)),2) )
	min_truncatednormal = a_truncatednormal
	



init_distributions(122.86333,13.68058,0,190)



## truncatednormal Distribution ##
def PDF_truncatednormal(t):
	return (phi((t-mu_truncatednormal)/sqrt(sigma2_truncatednormal)) / (sqrt(sigma2_truncatednormal)*(Phi(beta_truncatednormal)-Phi(alpha_truncatednormal))))
	#return (phi((t-mu_truncatednormal)/sqrt(sigma2_truncatednormal))/(sqrt(sigma2_truncatednormal)*(1-Phi(alpha_truncatednormal))))

def CDF_truncatednormal(t):
	p =((Phi((t-mu_truncatednormal)/sqrt(sigma2_truncatednormal))-Phi(alpha_truncatednormal))/(Phi(beta_truncatednormal)-Phi(alpha_truncatednormal)))
	return p
	#return (Phi((t-mu_truncatednormal)/sqrt(sigma2_truncatednormal))-Phi(alpha_truncatednormal))/(1-Phi(alpha_truncatednormal)) #( (erf((t-mu_truncatednormal)/(sqrt(sigma2_truncatednormal*2))) - erf((a_truncatednormal-mu_truncatednormal)/(sqrt(sigma2_truncatednormal*2)))) / (1-erf((a_truncatednormal-mu_truncatednormal)/(sqrt(sigma2_truncatednormal*2)))) )

def CDFr_truncatednormal(t):
	return 1.0-CDF_truncatednormal(t)

def quant_truncatednormal(quantile):
	return erfinv(2*(quantile*(Phi(beta_truncatednormal)-Phi(alpha_truncatednormal))+Phi(alpha_truncatednormal)-0.5))*sqrt(2*sigma2_truncatednormal) + mu_truncatednormal
	# return erfinv(quantile*(1-erf((a_truncatednormal-mu_truncatednormal)/sqrt(2*sigma2_truncatednormal)))+erf((a_truncatednormal-mu_truncatednormal)/sqrt(2*sigma2_truncatednormal)))*sqrt(2*sigma2_truncatednormal) + mu_truncatednormal


