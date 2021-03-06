# -*- coding: utf-8 -*-
from __future__ import division

__author__= "Ana Ortega (AO_O) and Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2015, AO_O and LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= " ana.Ortega.Ort@gmail.com, l.pereztato@gmail.com"

import scipy.interpolate
from materials import typical_materials
import math

def sqr(a):
  return a*a

# Rigideces de apoyos de neopreno

def getKhoriz(G, a, b, e):
  '''
  Rigidez frente al desplazamiento horizontal de
  un apoyo de neopreno rectangular.

  :param G: Módulo de cortante del elastómero.
  :param a,b: Lados del neopreno.
  :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).  
  '''
  return G*a*b/e

'''
Puntos que definen el valor de V2 en función de b/a
   ver libro de puentes de Manterola página 591
'''
x= [1,1.5,2,3,4,6,8,10,10000]
y= [0.208,0.231,0.246,0.267,0.282,0.299,0.307,0.313,1/3]
tablaV2neopreno= scipy.interpolate.interp1d(x,y)

'''
Puntos que definen el valor de V3 en función de b/a
   ver libro de puentes de Manterola página 591
'''
x= [1,1.5,2,3,4,6,8,10,10000]
y= [0.14,0.196,0.229,0.263,0.281,0.299,0.307,0.313,1/3]
tablaV3neopreno=  scipy.interpolate.interp1d(x,y)

def getV2(a, b):
    '''
    Factor de forma v2 de  un apoyo de neopreno rectangular.

    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
    '''
    return tablaV2neopreno(b/a)

def getV3(a, b):
    '''
    Factor de forma v3 de  un apoyo de neopreno rectangular.

    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
    '''
    return tablaV3neopreno(b/a)

def getEneopreno(G, a, b, e):
    '''
    Módulo elástico frente al desplazamiento vertical de
    un apoyo de neopreno rectangular.

    :param G: Módulo de cortante del elastómero.
    :param a: Lado del neopreno paralelo al eje del dintel.
    :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).
   v3: Factor de forma del apoyo.
  
    '''
    return 3*G*sqr(a/e)*getV3(a,b)

def getKvert(G, a, b, e):
    '''
    Rigidez frente al desplazamiento vertical de un neopreno rectangular.
    :param E: Módulo elástico del elastómero.
    :param a,b: Lados del neopreno.
    :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).
  
    '''
    return getEneopreno(G,a,b,e)*a*b/e

    '''
    Puntos que definen el valor de V4 en función de b/a
    ver libro de puentes de Manterola página 592
    '''
x= [1,2,4,8,10000]
y= [85.7,71.4,64.5,61.2,60]
tablaV4neopreno= scipy.interpolate.interp1d(x,y)


def getV4(a, b):
    '''
    Factor de forma v4 de  un apoyo de neopreno rectangular.

    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
  
    '''
    return tablaV4neopreno(b/a)


def getKgiroEjeDintel(G, a, b, e):
    '''
    Rigidez frente al giro en torno a un eje paralelo al del dintel
    que pase por el centro de un neopreno rectangular.

    :param E: Módulo elástico del elastómero.
    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
    :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).
  
    '''
    return G*a*pow(b,5.0)/(getV4(a,b)*pow(e,3.0))

def getKgiroEjeNormalAlDintel(G, a, b, e):
    '''
    Rigidez frente al giro en torno a un eje normal al del dintel
    que pase por el centro de un neopreno rectangular.

    :param E: Módulo elástico del elastómero.
    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
    :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).
  
    '''
    return G*b*pow(a,5.0)/(getV4(a,b)*pow(e,3.0))

'''
    Puntos que definen el valor de beta en función de h/b
   ver ALPHA en libro hormigón Jiménez Montoya 14a. edición página 405
'''
x= [1,1.25,1.5,2,3,4,6,10,10000]
y= [0.14,0.171,0.196,0.229,0.263,0.281,0.299,0.313,1/3]
tablaBetaTorsionNeoprenoRectang= scipy.interpolate.interp1d(x,y)

def ifte(a,b,c):
  if(a):
    return b
  else:
    return c

def getBetaTorsionNeoprenoRectang(b, h):
  '''
  Devuelve el valor del coeficiente beta en función de h/b
  ver ALPHA en libro hormigón Jiménez Montoya 14a. edición página 405
  '''
  return ifte(b<h,tablaBetaTorsionNeoprenoRectang(h/b),tablaBetaTorsionNeoprenoRectang(b/h))

def getKgiroEjeVertical(G, a, b, e):
    '''
    Rigidez frente al giro en torno a un eje vertical
    que pase por el centro de un neopreno rectangular.

    :param E: Módulo elástico del elastómero.
    :param a: Lado del neopreno paralelo al eje del dintel.
    :param b: Lado del neopreno normal al eje del dintel.
    :param e: espesor neto de elastómero (el total menos el ocupado por las chapas).
    '''
    return getBetaTorsionNeoprenoRectang(a,b)*G*a*pow(b,3.0)/e

# Define materiales para modelizar neopreno.
def defineMaterialesNeopreno(preprocessor,G, a, b, e, nmbMatKX, nmbMatKY, nmbMatKZ, nmbMatKTHX, nmbMatKTHY, nmbMatKTHZ):
  matKX= typical_materials.defElasticMaterial(preprocessor, nmbMatKX, getKhoriz(G,a,b,e))
  matKY= typical_materials.defElasticMaterial(preprocessor, nmbMatKY, getKhoriz(G,a,b,e))
  matKZ= typical_materials.defElasticMaterial(preprocessor, nmbMatKZ, getKvert(G,a,b,e))
  matKTHX= typical_materials.defElasticMaterial(preprocessor, nmbMatKTHX, getKgiroEjeDintel(G,a,b,e))
  matKTHY= typical_materials.defElasticMaterial(preprocessor, nmbMatKTHY, getKgiroEjeNormalAlDintel(G,a,b,e))
  matKTHZ= typical_materials.defElasticMaterial(preprocessor, nmbMatKTHZ, getKgiroEjeVertical(G,a,b,e))

