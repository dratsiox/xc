# -*- coding: utf-8 -*-
''' Verification test of una sección de hormigón armado.
   los resultados se comparan con los obtenidos del prontuario.
   informático del hormigón estructural (Cátedra de hormigón de la ETSICCP-IECA
   UPM). '''

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"


import math
import xc_base
import geom
import xc

from materials.ehe import EHE_concrete
from materials.ehe import EHE_reinforcing_steel

# Coeficientes de seguridad.
gammac= 1.5 # Coeficiente de minoración de la resistencia del hormigón.
gammas= 1.15 # Coeficiente de minoración de la resistencia of the steel.


CurvZ1= 11.474e-3 # Curvatura de prueba 1.
x1= 0.0997 # Profundidad de la fibra neutra 1.

CurvZ2= 34.787e-3 # Curvatura de prueba 2.
x2= 0.0649 # Profundidad de la fibra neutra 2.
prueba= xc.ProblemaEF()
prueba.logFileName= "/tmp/borrar.log" # Ignore warning messages
preprocessor=  prueba.getPreprocessor

concr=EHE_concrete.HA25
concr.alfacc=0.85    #f_maxd= 0.85*fcd coeficiente de fatiga del hormigón (generalmente alfacc=1)

tag= concr.defDiagD(preprocessor)
tag= EHE_reinforcing_steel.B500S.defDiagD(preprocessor)
# Define materials
import os
pth= os.path.dirname(__file__)
if(not pth):
  pth= "."
#print "pth= ", pth
execfile(pth+"/secc_hormigon_01.py")

secHA= preprocessor.getMaterialLoader.newMaterial("fiber_section_3d","secHA")
fiberSectionRepr= secHA.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSecHormigon01")
secHA.setupFibers()

epsG1= CurvZ1*(defSec['depth']/2-x1)
secHA.revertToStart()
secHA.setTrialSectionDeformation(xc.Vector([epsG1,CurvZ1,0]))
secHA.commitState()
N1= secHA.getStressResultantComponent("N")
Mz1= secHA.getStressResultantComponent("Mz")
Mz1Dato= 54.4e3
ratio1= (Mz1-Mz1Dato)/Mz1Dato

epsG2= CurvZ2*(defSec['depth']/2-x2)
secHA.revertToStart()
secHA.setTrialSectionDeformation(xc.Vector([epsG2,CurvZ2,0]))
secHA.commitState()
N2= secHA.getStressResultantComponent("N")
Mz2= secHA.getStressResultantComponent("Mz")
Mz2Dato= 55.7e3
ratio2= (Mz2-Mz2Dato)/Mz2Dato




''' 
print "Axil calculado; N1= ",(N1/1e3)," kN \n"
print "Momento z calculado; Mz1= ",(Mz1/1e3)," kN m\n"
print "Momento z dato; Mz1Dato= ",(Mz1Dato/1e3)," kN \n"
print "ratio1= ",(ratio1)

print "Axil calculado; N2= ",(N2/1e3)," kN \n"
print "Momento z calculado; Mz2= ",(Mz2/1e3)," kN m\n"
print "Momento z dato; Mz2Dato= ",(Mz2Dato/1e3)," kN \n"
print "ratio2= ",(ratio2)
 '''

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-4) & (abs(ratio2)<1e-4) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."

