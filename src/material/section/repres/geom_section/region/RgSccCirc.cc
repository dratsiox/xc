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
//RgSccCirc.cpp
// Written by Remo M. de Souza
// December 1998

#include <cmath>
#include <utility/matrix/Matrix.h>
#include <material/section/repres/geom_section/region/RgSccCirc.h>
#include <material/section/repres/cell/QuadCell.h>

#include "xc_basic/src/texto/cadena_carac.h"
#include "xc_utils/src/geom/d2/poligonos2d/Poligono2d.h"
#include "xc_utils/src/geom/d2/SectorAnilloCircular2d.h"
#include "xc_utils/src/geom/d2/Rejilla2d.h"

const int I= 0, J=1, K= 2, L= 3; //Index of vertices.
const int Y= 0, Z=1; //Index of Y and Z components.

//! @brief Default constructor.
XC::RgSccCirc::RgSccCirc(Material *mat)
  : RgQuadCell(mat), centerPosit(2),intRad(0.0), extRad(0.0), initAng(0.0), finalAng(360.0)
  {}


XC::RgSccCirc::RgSccCirc(Material *mat, int numSubdivCircunf, int numSubdivRadial,
                     const XC::Vector &centerPosition, double internRadius, 
                     double externRadius, double initialAngle, double finalAngle)
  : RgQuadCell(mat,numSubdivCircunf,numSubdivRadial),
    centerPosit(centerPosition), intRad(internRadius), extRad(externRadius), 
    initAng(initialAngle), finalAng(finalAngle)
  {}

void XC::RgSccCirc::setCenterPosition(const Vector &centerPosition)
  { centerPosit = centerPosition; }

void XC::RgSccCirc::setCenterPos(const Pos2d &p)
  {
    centerPosit(0)= p.x();
    centerPosit(1)= p.y();
  }

Pos2d XC::RgSccCirc::getCenterPos(void) const
  { return Pos2d(centerPosit(0),centerPosit(1)); }

void XC::RgSccCirc::setRadii(double internRadius, double externRadius)
  {
    intRad = internRadius;
    extRad = externRadius;
  }

void XC::RgSccCirc::setAngles(double initialAngle, double finalAngle)
  {
    initAng  = initialAngle;
    finalAng = finalAngle;
  }

void XC::RgSccCirc::getRadii(double &internRadius, double &externRadius) const
  {
    internRadius = intRad;
    externRadius = extRad;
  }

void XC::RgSccCirc::getAngles(double &initialAngle, double &finalAngle) const
  {
    initialAngle = initAng;
    finalAngle   = finalAng;
  }

const XC::Matrix &XC::RgSccCirc::getVertCoords(void) const
  {
    static Matrix vertCoord(4,2);
    const double cosI= cos(initAng);
    const double cosF= cos(finalAng);
    const double sinI= sin(initAng);
    const double sinF= sin(finalAng);
    vertCoord(I,Y)= centerPosit(0) + intRad*cosI;
    vertCoord(I,Z)= centerPosit(1) + intRad*sinI;
    vertCoord(J,Y)= centerPosit(0) + extRad*cosI;
    vertCoord(J,Z)= centerPosit(1) + extRad*sinI;
    vertCoord(K,Y)= centerPosit(0) + extRad*cosF;
    vertCoord(K,Z)= centerPosit(1) + extRad*sinF;
    vertCoord(L,Y)= centerPosit(0) + intRad*cosF;
    vertCoord(L,Z)= centerPosit(1) + intRad*sinF;
    return vertCoord;
  }

double XC::RgSccCirc::getMaxY(void) const
  {
    std::cerr << "RgSccCirc::getMaxY not implemented." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMaxZ(void) const
  {
    std::cerr << "RgSccCirc::getMaxZ not implemented." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMinY(void) const
  {
    std::cerr << "RgSccCirc::getMinY not implemented." << std::endl;
    return 0.0;
  }
double XC::RgSccCirc::getMinZ(void) const
  {
    std::cerr << "RgSccCirc::getMinZ not implemented." << std::endl;
    return 0.0;
  }

const XC::Vector &XC::RgSccCirc::getCenterPosition(void) const
  { return centerPosit; }

//! @brief Returns a poligono inscrito en el sector del anillo circular.
Poligono2d XC::RgSccCirc::getPoligono(void) const
  { return getSector().getPoligono2d(nDivCirc()); }

SectorAnilloCircular2d &XC::RgSccCirc::getSector(void) const
  {
    static SectorAnilloCircular2d retval;
    Pos2d O(centerPosit(0),centerPosit(1));
    Circulo2d c(O,extRad);
    SectorCircular2d sc(c,initAng,finalAng);
    retval= SectorAnilloCircular2d(sc,intRad);
    return retval;
  }

//! @brief Return the discretization grid.
const Rejilla2d &XC::RgSccCirc::getMesh(void) const
  { return alloc(Rejilla2d(getSector().genMesh(nDivRad(),nDivCirc()))); }

const XC::VectorCells &XC::RgSccCirc::getCells(void) const
  {
    if(nDivRad() > 0  && nDivCirc() > 0)
      {
        getMesh();
        int numCells  = this->getNumCells();

        cells.resize(numCells);

        int k= 0;
        XC::Matrix cellVertCoord(4,2);
        for(int j= 1;j<nDivRad()+1;j++)
          {
            for(int i= 1;i<nDivCirc()+1;i++)
              {
                cellVertCoord= getCellVertCoords(i,j);   //centerPosit(0) + rad_j  * cosTh1;
                cells.put(k,QuadCell(cellVertCoord)); 
                //std::cerr << "\ncreating cells XC::Cell " << k << " :" << cells[k];
                k++; 
              }
           }
       }
     return cells;
  }

//! @brief Virtual constructor.
XC::RegionSecc *XC::RgSccCirc::getCopy(void) const
  { return new XC::RgSccCirc(*this); }
 
void XC::RgSccCirc::Print(std::ostream &s, int flag) const
  {
    s << "\nRgQuadCell Type: RgSccCirc";
    //s << "\nMaterial Id: " << getMaterialID();
    s << "\nNumber of subdivisions in the radial direction: " << nDivRad();
    s << "\nNumber of subdivisions in the circunferential direction: " << nDivCirc();
    s << "\nCenter Position: " << centerPosit;
    s << "\nInternal Radius: " << intRad << "\tExternal Radius: " << extRad;
    s << "\nInitial Angle: " << initAng << "\tFinal Angle: " << finalAng;
  }

std::ostream &XC::operator<<(std::ostream &s, XC::RgSccCirc &RgSccCirc)
  {
    RgSccCirc.Print(s);
    return s;
  }
