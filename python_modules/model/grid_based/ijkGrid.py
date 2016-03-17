# -*- coding: utf-8 -*-

'''ijkGrid.py: model generation based on a grid of 3D positions.'''

__author__= "Ana Ortega (AOO) and Luis C. Pérez Tato (LCPT)"
__cppyright__= "Copyright 2015, AOO and LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com" 

import xc_base
import geom
import xc

class IJKRange(object):
  '''Defines a range of indexes i,j,k in the grid to be used
  select the region bounded by the coordinates associated with
  those indexes.
  Attributes:
    ijkMin: list with minimum value of indexes [minIindex,minJindex,minKindex]
    ijkMin: list with maximum value of indexes [maxIindex,maxJindex,maxKindex]
  '''
  def __init__(self,ijkMin,ijkMax):
    self.ijkMin= ijkMin
    self.ijkMax= ijkMax
  def getIMin(self):
    '''returns the value of the index I minimum in the range '''
    return self.ijkMin[0]
  def getIMax(self):
    '''returns the value of the index I maximum in the range '''
    return self.ijkMax[0]
  def getJMin(self):
    '''returns the value of the index J minimum in the range '''
    return self.ijkMin[1]
  def getJMax(self):
    '''returns the value of the index J maximum in the range '''
    return self.ijkMax[1]
  def getKMin(self):
    '''returns the value of the index K minimum in the range '''
    return self.ijkMin[2]
  def getKMax(self):
    '''returns the value of the index K maximum in the range '''
    return self.ijkMax[2]
  def getRange(self,index):
    '''returns a list with the range of indexes between minIindex and maxIindex '''
    mn= self.ijkMin[index]
    mx= self.ijkMax[index]+1 #Last index included
    return range(mn,mx)
  def getIRange(self):
    '''returns a list with the range of indexes between minIindex and maxIindex '''
    return self.getRange(0)
  def getJRange(self):
    '''returns a list with the range of index between minJindex and maxJindex '''
    return self.getRange(1)

  def getKRange(self):
    '''returns a list with the range of indexes between minKindex and maxKindex '''
    return self.getRange(2)

  def __str__(self):
    return 'IRange: ' + str(self.getIRange()) + ' JRange: ' + str(self.getJRange()) + ' KRange: ' + str(self.getKRange())


def getLin2Pts(lstLinBusq,tPto1,tPto2):
    #Devuelve la línea que pertenene al conjunto lstLinBusq y tiene
    #como extremos los puntos de tags tPto1 y tPto2
    broke_out= False
    for l in lstLinBusq:
        extr= l.getKPoints()
        if (extr[0]== tPto1 and extr[1]== tPto2) or (extr[0]== tPto2 and extr[1]== tPto1):
            broke_out= True
            break
    if not broke_out:
        print "Didn't find the line"
    else:
        return l

class moveRange(object):
  '''Applies a displacement to a range of grid points
  Attributes:
    range: range of indexes in the grid 
           e.g.: grid.IJKRange([minI,minJ,minK],[maxI,maxJ,maxK])
    vDisp: list of displacements in global X, Y, Z directions
           e.g. [dispX,dispY,dispZ]
  '''
  def __init__(self,range,vDisp):
    self.range= range
    self.vDisp= vDisp

