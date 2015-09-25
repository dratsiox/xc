# -*- coding: utf-8 -*-
# Comprobación de secciones de hormigón frente a fisuración.

from materials.ehe import fisuracionEHE
from materials.sia262 import crackControlSIA262 as ccSIA
from materials.xLamina import extrae_combinaciones as ec
from materials.xLamina import calculo_comb
import os
from postprocess.reports import common_formats as fmt


def lanzaCalculoFIS(nmbArch, nmbRegDatosScc1, nmbRegDatosScc2, nmbArchDefHipELS):
  '''
   Lanza la comprobación de fisuración en una lámina
      cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
      con los materiales que se definen en el archivo nmbArchMateriales,
      las características de las secciones que se definen en los registros
      datosScc1 y datosScc2, las combinaciones definidas en el archivo
      nmbArchDefHipELS e imprime los resultados en archivos con
      el nombre nmbArchFis.*
  '''
  extraeDatosLST(nmbArch+".lst")
  xLaminaConstruyeModeloFibras(nmbRegDatosScc1,nmbRegDatosScc2)
  xLaminaCalculaCombEstatNoLin(nmbArchDefHipELS)
  xLaminaPrintFIS(nmbArch+"FIS",deref(nmbRegDatosScc1).nmbSeccion,deref(nmbRegDatosScc2).nmbSeccion)

def xLaminaPrintFIS(nmbArchSalida, nmbSeccion1, nmbSeccion2):
  # Imprime los resultados de la comprobación frente a fisuración
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  ansysOutput1= open(nmbArchSalida+".mac","w")
  ansysOutput2= open(nmbArchSalida+"esf.mac","w")
  printCabeceraListadoFisuracion(texOutput1,"1 ("+ nmbSeccion1 +")")
  printCabeceraListadoFisuracion(texOutput2,"2 ("+ nmbSeccion2 +")")
  e= mdlr.getElementLoader
  for e in elementos:
    if(odd(e.tag)):
      texOutput1.write(floor(e,tag/10)+' & '+e,HIPCP+' & '+fmt.Esf.format(NCP/1e3)+' & '+fmt.Esf.format(MyCP/1e3)+' & '+fmt.Esf.format(MzCP/1e3)+' & '+fmt.Long.format(WkCP*1e3,'{:3.2f}'),"\\\\\n")
      ansysOutput1.write("detab,"+floor(tag/10)+',WK1,'+WkCP*1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',N1,'+NCP/1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',My1,'+MyCP/1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',Mz1,'+MzCP/1e3+'\n')
    else:
      texOutput2.write(floor(tag/10)+' & '+HIPCP+' & '+'{:5.2f}'.format(N/1e3)+' & '+'{:5.2f}'.format(MyCP/1e3)+' & '+'{:5.2f}'.format(MzCP/1e3)+' & '+'{:3.2f}'.format(WkCP*1e3),"\\\\\n")
      ansysOutput1.write("detab,"+floor(tag/10)+',WK2,'+WkCP*1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',N2,'+NCP/1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',My2,'+MyCP/1e3+'\n')
      ansysOutput2.write("detab,"+floor(tag/10)+',Mz2,'+MzCP/1e3+'\n')
  printCierreListadoFisuracion("texOutput1")
  printCierreListadoFisuracion("texOutput2")
  texOutput1.close()
  texOutput2.close()
  ansysOutput1.close()
  ansysOutput2.close()
    
  os.system("cat /tmp/texOutput1.tmp /tmp/texOutput2.tmp > "+nmbArchSalida+".tex")
    
  os.sys("rm -f "+"/tmp/acciones.xci")
  os.sys("rm -f "+"/tmp/cargas.xci")
  os.sys("rm -f "+"/tmp/elementos.xci")
  os.sys("rm -f "+"/tmp/texOutput1.tmp")
  os.sys("rm -f "+"/tmp/texOutput2.tmp")

def lanzaCalculoFISFromXCData(mdlr,analysis,nmbArchCsv,nmbArchSalida, mapSectionsForEveryElement,trataResultsCombFIS):
  '''
   Lanza la comprobación de fisuración en una lámina
      cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
      con los materiales que se definen en el archivo nmbArchMateriales,
      las características de las secciones que se definen en mapSections,
      e imprime los resultados en archivos con
      el nombre nmbArchFis.*
  '''
  elems= ec.creaElems(mdlr,nmbArchCsv, mapSectionsForEveryElement)
  ccSIA.defVarsControlFISSIA262(elems)
  calculo_comb.xLaminaCalculaComb(mdlr,analysis,trataResultsCombFIS)
  xLaminaPrintFISSIA262(mdlr,nmbArchSalida,mapSectionsForEveryElement)

def lanzaCalculoFISFromXCDataPlanB(mdlr,analysis,nmbArchCsv,nmbArchSalida, mapSectionsForEveryElement,mapSectionsDefinition,trataResultsCombFIS):
  '''
   Lanza la comprobación de fisuración en una lámina
      cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
      con los materiales que se definen en el archivo nmbArchMateriales,
      las características de las secciones que se definen en mapSections,
      e imprime los resultados en archivos con
      el nombre nmbArchFis.*
  '''
  elems= ec.extraeDatos(mdlr,nmbArchCsv, mapSectionsForEveryElement,mapSectionsDefinition, None)
  ccSIA.defVarsControlFISSIA262(elems)
  calculo_comb.xLaminaCalculaComb(mdlr,analysis,trataResultsCombFIS)
  xLaminaPrintFISSIA262(mdlr,nmbArchSalida,mapSectionsForEveryElement)

