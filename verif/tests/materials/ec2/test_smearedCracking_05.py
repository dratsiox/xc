# -*- coding: utf-8 -*-

__author__= "Ana Ortega (AO_O) "
__copyright__= "Copyright 2016, AO_O"
__license__= "GPL"
__version__= "3.0"
__email__= "ana.ortega@ciccp.es "

''' Evaluation of crack amplitude in a RC rectangular section under
bending moment.
The data are taken from Example 7.5 of the manual «EC2-worked examples»
titled «Application of the approximated method [EC2 clause 7.4]» 

The section is 1000mm width by 500 mm height. As=10fi26. A bending moment of 
600 kNm is applied
'''

import xc_base
import geom
import xc
import math
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials.ec2 import EC2_concrete
from materials import concreteBase
from materials.ec2 import EC2_reinforcing_steel
from materials import typical_materials
from materials.fiber_section import createFiberSets
from materials.fiber_section import sectionReport 
import matplotlib.pyplot as plt
import numpy as np
from materials.ec2 import crack_control_utils

#Data from the experiment
width=1.0     #width (cross-section coordinate Y)
depth=0.5     #depth (cross-section coordinate Z)
f_ck=33e6      #concrete characteristic strength [Pa] (concrete C30-37 adopted)
f_ct=3.086*1e6   # concrete tensile strength
               # (in the test the calculated fctm=2896468.15 is used)

coverInf=0.05     #bottom cover
coverLat=0.04     #lateral cover
A_s=6903e-6    #area of bottom reirnfocement layer (6 fi 26)

ro_s=A_s/width/depth  #longitudinal steel ratio 

ro_s_eff=0.0643875431034      #effective ratio of reinforcement

M_y=-600e3      #bending moment [Nm]



#Other data
nDivIJ= 20  #number of cells (fibers) in the IJ direction (cross-section coordinate Y)
nDivJK= 20  #number of cells (fibers) in the JK direction (cross-section coordinate Z)

areaFi26=math.pi*(26e-3)**2/4.0

l= 1e-7     # Distance between nodes


# Model definition
problem=xc.ProblemaEF()              #necesary to create this instance of
                                     #the class xc.ProblemaEF()
preprocessor=problem.getPreprocessor
nodes= preprocessor.getNodeLoader     #nodes container
modelSpace= predefined_spaces.StructuralMechanics3D(nodes)  #Defines the dimension of nodes  three coordinates (x,y,z) and six DOF for each node (Ux,Uy,Uz,thetaX,thetaY,thetaZ)


nodes.defaultTag= 1 #First node number.
nod= nodes.newNodeXYZ(1.0,0,0)     #node 1 defined by its (x,y,z) global coordinates
nod= nodes.newNodeXYZ(1.0+l,0,0)   #node 2 defined by its (x,y,z) global coordinates

# Materials definition
concrete=EC2_concrete.EC2Concrete("C33",-33e6,1.5)          #concrete according to EC2 fck=33 MPa      

#Reinforcing steel.
rfSteel=EC2_reinforcing_steel.S450C          #reinforcing steel according to EC2 fyk=450 MPa
steelDiagram= rfSteel.defDiagK(preprocessor) #Definition of steel stress-strain diagram in XC. 


#Parameters for tension stiffening of concrete
paramTS=concreteBase.paramTensStiffenes(concrMat=concrete,reinfMat=rfSteel,reinfRatio=ro_s_eff,diagType='K')
concrete.tensionStiffparam=paramTS           #parameters for tension stiffening are assigned to concrete
concrDiagram=concrete.defDiagK(preprocessor) #Definition of concrete stress-strain diagram in XC.

# Section geometry (rectangular 0.3x0.5, 20x20 cells)
geomSectFibers= preprocessor.getMaterialLoader.newSectionGeometry("geomSectFibers")
y1= width/2.0
z1= depth/2.0
#concrete region
regiones= geomSectFibers.getRegions
concrSect= regiones.newQuadRegion(concrete.nmbDiagK)
concrSect.nDivIJ= nDivIJ
concrSect.nDivJK= nDivJK
concrSect.pMin= geom.Pos2d(-y1,-z1)
concrSect.pMax= geom.Pos2d(+y1,+z1)