class ijkGrid(object):
  ''' Possible (or allowed) X,Y,Z positions for the model''' 
  def __init__(self,prep,xList,yList,zList):
    self.gridCoo= list()
    self.gridCoo.append(xList)
    self.gridCoo.append(yList)
    self.gridCoo.append(zList)
    self.rangesToMove= list()
    self.prep= prep
    self.indices= self.prep.getCad.get3DNets.new3DNet()
    self.indices.dim(len(self.gridCoo[0]),len(self.gridCoo[1]),len(self.gridCoo[2]))
    self.pointCounter= 1
    self.prep.getCad.getSurfaces.defaultTag= 1

  def lastXIndex(self):
    return len(self.gridCoo[0])-1
  def lastYIndex(self):
    return len(self.gridCoo[1])-1
  def lastZIndex(self):
    return len(self.gridCoo[2])-1

  def generatePoints(self):
    '''Point generation.'''
    points= self.prep.getCad.getPoints
    k= 1;
    for z in self.gridCoo[2]:
      j= 1
      for y in self.gridCoo[1]:
        i= 1
        for x in self.gridCoo[0]:
          pt= points.newPntIDPos3d(self.pointCounter,geom.Pos3d(x,y,z))
          self.indices.setPnt(i,j,k,pt.tag)
          self.pointCounter+=1; i+=1
        j+= 1
      k+= 1
    for rm in self.rangesToMove:
      r= rm.range
      vDisp= rm.vDisp
      for i in r.getIRange():
        for j in r.getJRange():
          for k in r.getKRange():
            tagp= self.getTagIndices(i,j,k)
            pt= points.get(tagp)
            pt.getPos.x+= vDisp[0]
            pt.getPos.y+= vDisp[1]
            pt.getPos.z+= vDisp[2]
             


  def getTagIndices(self,i,j,k):
    'devuelve el tag del punto situado en las posiciones de la rejilla'
    'posIJKrej=[posX,posY,posZ]'
    'el origen de la rejilla está en las posiciones [0,0,0]'
    #tagPto= posIJKrej[0]+1+posIJKrej[1]*len(rejIJK[0])+posIJKrej[2]*len(rejIJK[0])*len(rejIJK[1])
    tagPto= self.indices.getPnt(i+1,j+1,k+1).tag
    return tagPto

  def generateAreas(self,ijkRange,dicGeomEnt):
    'genera las superficies contenidas en un rectángulo comprendido entre las coordenadas'
    'que corresponden a las posiciones en la rejilla ijkRange.ijkMin=[posXmin,posYmin,posZmin] y'
    'ijkRange.ijkMax=[posXmax,posYmax,posZmax]'
    'también rellena el diccionario de superficies'
    retval= list()
    surfaces= self.prep.getCad.getSurfaces
    i= ijkRange.ijkMin[0]
    j= ijkRange.ijkMin[1]
    k= ijkRange.ijkMin[2]
    if ijkRange.ijkMax[2]== ijkRange.ijkMin[2]:
        'superf. contenidas en plano XY'
        while i<= ijkRange.ijkMax[0]-1:
            j= ijkRange.ijkMin[1]
            while j<= ijkRange.ijkMax[1]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i+1,j,k)
                pto3= self.getTagIndices(i+1,j+1,k)
                pto4= self.getTagIndices(i,j+1,k)
                a= surfaces.newQuadSurfacePts(pto1,pto2,pto3,pto4)
                a.nDivI=1     #se inicializa el nº de divisiones a 1 (en otro caso
                a.nDivJ=1     #crea como mínimo 4 divisiones en lados comunes a superficies existentes)
                retval.append(a)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                dicGeomEnt[nmbrSup]= a
                j+=1
            i+=1
    elif ijkRange.ijkMax[1]== ijkRange.ijkMin[1]:
        'superf. contenidas en plano XZ'
        while i<= ijkRange.ijkMax[0]-1:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i,j,k+1)
                pto3= self.getTagIndices(i+1,j,k+1)
                pto4= self.getTagIndices(i+1,j,k)
                a= surfaces.newQuadSurfacePts(pto1,pto2,pto3,pto4)
                a.nDivI=1     #se inicializa el nº de divisiones a 1 (en otro caso
                a.nDivJ=1     #crea como mínimo 4 divisiones en lados comunes a superficies existentes)
                retval.append(a)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                dicGeomEnt[nmbrSup]= a
                k+=1
            i+=1
    elif ijkRange.ijkMax[0]== ijkRange.ijkMin[0]:
        'superf. contenidas en plano YZ'
        while j<= ijkRange.ijkMax[1]-1:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i,j+1,k)
                pto3= self.getTagIndices(i,j+1,k+1)
                pto4= self.getTagIndices(i,j,k+1)
                a= surfaces.newQuadSurfacePts(pto1,pto2,pto3,pto4)
                a.nDivI=1     #se inicializa el nº de divisiones a 1 (en otro caso
                a.nDivJ=1     #crea como mínimo 4 divisiones en lados comunes a superficies existentes)
                retval.append(a)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                dicGeomEnt[nmbrSup]= a
                k+=1
            j+=1
    return retval

  def generateLines(self,ijkRange,dicLin): #lines,rejXYZ,posXYZmin,posXYZmax,dicLin):
    'genera las líneas contenidas en un eje paralelo a uno de los globales, entre las coordenadas'
    'que corresponden a las posiciones en la rejilla posXYZmin=[posXmin,posYmin,posZmin] y'
    'posXYZmax=[posXmax,posYmax,posZmax]'
    'también rellena el diccionario de líneas'
    retval= list()
    lines= self.prep.getCad.getLines
    i=ijkRange.ijkMin[0]
    j=ijkRange.ijkMin[1]
    k=ijkRange.ijkMin[2]
    if ijkRange.ijkMax[1]==ijkRange.ijkMin[1] and ijkRange.ijkMax[2]==ijkRange.ijkMin[2] :
        'línea paralela al eje X'
        while i<=ijkRange.ijkMax[0]-1:
            pto1=self.getTagIndices(i,j,k)
            pto2=self.getTagIndices(i+1,j,k)
            l=lines.newLine(pto1,pto2)
            l.nDiv=1     #se inicializa el nº de divisiones a 1 
            retval.append(l)
            nmbrLin='l'+'%04.0f' % pto1 +'%04.0f' % pto2 
            dicLin[nmbrLin]=l
            i+=1
    elif ijkRange.ijkMax[0]==ijkRange.ijkMin[0] and ijkRange.ijkMax[2]==ijkRange.ijkMin[2] :
        'línea paralela al eje Y'
        while j<=ijkRange.ijkMax[1]-1:
            pto1=self.getTagIndices(i,j,k)
            pto2=self.getTagIndices(i,j+1,k)
            l=lines.newLine(pto1,pto2)
            l.nDiv=1     #se inicializa el nº de divisiones a 1 
            retval.append(l)
            nmbrLin='l'+'%04.0f' % pto1 +'%04.0f' % pto2 
            dicLin[nmbrLin]=l
            j+=1
    elif ijkRange.ijkMax[0]==ijkRange.ijkMin[0] and ijkRange.ijkMax[1]==ijkRange.ijkMin[1] :
        'línea paralela al eje Z'
        while k<=ijkRange.ijkMax[2]-1:
            pto1=self.getTagIndices(i,j,k)
            pto2=self.getTagIndices(i,j,k+1)
            l=lines.newLine(pto1,pto2)
            l.nDiv=1     #se inicializa el nº de divisiones a 1 
            retval.append(l)
            nmbrLin='l'+'%04.0f' % pto1 +'%04.0f' % pto2 
            dicLin[nmbrLin]=l
            k+=1
    return retval

  def getSetInRange(self,ijkRange,dicGeomEnt,nmbrSet):
    'devuelve el set de entidades (superficies y todas las asociadas a estas superficies)'
    'contenidas en un rectángulo comprendido entre las coordenadas'
    'que corresponden a las posiciones en la rejilla ijkRange.ijkMin=[posXmin,posYmin,posZmin] y'
    'ijkRange.ijkMax=[posXmax,posYmax,posZmax]'
    retval= self.prep.getSets.defSet(nmbrSet)
    i= ijkRange.ijkMin[0]
    j= ijkRange.ijkMin[1]
    k= ijkRange.ijkMin[2]
    if ijkRange.ijkMax[2]== ijkRange.ijkMin[2]:
        'superf. contenidas en plano XY'
        while i<= ijkRange.ijkMax[0]-1:
            j= ijkRange.ijkMin[1]
            while j<= ijkRange.ijkMax[1]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i+1,j,k)
                pto3= self.getTagIndices(i+1,j+1,k)
                pto4= self.getTagIndices(i,j+1,k)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                retval.getSurfaces.append(dicGeomEnt[nmbrSup])
                j+=1
            i+=1
    elif ijkRange.ijkMax[1]== ijkRange.ijkMin[1]:
        'superf. contenidas en plano XZ'
        while i<= ijkRange.ijkMax[0]-1:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i,j,k+1)
                pto3= self.getTagIndices(i+1,j,k+1)
                pto4= self.getTagIndices(i+1,j,k)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                retval.getSurfaces.append(dicGeomEnt[nmbrSup])
                k+=1
            i+=1
    elif ijkRange.ijkMax[0]== ijkRange.ijkMin[0]:
        'superf. contenidas en plano YZ'
        while j<= ijkRange.ijkMax[1]-1:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]-1:
                pto1= self.getTagIndices(i,j,k)
                pto2= self.getTagIndices(i,j+1,k)
                pto3= self.getTagIndices(i,j+1,k+1)
                pto4= self.getTagIndices(i,j,k+1)
                nmbrSup='s'+'%04.0f' % pto1 +'%04.0f' % pto2 +'%04.0f' % pto3 +'%04.0f' % pto4
                retval.getSurfaces.append(dicGeomEnt[nmbrSup])
                k+=1
            j+=1
    retval.fillDownwards()    
    return retval

  def getLstLinRange(self,ijkRange):
    'devuelve una lista con las líneas contenidas en el segmento que va' 
    'de ijkRange.ijkMin=[posXmin,posYmin,posZmin]'
    'a ijkRange.ijkMax=[posXmax,posYmax,posZmax]'
    setLinBusq= self.prep.getSets["total"].getLines
    lstLinBusq= setLinBusq
    #lstLinBusq= setLin2lstLin(setLinBusq)
    retval=[]
    i= ijkRange.ijkMin[0]
    j= ijkRange.ijkMin[1]
    k= ijkRange.ijkMin[2]
    tagPto1= self.getTagIndices(i,j,k)
    if ijkRange.ijkMin[1]== ijkRange.ijkMax[1] and ijkRange.ijkMin[2]== ijkRange.ijkMax[2]:
        'línea según el eje X'
        i+=1
        while i<= ijkRange.ijkMax[0]:
            tagPto2= self.getTagIndices(i,j,k)
            l= getLin2Pts(lstLinBusq,tagPto1,tagPto2)
            retval.append(l)
            tagPto1= tagPto2
            i+=1
            #print tagPto1,tagPto2,l.tag
    elif ijkRange.ijkMin[0]== ijkRange.ijkMax[0] and ijkRange.ijkMin[2]== ijkRange.ijkMax[2]:
        'línea según el eje Y'
        j+=1
        while j<= ijkRange.ijkMax[1]:
            tagPto2= self.getTagIndices(i,j,k)
            l= getLin2Pts(lstLinBusq,tagPto1,tagPto2)
            retval.append(l)
            tagPto1= tagPto2
            j+=1
            #print tagPto1,tagPto2,l.tag
    elif ijkRange.ijkMin[0]== ijkRange.ijkMax[0] and ijkRange.ijkMin[1]== ijkRange.ijkMax[1]:
        'línea según el eje Z'
        k+=1
        while k<= ijkRange.ijkMax[2]:
            tagPto2= self.getTagIndices(i,j,k)
            l= getLin2Pts(lstLinBusq,tagPto1,tagPto2)
            retval.append(l)
            tagPto1= tagPto2
            k+=1
            #print tagPto1,tagPto2,l.tag
    return retval

  def getSetPtosRange(self,ijkRange,nombre):
    'devuelve un set de puntos contenidos en un rectángulo comprendido entre las coordenadas'
    'que corresponden a las posiciones en la rejilla ijkRange.ijkMin=[posXmin,posYmin,posZmin] y'
    'ijkRange.ijkMax=[posXmax,posYmax,posZmax]'
    retval= self.prep.getSets.defSet(nombre)
    i= ijkRange.ijkMin[0]
    j= ijkRange.ijkMin[1]
    k= ijkRange.ijkMin[2]
    if ijkRange.ijkMax[2]== ijkRange.ijkMin[2]:
        'superf. contenidas en plano XY'
        while i<= ijkRange.ijkMax[0]:
            j= ijkRange.ijkMin[1]
            while j<= ijkRange.ijkMax[1]:
                tgpto= self.getTagIndices(i,j,k)
                pto= puntos.get(tgpto)
                retval.getPoints.append(pto)
                j+=1
            i+=1
    elif ijkRange.ijkMax[1]== ijkRange.ijkMin[1]:
        'superf. contenidas en plano XZ'
        while i<= ijkRange.ijkMax[0]:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]:
                tgpto= self.getTagIndices(i,j,k)
                pto= puntos.get(tgpto)
                #print pto.tag,pto.getPos.x,pto.getPos.y,pto.getPos.z
                retval.getPoints.append(pto)
                k+=1
            i+=1
    elif ijkRange.ijkMax[0]== ijkRange.ijkMin[0]:
        'superf. contenidas en plano YZ'
        while j<= ijkRange.ijkMax[1]:
            k= ijkRange.ijkMin[2]
            while k<= ijkRange.ijkMax[2]:
              tgpto= self.getTagIndices(i,j,k)
              pto= puntos.get(tgpto)
              #print pto.tag,pto.getPos.x,pto.getPos.y,pto.getPos.z
              retval.getPoints.append(pto)
              k+=1
            j+=1
    return retval

  def applyLoadInRange(self,ijkRange,dicGeomEnt,nmbrSet,loadVector):
    s= self.getSetInRange(ijkRange,dicGeomEnt,nmbrSet)
    sElem=s.getElements
    for e in sElem:
      #print e.tag
      e.vector3dUniformLoadGlobal(loadVector)

  def applyEarthPressure(self,ijkRange,dicGeomEnt,nmbrSet,earthPressLoadressure):
    s= self.getSetInRange(ijkRange,dicGeomEnt,nmbrSet)
    sElem=s.getElements
    for e in sElem:
      zElem=e.getCooCentroid(False)[2]
      presElem= earthPressLoadressure.getPressure(zElem) 
      if(presElem!=0.0):
        #print K, zT, zA, gammaT, gammaA,zElem, presElem
        vectorCarga=presElem*xc.Vector(earthPressLoadressure.vDir)
        e.vector3dUniformLoadGlobal(vectorCarga)
        