def strElementProp(eTag,nmbProp,vProp):
  retval= "mdlr.getElementLoader.getElement("
  retval+= str(eTag)
  retval+= ").setProp("
  retval+= '"' + nmbProp + '"'
  retval+= ',' + str(vProp) + ")\n"
  return retval

def xLaminaPrintFISSIA262(mdlr,nmbArchSalida, mapSections):
  # Imprime los resultados de la comprobación frente a fisuración
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  xcOutput= open(nmbArchSalida+".py","w")
  #printCabeceraListadoFisuracion("texOutput1","1 ("+ nmbSeccion1 +")")
  #printCabeceraListadoFisuracion("texOutput2","2 ("+ nmbSeccion2 +")")
  elementos= mdlr.getSets.getSet("total").getElements
  strHeader= "eTag & idSection & HIPCP & NCP kN & MyCP kN m/m & MzCP kN m/m & $sg_{max} MPa \\\\\n"
  texOutput1.write(strHeader)
  texOutput2.write(strHeader)
  for e in elementos:
    tag= e.getProp("idElem")
    idSection= e.getProp("idSection")
    HIPCPPos= e.getProp("HIPCPPos")
    NCPPos= e.getProp("NCPPos")
    MyCPPos= e.getProp("MyCPPos")
    MzCPPos= e.getProp("MzCPPos")
    sg_sPos= e.getProp("sg_sPos")
    HIPCPNeg= e.getProp("HIPCPNeg")
    NCPNeg= e.getProp("NCPNeg")
    MyCPNeg= e.getProp("MyCPNeg")
    MzCPNeg= e.getProp("MzCPNeg")
    sg_sNeg= e.getProp("sg_sNeg")
    strOutPos= str(tag)+" & " +idSection + " & + & "+HIPCPPos+' & '+fmt.Esf.format(NCPPos/1e3)+' & '+fmt.Esf.format(MyCPPos/1e3)+' & '+fmt.Esf.format(MzCPPos/1e3)+' & '+fmt.Stress.format(sg_sPos/1e6)+"\\\\\n"
    strOutNeg= str(tag)+" & " +idSection + " & - & "+HIPCPNeg+' & '+fmt.Esf.format(NCPNeg/1e3)+' & '+fmt.Esf.format(MyCPNeg/1e3)+' & '+fmt.Esf.format(MzCPNeg/1e3)+' & '+fmt.Stress.format(sg_sNeg/1e6)+"\\\\\n"
    if(e.getProp("dir")==1):
      texOutput1.write(strOutPos)
      texOutput1.write(strOutNeg)
      xcOutput.write(strElementProp(tag,"sg_sPos1",sg_sPos/1e6))
      xcOutput.write(strElementProp(tag,"NCPPos1",NCPPos/1e3))
      xcOutput.write(strElementProp(tag,"MyCPPos1",MyCPPos/1e3))
      xcOutput.write(strElementProp(tag,"MzCPPos1",MzCPPos/1e3))
      xcOutput.write(strElementProp(tag,"sg_sNeg1",sg_sNeg/1e6))
      xcOutput.write(strElementProp(tag,"NCPNeg1",NCPNeg/1e3))
      xcOutput.write(strElementProp(tag,"MyCPNeg1",MyCPNeg/1e3))
      xcOutput.write(strElementProp(tag,"MzCPNeg1",MzCPNeg/1e3))
    else:
      texOutput2.write(strOutPos)
      texOutput2.write(strOutNeg)
      xcOutput.write(strElementProp(tag,"sg_sPos2",sg_sPos/1e6))
      xcOutput.write(strElementProp(tag,"NCPPos2",NCPPos/1e3))
      xcOutput.write(strElementProp(tag,"MyCPPos2",MyCPPos/1e3))
      xcOutput.write(strElementProp(tag,"MzCPPos2",MzCPPos/1e3))
      xcOutput.write(strElementProp(tag,"sg_sNeg2",sg_sNeg/1e6))
      xcOutput.write(strElementProp(tag,"NCPNeg2",NCPNeg/1e3))
      xcOutput.write(strElementProp(tag,"MyCPNeg2",MyCPNeg/1e3))
      xcOutput.write(strElementProp(tag,"MzCPNeg2",MzCPNeg/1e3))
  #printCierreListadoFisuracion("texOutput1")
  #printCierreListadoFisuracion("texOutput2")
  texOutput1.close()
  texOutput2.close()
  xcOutput.close()
    
  os.system("cat /tmp/texOutput1.tmp /tmp/texOutput2.tmp > "+nmbArchSalida+".tex")
    
  # os.system("rm -f "+"/tmp/acciones.xci")
  # os.system("rm -f "+"/tmp/cargas.xci")
  # os.system("rm -f "+"/tmp/elementos.xci")
  os.system("rm -f "+"/tmp/texOutput1.tmp")
  os.system("rm -f "+"/tmp/texOutput2.tmp")