#reinforcement layers
reinforcement= geomSectFibers.getReinfLayers
#bottom layer (positive bending)
reinfBottLayer= reinforcement.newStraightReinfLayer(rfSteel.nmbDiagK) #Steel stress-strain diagram to use.
reinfBottLayer.numReinfBars= 13
reinfBottLayer.barArea= areaFi26
yBotL=(width-2*coverLat-0.026)/2.0
zBotL=-depth/2.0+coverInf+0.026/2.0
reinfBottLayer.p1= geom.Pos2d(-yBotL,zBotL) # center point position of the starting rebar
reinfBottLayer.p2= geom.Pos2d(yBotL,zBotL) # center point position of the starting rebar

#Section material 
#it is a generic section created to be assigned to the elements specified
#its stress and strain state is neutral (if we ask this section for stress or strain
#values the result is always 0)
materiales= preprocessor.getMaterialLoader
sctFibers= materiales.newMaterial("fiber_section_3d","sctFibers")
fiberSectionRepr= sctFibers.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSectFibers")
sctFibers.setupFibers()



# Elements definition
elementos= preprocessor.getElementLoader
elementos.defaultMaterial='sctFibers'
elementos.dimElem= 1
elementos.defaultTag= 1
elem= elementos.newElement("zero_length_section",xc.ID([1,2]))

# Constraints
constCont= preprocessor.getConstraintLoader      #constraints container
fix_node_6dof.fixNode6DOF(constCont,1)
#fix_node_6dof.Nodo6DOFMovYZImpedidos(constCont,2)
fix_node_6dof.Nodo6DOFMovXGiroYLibres(constCont,2)
#fix_node_6dof.Nodo6DOFMovXGirosYZLibres(constCont,2)
# Loads definition
cargas= preprocessor.getLoadLoader   #loads container

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
pointLoad=xc.Vector([0,0,0,0,M_y,0])
lp0.newNodalLoad(2,pointLoad)    #applies the point load on node 2 

#We add the load case to domain.
casos.addToDomain("0")           #reads load pattern "0" and adds it to the domain

# Solve
#analisis= predefined_solutions.simple_newton_raphson(problem)
analisis= predefined_solutions.simple_static_modified_newton(problem)
analOk= analisis.analyze(1)


#printing results
nodos= preprocessor.getNodeLoader
nodos.calculateNodalReactions(True)
nodos= preprocessor.getNodeLoader

'''
RN1= nodos.getNode(1).getReaction[0]   #Axial FX reaction (constrained DOF: ux) at node 1
RQY1= nodos.getNode(1).getReaction[1]   #Vertical FY reaction (constrained DOF: uY) at node 1
RQZ1= nodos.getNode(1).getReaction[2]   #Vertical FY reaction (constrained DOF: uZ) at node 1
RMX1= nodos.getNode(1).getReaction[3]   #Bending moment Mx reaction at node 1
RMY1= nodos.getNode(1).getReaction[4]   #Bending moment My reaction at node 1
RMZ1= nodos.getNode(1).getReaction[5]   #Bending moment Mz reaction at node 1


RN2= nodos.getNode(2).getReaction[0]   #Axial FX reaction (constrained DOF: ux) at node 2
RQY2= nodos.getNode(2).getReaction[1]   #Vertical FY reaction (constrained DOF: uY) at node 2
RQZ2= nodos.getNode(2).getReaction[2]   #Vertical FY reaction (constrained DOF: uZ) at node 2
RMX2= nodos.getNode(2).getReaction[3]   #Bending moment Mx reaction at node 2
RMY2= nodos.getNode(2).getReaction[4]   #Bending moment My reaction at node 2
RMZ2= nodos.getNode(2).getReaction[5]   #Bending moment Mz reaction at node 2

print 'Rnode1= (',nodos.getNode(1).getReaction[0],',',nodos.getNode(1).getReaction[1],',',nodos.getNode(1).getReaction[2],',',nodos.getNode(1).getReaction[3],',',nodos.getNode(1).getReaction[4],',',nodos.getNode(1).getReaction[5],')'
print 'Rnode2= (',nodos.getNode(2).getReaction[0],',',nodos.getNode(2).getReaction[1],',',nodos.getNode(2).getReaction[2],',',nodos.getNode(2).getReaction[3],',',nodos.getNode(2).getReaction[4],',',nodos.getNode(2).getReaction[5],')'
'''
elementos= preprocessor.getElementLoader
ele1= elementos.getElement(1)
#section of element 1: it's the copy of the material section 'sctFibers' assigned
#to element 1 and specific of this element. It has the tensional state of the element
sccEl1= ele1.getSection()         
fibersSccEl1= sccEl1.getFibers()

