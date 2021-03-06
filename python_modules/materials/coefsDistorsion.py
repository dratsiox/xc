# -*- coding: utf-8 -*-
from __future__ import division

__author__= "Ana Ortega (AO_O) and Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2015, AO_O and LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= " ana.Ortega.Ort@gmail.com, l.pereztato@gmail.com"

alphaSeccRectangular=5/6
alphaSeccCircular=6/7
def alphaSeccAnilloCircular(R,r):
    '''Coeficientes de distorsión para diferentes tipos de secciones
    Ver "Cálculo de estructuras por el método de los elementos finitos"
    de Eugenio Oñate página 122.}
    '''
    c=r/R
    K=c/(1+c**2)
    return 6/(7+20*(K)**2)
