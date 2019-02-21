#!/usr/bin/env python
import numpy as np
from scipy.optimize import leastsq
from scipy.optimize import least_squares

"""Some data rejiggling functions"""
def logistic_model ( p, time ):
    """A logistic model, fitting the first steps of the phenological clcle"""
    return p[0] + p[1]* ( 1./(1+np.exp(-p[2]*(time-p[3]))) )
	
def dbl_logistic_model ( p, time ):
    """A double logistic model, as in Sobrino and Juliean, or Zhang et al"""
    return p[0] + p[1]* ( 1./(1+np.exp(-p[2]*(time-p[3]))) + 1./(1+np.exp( p[4]*(time-p[5])))  - 1 )
						  
def asymmetric_gaussian_model ( p, time ):
    """A logistic model, fitting the first steps of the phenological clcle"""
    return p[0] + p[1]* ( 1./(1+np.exp((p[2]-time)/p[3])**p[4]) )
	
def get_model(date, ndvi, params=None, pheno_model = "dbl_logistic"):
    
    if pheno_model == "logistic":
        pheno_func = logistic_model
        n_params = 4 # 4 terms
    elif pheno_model == "gaussian":
        pheno_func = asymmetric_gaussian_model
        n_params = 5 # 5 terms
    elif pheno_model == "dbl_logistic":
        n_params = 6 # 6 terms
        pheno_func = dbl_logistic_model
    
    if params is None:
        # The user hasn't provided a starting guess
        params = [.5,] * n_params
        # Dbl_logistic might require sensible starting point
        if pheno_model == "logistic":
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.19
            params[3] = date[0]
        elif pheno_model == "gaussian": 
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = (date[0]+date[len(date)-1])/2.
            params[3] = 2.
            params[4] = 2.      
        elif pheno_model == "dbl_logistic":      
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.19
            params[3] = date[0]
            params[4] = 0.13
            params[5] = date[len(date)-1]
    

    model = pheno_func(params, date)

    
    return model
	
def mismatch_function (p, pheno_func, ndvi_in, time):
    """The NDVI/Phenology model mismatch function. It will be minimised wrt to the VI observations. 
    This function will take different phenology models and NDVI."""

    # output stores the predictions    
    output = []
    
    fitness = lambda p, ndvi_in: \
                    ndvi_in - pheno_func (p, time )
    oot = fitness ( p, ndvi_in )            
    [ output.append ( x ) for x in oot ]
    
    #print('sto fittando')
    #print(p)
    return np.array(output).squeeze()
	
def fit_phenology_model (date, ndvi, pheno_model, params=None):
    
    """This function fits a phenology model of choice for a given NDVI and time period. """
  
    if pheno_model == "logistic":
        pheno_func = logistic_model
        n_params = 4 # 4 terms
    elif pheno_model == "gaussian":
        pheno_func = asymmetric_gaussian_model
        n_params = 5 # 5 terms
    elif pheno_model == "dbl_logistic":
        n_params = 6 # 6 terms
        pheno_func = dbl_logistic_model
    
    if params is None:
        print('The user has not provided a starting guess')
        params = [.5,] * n_params
        # Dbl_logistic might require sensible starting point
        if pheno_model == "logistic":
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.19
            params[3] = date[0]
            print(params)
        elif pheno_model == "gaussian": 
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = (date[0]+date[len(date)-1])/2.
            params[3] = 2.
            params[4] = 2.  
            print(params)
        elif pheno_model == "dbl_logistic":      
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.05
            params[3] = date[0]
            params[4] = 0.05
            params[5] = date[len(date)-1]
            print(params)
    
    #print(params)
    (xsol, msg) = leastsq (mismatch_function, params, \
        args=(pheno_func, ndvi, date), maxfev=1000000)
                 
    return (xsol, msg)


def constrained_fit_phenology_model (date, ndvi, pheno_model, bounds=None, params=None):

    """This function fits a phenology model of choice for a given NDVI and time period. """

    if pheno_model == "logistic":
        pheno_func = logistic_model
        n_params = 4 # 4 terms
    elif pheno_model == "gaussian":
        pheno_func = asymmetric_gaussian_model
        n_params = 5 # 5 terms
    elif pheno_model == "dbl_logistic":
        n_params = 6 # 6 terms
        pheno_func = dbl_logistic_model

    if params is None:
        print('The user has not provided a starting guess')
        params = [.5,] * n_params
        # Dbl_logistic might require sensible starting point
        if pheno_model == "logistic":
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.19
            params[3] = date[0]
            print(params)
        elif pheno_model == "gaussian":
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = (date[0]+date[len(date)-1])/2.
            params[3] = 2.
            params[4] = 2.
            print(params)
        elif pheno_model == "dbl_logistic":
            params[0] = ndvi.min()
            params[1] = ndvi.max() - ndvi.min()
            params[2] = 0.05
            params[3] = date[0]
            params[4] = 0.05
            params[5] = date[len(date)-1]
            print(params)

    #print(params)
    res = least_squares(mismatch_function, params, \
		    args=(pheno_func, ndvi, date), bounds=bounds, max_nfev=1000000)

    return res