#Creation of two separate sets of fibers: concrete and reinforcement steel 
setsRCEl1= createFiberSets.fiberSectionSetupRCSets(scc=sccEl1,concrMatTag=concrete.matTagK,concrSetName="concrSetFbEl1",reinfMatTag=rfSteel.matTagK,reinfSetName="reinfSetFbEl1")

sumAreas= fibersSccEl1.getSumaAreas(1.0)  #total sum of the fibers area
                                    #that sum is multiplied by the coefficient
                                    #passed as a parameter

Iz= fibersSccEl1.getIz(1.0,0.0)
IEIy= sccEl1.getInitialTangentStiffness().at(3,3)
IEIz= sccEl1.getInitialTangentStiffness().at(2,2)
TEIy= sccEl1.getTangentStiffness().at(3,3)
TEIz= sccEl1.getTangentStiffness().at(2,2)
Iy= fibersSccEl1.getIy(1.0,0.0)
esfN= fibersSccEl1.getResultant()
esfMy= fibersSccEl1.getMy(0.0)
esfMz= fibersSccEl1.getMz(0.0)
defMz= sccEl1.getSectionDeformationByName("defMz")
defN= sccEl1.getSectionDeformationByName("defN")
x= sccEl1.getNeutralAxisDepth()
Resul= sccEl1.getStressResultant()
Deform= sccEl1.getSectionDeformation()



#Results for the concrete fibers in section of element 1

fibraCEpsMin= -1
fConcrMin= setsRCEl1.concrFibers.getFiberWithMinStrain()
epsCMin= fConcrMin.getMaterial().getStrain() # minimal strain among concrete fibers
sgCMin= fConcrMin.getMaterial().getStress()  # stress in the concrete fiber with minimal strain
YepsCMin= fConcrMin.getPos().x # y coord. of the concrete fiber with minimal strain
ZepsCMin= fConcrMin.getPos().y # z coord. of the concrete fiber with minimal strain

fConcrMax= setsRCEl1.concrFibers.getFiberWithMaxStrain()
epsCMax= fConcrMax.getMaterial().getStrain() # maximal strain among concrete fibers
sgCMax= fConcrMax.getMaterial().getStress()  # stress in the concrete fiber with maximal strain
YepsCMax= fConcrMax.getPos().x # y coord. of the concrete fiber with maximal strain
ZepsCMax= fConcrMax.getPos().y # z coord. of the concrete fiber with maximal strain


#Results for the steel fibers in section of element 1
fReinfMax= setsRCEl1.reinfFibers.getFiberWithMaxStrain()
epsSMax= fReinfMax.getMaterial().getStrain() # maximal strain among steel fibers
sgSMax= fReinfMax.getMaterial().getStress() # stress in the steel fiber with maximal strain
YepsSMax= fReinfMax.getPos().x # y coord. of the steel fiber with maximal strain
ZepsSMax= fReinfMax.getPos().y # z coord. of the steel fiber with maximal strain

'''
print "sumAreas= ",(sumAreas)
print "Iz= ",(Iz)
print "IEIz= ",IEIz
print "TEIz= ",TEIz
print "E1= ",(IEIz/Iz)
print "Iy= ",(Iy)
print "IEIy= ",IEIy
print "TEIy= ",TEIy
print "E2= ",(IEIy/Iy)
print "Neutral fiber depth: ",x," m"
print "Sum of normal stresses: ",Resul*1e-3," kN"
print "Strain: ",Deform*1e3,"E-3"

print "\nMinimum strain in concrete fibers: ",(epsCMin*1E3),"E-3"
print "Minimum stress in concrete fibers: ",(sgCMin/1e6),"E6"

print "Y coordinate of the concrete fiber with minimum strain: ",(YepsCMin)
print "Z coordinate of the concrete fiber with minimum strain: ",(ZepsCMin)

print "\nMinimum strain in steel fibers: ",(epsSMin*1E3),"E-3"
print "Minimum stress in steel fibers: ",(sgSMin/1e6),"E6"
print "Y coordinate of the steel fiber with minimum strain: ",(YepsSMin)
print "Z coordinate of the steel fiber with minimum strain: ",(ZepsSMin)

print "\nMaximum strain in steel fibers: ",(epsSMax*1E3),"E-3"
print "Maximum stress in steel fibers: ",(sgSMax/1e6),"E6"
print "Y coordinate of the steel fiber with maximum strain: ",(YepsSMax)
print "Z coordinate of the steel fiber with maximum strain: ",(ZepsSMax)
'''