def setEntLstSurf(preprocessor,lstSurf,nmbrSet):
  'devuelve el set de las entidades asociadas con las superficies contenidas'
  'en la lista de superficies lstSurf'
  retval= preprocessor.getSets.defSet(nmbrSet)
  for s in lstSurf:
    retval.getSurfaces.append(s)
  retval.fillDownwards()
  return retval



#Funciones geométricas simples

def lstNodesPLine(setBusq,lstPtsPLine):
    #Devuelve los nodos del conjunto "setBusq" que pertenecen a la línea
    #quebrada definida por los puntos de la lista ordenada "lstPts"
    nodAux= setBusq.getNodes
    retval= [] 
    for i in range(0,len(lstPtsPLine)-1):
        segmAux= geom.LineSegment3d(lstPtsPLine[i].getPos,lstPtsPLine[i+1].getPos)
        for n in nodAux:
            p= n.getInitialPos3d
            d= p.distSegmento3d(segmAux)
            if(d<0.01):
                retval.append(n)
    retval= list(set(retval))       #elimina nudos repetidos
    return retval

def setLin2lstLin(setLin):
    #devuelve una lista con las líneas del setLin
    retval=[]
    linAux= setLin.getLines
    for l in linAux:
        retval.append(l)
    return retval
    
def setPnt2lstPnt(setPnt):
    #devuelve una lista con los puntos del setPto
    retval=[]
    pntAux= setPnt.getPoints
    for p in pntAux:
        retval.append(p)
    return retval
 
def setSurf2lstSurf(setSurf):
    #devuelve una lista con las superficies del setSurf
    retval=[]
    surfAux= setSurf.getSurfaces
    for s in surfAux:
        retval.append(s)
    return retval

def setNod2lstNod(setNod):
    #devuelve una lista con los nodos de setNod
    retval=[]
    nodAux= setNod.getNodes
    for n in nodAux:
        retval.append(n)
    return retval

def setElem2lstElem(setElem):
    #devuelve una lista con los elementos en setElem
    retval=[]
    elemAux= setElem.getElements
    for n in elemAux:
        retval.append(n)
    return retval
   
def lstUnionSetsSurf(setSurf1,setSurf2):
    #devuelve una lista de tags de superficies unión de los conjuntos
    #setSurf1 y setSurf2
    retval=[]
    for s in setSurf1.getSufaces:
        retval.append(s.tag)
    for s in setSurf2.getSufaces:
        retval.append(s.tag)
    retval= list(set(retval))       #elimina tags repetidos
    return retval