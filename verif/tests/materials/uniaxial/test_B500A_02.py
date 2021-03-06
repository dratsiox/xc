# -*- coding: utf-8 -*-
# Home made test
# Design stress-strain diagram of the steel B500A (SIA)
import xc_base
import geom
import xc

from materials.sia262 import SIA_reinforcing_steel
from materials import reinforcingSteelTest

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

# Model definition
prueba= xc.ProblemaEF()
prueba.logFileName= "/tmp/borrar.log" # Para no imprimir mensajes de advertencia
preprocessor=  prueba.getPreprocessor
# Define materials
errMax= reinforcingSteelTest.testDiagDAceroArmar(preprocessor, SIA_reinforcing_steel.B500A)


# print "errMax= ",(errMax)
import os
fname= os.path.basename(__file__)
if errMax<1e-10:
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