#              *Crack width calculation*

#depth of the effective area:
hceff=crack_control_utils.h_c_eff(depth_tot=depth,depht_eff=depth-coverInf-0.026/2.0,depth_neutral_axis=abs(x))
# print 'depth of the effective area: ',hceff,' m'
#effective reinforcement ratio
roseff=crack_control_utils.ro_eff(A_s=A_s,width=width,h_c_eff=hceff)
# print 'effective reinforcement ratio: ',roseff
#maximum crack spacing
srmax=crack_control_utils.s_r_max(k1=0.8,k2=0.5,k3=3.4,k4=0.425,cover=coverInf,fiReinf=0.024,ro_eff=roseff)
# print 'maximum crack spacing: ',srmax,' m'
#mean strain in the concrete between cracks
eps_cm=concrete.fctm()/concrete.E0()/2.0
#mean strain in the reinforcemen takin into account the effects of tension stiffening
eps_sm=epsSMax
#crack withs
w_k=srmax*(eps_sm-eps_cm)
# print 'crack withs: ',w_k*1e3, ' mm'

xComp= -0.178266877222
epsCMinComp= -0.0006080494122
sgCMinComp= 930543.821563
epsSMaxComp= 0.000949058858185
sgSMaxComp= 189811771.637
hceffComp= 0.107244374259
roseffComp= 0.0643670127005
srmaxComp= 0.233386505429
eps_cmComp= 4.67648930823e-05
w_kComp= 0.000210583235385

ratio1=(xComp-x)/xComp
ratio2=(epsCMinComp- epsCMin)/epsCMinComp
ratio3=(sgCMinComp- sgCMin)/sgCMinComp
ratio4=(epsSMaxComp- epsSMax)/epsSMaxComp
ratio5=(sgSMaxComp- sgSMax)/sgSMaxComp
ratio6=(hceffComp-hceff)/hceffComp
ratio7=(roseffComp-roseff)/roseffComp
ratio8=(srmaxComp-srmax)/srmaxComp
ratio9=(eps_cmComp-eps_cm)/eps_cmComp
ratio10=(w_kComp-w_k)/w_kComp

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-5) & (abs(ratio2)<1e-5) & (abs(ratio3)<1e-5) & (abs(ratio4)<1e-3) & (abs(ratio5)<1e-2) & (abs(ratio6)<1e-3) & (abs(ratio7)<1e-2) & (abs(ratio8)<1e-3) & (abs(ratio9)<1e-3) & (abs(ratio10)<1e-5):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."

####  FIGURES & REPORTS
# ## plotting of section geometric and mechanical properties
# from materials.fiber_section import plotGeomSeccion as pg
# pg.plotGeomSeccion(geomSectFibers,'./secEx74.eps')


# #sectInf=sectionReport.SectionInfo(preprocessor=preprocessor,section=sctFibers) #Obtains section parameters for report
# #sectInf.writeReport(archTex='./secEx74.tex', pathFigura='./secEx74.eps')

# #plot cross-section strains and stresses 
# from postprocess import utils_display
# '''
#   fiberSet: set of fibers to be represented
#   title:    general title for the graphic
#   fileName: name of the graphic file (defaults to None: no file generated)
#   nContours: number of contours to be generated (defaults to 100)
# ''' 
# #utils_display.plotStressStrainFibSet(fiberSet=fibersSccEl1,title='cross-section concrete fibers',fileName='problem.jpeg')
# utils_display.plotStressStrainFibSet(fiberSet=setsRCEl1.concrFibers.fSet,title='cross-section concrete fibers',fileName='problem.jpeg')

# #report of the section material
# sectParam=sectionReport.SectionInfo(preprocessor=preprocessor,sectName='example_7.5_EC2W_0.2mm',sectDescr='Test example 7.5 EC2 Worked examples - $w_k \\approx 0.2 mm$. Section definition',concrete=concrete,rfSteel=rfSteel,concrDiag=concrDiagram,rfStDiag=steelDiagram,geomSection=geomSectFibers,width=width,depth=depth) #Obtains section parameters for report
# sectParam.writeReport(archTex='figures/sections/secEx75B.tex', pathFigura='figures/sections/secEx75B.eps')

