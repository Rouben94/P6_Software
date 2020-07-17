import numpy as np
from scipy.special import erf
import math
# Read out Excel Values to work with

import pandas as pd
from pandas import ExcelWriter
from pandas import ExcelFile

df = pd.read_excel('Gauss-Strahl.xlsx', sheet_name='Strahlweite')
df_cleaned = df.dropna(how='all')
df_x = df_cleaned['x [mm]'].astype(np.float16)
x = df_x.values
df_y = df_cleaned['P [mW]'].astype(np.float16)
y = df_y.values

print(x)
print(y)


# curve-fit() function imported from scipy 
from scipy.optimize import curve_fit 
  
from matplotlib import pyplot as plt 

# Test function with coefficients as parameters
P=y
def test(x, P0, Pmax,x0,w): 
    return P0 + Pmax/2 * (1-erf(math.sqrt(2)*(x-x0)/w))
    #return P0 + Pmax/2 * (1-(math.sqrt(2)*(x-x0)/w))
  
# curve_fit() function takes the test-function 
# x-data and y-data as argument and returns  
# the coefficients a and b in param and 
# the estimated covariance of param in param_cov 
param, param_cov = curve_fit(test, x, P) 
  
  
print("Funcion coefficients:") 
print(param) 
print("Covariance of coefficients:") 
print(param_cov) 
  
  
'''Below 4 lines can be un-commented for plotting results  
using matplotlib as shown in the first example. '''
  
# plt.plot(x, y, 'o', color ='red', label ="data") 
# plt.plot(x, ans, '--', color ='blue', label ="optimized data") 
# plt.legend() 
# plt.show()

from sympy import symbols, Eq, solve
w, w0, z, z0, z1 = symbols('w w0 z z0 z1')
z=50
w=0.254950975679639
w0=0.25
z1=100
#eq1 = Eq(w0*math.sqrt(1+((z-z1)/z0)**2)-w)

sol = solve(eq1,z0)
print(sol)
