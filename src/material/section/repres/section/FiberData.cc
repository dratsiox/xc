//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  This program derives from OpenSees <http://opensees.berkeley.edu>
//  developed by the  «Pacific earthquake engineering research center».
//
//  Except for the restrictions that may arise from the copyright
//  of the original program (see copyright_opensees.txt)
//  XC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or 
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful, but 
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details. 
//
//
// You should have received a copy of the GNU General Public License 
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//FiberData.cc

#include <material/section/repres/section/FiberData.h>
#include <material/section/repres/cell/VectorCells.h>
#include <material/section/repres/cell/Cell.h>
#include "material/section/repres/geom_section/region/ListRegiones.h"
#include <material/section/repres/geom_section/region/RegionSecc.h>
#include <material/section/repres/geom_section/reinfBar/VectorReinfBar.h>
#include <material/section/repres/geom_section/reinfBar/ReinfBar.h>
#include "material/section/repres/geom_section/reinfLayer/ReinfLayer.h"
#include "material/section/repres/geom_section/reinfLayer/ListReinfLayer.h"
#include "material/uniaxial/UniaxialMaterial.h"
#include "material/section/fiber_section/fiber/UniaxialFiber2d.h"
#include "material/section/fiber_section/fiber/UniaxialFiber3d.h"
#include "material/section/repres/geom_section/GeomSection.h"


//! @brief Constructor.
XC::FiberData::FiberData(const int &sz)
  : fibersMaterial(sz),fibersPosition(2,sz),fibersArea(sz) {}

//! @brief Constructor.
XC::FiberData::FiberData(const GeomSection &gmSecc)
  : fibersMaterial(gmSecc.getNumFiberData()),fibersPosition(2,gmSecc.getNumFiberData()),fibersArea(gmSecc.getNumFiberData())
  {
    int k= PutCells(0,gmSecc.getRegiones());
    k= PutCapasArmadura(k,gmSecc.getCapasArmadura());
  }

//! @brief Coloca las celdas being passed as parameters.
size_t XC::FiberData::put_cells(const size_t &offset,const VectorCells &cells,Material *matPtr)
  {
    const size_t numCells= cells.size();
    size_t k= offset;
    static Vector fiberPosition(2); //!< Position of the fiber.

    for(size_t j= 0;j<numCells;j++)
      {
        setPtrMaterial(k,matPtr);
        Area(k)= cells[j]->getArea();
        fiberPosition= cells[j]->getCentroidPosition();

        Position(0,k)= fiberPosition(0);
        Position(1,k)= fiberPosition(1);
        k++;
      }
    //cells.libera();
    return k;
  }

//! @brief Coloca las barras being passed as parameters.
size_t XC::FiberData::put_reinf_bars(const size_t &offset,const VectorReinfBar &bars,Material *matPtr)
  {
    int k= offset;
    const size_t sz= bars.size();
    static Vector fiberPosition(2); //Position of the bars.

    for(size_t j= 0;j<sz;j++)
      {
        setPtrMaterial(k,matPtr);
        Area(k)= bars[j]->getArea();
        fiberPosition= bars[j]->getPosition();

        Position(0,k) = fiberPosition(0);
        Position(1,k) = fiberPosition(1);
        k++;
      }
    //bars.libera();
    return k;
  }

//! @brief Coloca las celdas de las regiones being passed as parameters.
size_t XC::FiberData::PutCells(const size_t &offset,const ListRegiones &regiones)
  {
    Material *matPtr= nullptr;
    int k= offset;
    for(XC::ListRegiones::const_iterator i= regiones.begin();i!=regiones.end();i++)
      {
        matPtr= (*i)->getMaterialPtr();
        const VectorCells &cells= (*i)->getCells();
        k= put_cells(k,cells,matPtr);
      }
    return k;
  }

//! @brief Coloca las barras de la lista being passed as parameter.
size_t XC::FiberData::PutCapasArmadura(const size_t &offset,const ListReinfLayer &capas_armado)
  {
    Material *matPtr= nullptr;
    int k= offset;
    for(ListReinfLayer::const_iterator i= capas_armado.begin();i!=capas_armado.end();i++)
      {
        matPtr= (*i)->getMaterialPtr();
        const ReinfLayer *capa= *i;
        const VectorReinfBar &bars= capa->getReinfBars();
        k= put_reinf_bars(k,bars,matPtr);
      }
    return k;
  }

void XC::FiberData::getFibras2d(contenedor_fibras &retval) const
  {
    const int sz= size();
    UniaxialMaterial *material= nullptr;
    Material *tmp_material= nullptr;
    UniaxialFiber2d *tmp_fiber= nullptr;
    for(int k=0;k<sz;k++)
      {
        tmp_material= getPtrMaterial(k);
        if(tmp_material)
          {
            material = dynamic_cast<UniaxialMaterial *>(tmp_material);
            if(!material)
              {
                std::cerr <<  "WARNING invalid material ID: " << tmp_material->getTag()
                          << " for region.\n";
                return;
              }

            if(Area(k)<=0)
              std::cerr << "ERROR, zero or negative area for the fiber in position: (" 
                        << Position(0,k) << "," << 0.0 << ")." << std::endl;
            tmp_fiber = new UniaxialFiber2d(k,*material,Area(k),Position(0,k));
            if(!tmp_fiber)
              {
                std::cerr <<  "WARNING unable to allocate fiber: " << k << std::endl;
                return;
              }
            else
              retval.push_back(tmp_fiber);
          }
        else
	  std::cerr << "FiberData::getFibras2d; material for fiber: " << k << " is not defined." << std::endl;
      }
  }

void XC::FiberData::getFibras3d(contenedor_fibras &retval) const
  {
    UniaxialMaterial *material= nullptr;
    Material *tmp_material= nullptr;
    UniaxialFiber3d *tmp_fiber= nullptr;
    static Vector fiberPosition(2);
    const int sz= size();
    for(int k= 0;k<sz;k++)
      {
        tmp_material= getPtrMaterial(k);
        if(tmp_material)
          {
            material = dynamic_cast<UniaxialMaterial *>(tmp_material);
            if(!material)
              {
                std::cerr <<  "WARNING invalid material ID: " << tmp_material->getTag() 
                          << "for region.\n";
                return;
              }
            fiberPosition(0)= Position(0,k);
            fiberPosition(1)= Position(1,k);
            if(Area(k)<=0)
              std::cerr << "ERROR, zero or negative area for the fiber in position: (" 
                        << fiberPosition(0) << "," << fiberPosition(1) << ")." << std::endl;
            tmp_fiber= new UniaxialFiber3d(k, *material, Area(k), fiberPosition);
            if(!tmp_fiber)
              {
                std::cerr <<  "WARNING unable to allocate fiber: " << k << std::endl;
                return;
              }
            else
              retval.push_back(tmp_fiber);
          }
        else
	  std::cerr << "FiberData::getFibras3d; material for fiber: " << k << " is not defined." << std::endl;
        //std::cerr << *retval[k];
      }
  }
