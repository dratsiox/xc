# -*- coding: utf-8 -*-
# Acero para armaduras activas.
'''
   nmbAceroPret: Nombre del acero.
   fpk: Valor característico del límite elástico a tracción (Pa) (Artículo 38.7 EHE).
   fpd: Valor de cálculo del límite elástico a tracción (Pa).
   fmax: Valor característico de la carga unitaria máxima del material expresada en Pa.
   tInic: Valor de la tensión de pretensado a tiempo infinito.
  
'''
import xc_base
import geom
from materials import typical_materials

class AceroPretEHE(object):
  """Acero de pretensar EHE."""
  nmbMaterial= "nil" # Nombre del material.
  nmbDiagK= "dgK"+nmbMaterial # Nombre para el diagrama característico.
  matTagK= -1 # Tag del material uniaxial con el diagrama característico del material.
  nmbDiagD= "dgD"+nmbMaterial # Nombre para el diagrama de cálculo.
  matTagD= -1 # Tag del material uniaxial con el diagrama de cálculo del material.
  fpk= 1171e6 # Límite elástico del acero en Pa.
  gammaS= 1.15 # Minoración del material.
  fmax= 1860e6
  Es= 190e9 # Módulo elástico del material.
  bsh= 0.001 # Relación entre pendientes (rama cedencia/rama elástica)

  def __init__(self, nmbAcero,fpk,fmax):
    self.nmbMaterial= nmbAcero # Nombre del material.
    self.nmbDiagK= "dgK"+nmbAcero # Nombre para el diagrama característico.
    self.matTagK= -1 # Tag del material uniaxial con el diagrama característico del material.
    self.nmbDiagD= "dgD"+nmbAcero # Nombre para el diagrama de cálculo.
    self.matTagD= -1 # Tag del material uniaxial con el diagrama de cálculo del
    self.fpk= fpk # Límite elástico del acero en Pa.
    self.fmax= fmax

  def fpd(self):
    return self.fpk/self.gammaS
  def tInic(self):
    return 0.75**2*self.fmax # Pretensado final (incial al 75 por ciento y 25 por ciento de pérdidas totales).
  def defDiagK(self,preprocessor,initialStress):
    # Diagrama tensión-deformación característico.
    acero= typical_materials.defSteel02(preprocessor,self.nmbDiagK,self.Es,self.fpk,self.bsh,initialStress)
    self.matTagK= acero.tag
    return self.matTagK

  def defDiagD(self,preprocessor,initialStress):
    # Diagrama tensión-deformación de cálculo.
    acero= typical_materials.defSteel02(preprocessor,self.nmbDiagD,self.Es,self.fpd(),self.bsh,initialStress)
    self.matTagD= acero.tag
    return self.matTagD

  def getDiagK(self,preprocessor):
    return preprocessor.getMaterialLoader.getMaterial(self.nmbDiagK)
  def getDiagD(self,preprocessor):
    return preprocessor.getMaterialLoader.getMaterial(self.nmbDiagD)

 
Y1860S7= AceroPretEHE("Y1860S7",fpk= 1171e6,fmax= 1860e6)




