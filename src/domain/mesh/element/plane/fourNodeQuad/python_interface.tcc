//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
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
//python_interface.tcc

class_<XC::FourNodeQuad, bases<QuadBase4N_Mech2D>, boost::noncopyable >("FourNodeQuad", no_init)
  .add_property("rho", &XC::FourNodeQuad::getRho,&XC::FourNodeQuad::setRho)
  .add_property("thickness", &XC::FourNodeQuad::getThickness,&XC::FourNodeQuad::setThickness)
//.add_property("getAvgStress", make_function(&XC::FourNodeQuad::getAvgStress, return_internal_reference<>()), "Returns average stress in element.")
//.add_property("getAvgStrain", make_function(&XC::FourNodeQuad::getAvgStrain, return_internal_reference<>()), "Returns average strain in element.")
  .def("detJ", &XC::FourNodeQuad::detJ)
   ;
