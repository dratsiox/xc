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
//TritrizPtrNod.cc

#include "TritrizPtrNod.h"
#include "domain/mesh/node/Node.h"
#include "xc_basic/src/funciones/algebra/integ_num.h"

#include <boost/any.hpp>
#include "domain/domain/Domain.h"
#include "domain/constraints/SFreedom_Constraint.h"


#include "xc_utils/src/geom/pos_vec/Pos3d.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/geom/d2/Poligono3d.h"

#include "boost/lexical_cast.hpp"

//! @brief Default constructor.
XC::TritrizPtrNod::TritrizPtrNod(const size_t capas)
  : TritrizPtrBase<MatrizPtrNod>(capas) {}
//! @brief Constructor.
XC::TritrizPtrNod::TritrizPtrNod(const size_t capas,const size_t filas,const size_t cols)
  : TritrizPtrBase<MatrizPtrNod>(capas)
  {
    for(size_t i=0;i<capas;i++)
      (*this)[i]= MatrizPtrNod(filas,cols);
  }

//! @brief Returns (if it exists) a pointer to the node
//! cuyo tag is being passed as parameter.
XC::Node *XC::TritrizPtrNod::buscaNodo(const int &tag)
  {
    Node *retval= nullptr;
    const size_t ncapas= GetCapas();
    for(size_t i=1;i<=ncapas;i++)
      {
        MatrizPtrNod &capa= operator()(i);
        retval= capa.buscaNodo(tag);
        if(retval) break;
      }
    return retval;
  }

//! @brief Returns the node closest to the point being passed as parameter.
const XC::Node *XC::TritrizPtrNod::getNearestNode(const Pos3d &p) const
  {
    TritrizPtrNod *this_no_const= const_cast<TritrizPtrNod *>(this);
    return this_no_const->getNearestNode(p);
  }

//! @brief Returns the node closest to the point being passed as parameter.
XC::Node *XC::TritrizPtrNod::getNearestNode(const Pos3d &p)
  {
    Node *retval= nullptr, *ptrNod= nullptr;
    const size_t ncapas= GetCapas();
    double d= DBL_MAX;
    double tmp= 0;
    if(ncapas>100)
      std::clog << "La «tritriz» de nodos es tiene "
                << ncapas << " capas "
                << " es mejor buscar por coordenadas en the set asociado."
                << std::endl;
    for(size_t i=1;i<=ncapas;i++)
      {
        MatrizPtrNod &capa= operator()(i);
        ptrNod= capa.getNearestNode(p);
        tmp= ptrNod->getDist2(p);
        if(tmp<d)
          {
            d= tmp;
            retval= ptrNod;
          }
      }
    return retval;
  }

//! @brief Returns the indexes of the node identified by the pointer
//! being passed as parameter.
XC::ID XC::TritrizPtrNod::getNodeIndices(const Node *n) const
  {
    ID retval(3);
    retval[0]= -1; retval[1]= -1; retval[2]= -1;
    const size_t ncapas= GetCapas();
    const size_t nfilas= getNumFilas();
    const size_t ncols= getNumCols();
    Node *ptrNod= nullptr;
    for(size_t i=1;i<=ncapas;i++)
      for(size_t j=1;j<=nfilas;j++)
        for(size_t k=1;k<=ncols;k++)
          {
            ptrNod= (*this)(i,j,k);
            if(ptrNod==n)
              {
                retval[0]= i; retval[1]= j; retval[2]= k;
                break;
              }
          }
    return retval;
  }

//! @brief Returns (if it exists) a pointer to the node
//! cuyo tag is being passed as parameter.
const XC::Node *XC::TritrizPtrNod::buscaNodo(const int &tag) const
  {
    const Node *retval= nullptr;
    const size_t ncapas= GetCapas();
    for(size_t i=1;i<=ncapas;i++)
      {
        const MatrizPtrNod &capa= operator()(i);
        retval= capa.buscaNodo(tag);
        if(retval) break;
      }
    return retval;
  }



XC::Vector XC::TritrizPtrNod::IntegSimpsonFilaI(const size_t &f,const size_t &c,const ExprAlgebra &e,const size_t &n) const
  {
    const_ref_fila_i fila= GetConstRefFilaI(f,c);
    const std::deque<double> dq_retval= IntegSimpsonFila(fila,"z",2,e,n);
    const size_t szr= dq_retval.size();
    Vector retval(szr);
    for(size_t i= 0;i<szr;i++)
      retval[i]= dq_retval[i];
    return retval;
  }

XC::Vector XC::TritrizPtrNod::IntegSimpsonFilaJ(const size_t &capa, const size_t &c,const ExprAlgebra &e,const size_t &n) const
  {
    const_ref_fila_j fila= GetConstRefFilaJ(capa,c);
    const std::deque<double> dq_retval= IntegSimpsonFila(fila,"x",0,e,n);
    const size_t szr= dq_retval.size();
    Vector retval(szr);
    for(size_t i= 0;i<szr;i++)
      retval[i]= dq_retval[i];
    return retval;
  }

XC::Vector XC::TritrizPtrNod::IntegSimpsonFilaK(const size_t &capa,const size_t &f,const ExprAlgebra &e,const size_t &n) const
  {
    const_ref_fila_k fila= GetConstRefFilaK(capa,f);
    const std::deque<double> dq_retval= IntegSimpsonFila(fila,"y",1,e,n);
    const size_t szr= dq_retval.size();
    Vector retval(szr);
    for(size_t i= 0;i<szr;i++)
      retval[i]= dq_retval[i];
    return retval;
  }

//! @brief Impone desplazamiento nulo en los nodos de this set.
void XC::TritrizPtrNod::fix(const SFreedom_Constraint &spc) const
  {
    if(Null()) return;
    const size_t ncapas= GetCapas();
    for(size_t i=1;i<=ncapas;i++)
      {
        const MatrizPtrNod &capa= operator()(i);
        capa.fix(spc);
      }
  }

std::vector<int> XC::TritrizPtrNod::getTags(void) const
  {
    const size_t ncapas= GetCapas();
    const size_t nfilas= getNumFilas();
    const size_t ncols= getNumCols();
    std::vector<int> retval(ncapas*nfilas*ncols,-1);
    size_t conta= 0;
    for(size_t i=1;i<=ncapas;i++)
      for(size_t j=1;j<=nfilas;j++)
        for(size_t k=1;k<=ncols;k++)
          retval[conta++]= (*this)(i,j,k)->getTag();
    return retval;
  }

void XC::TritrizPtrNod::Print(std::ostream &os) const
  {
    const size_t ncapas= GetCapas();
    const size_t nfilas= getNumFilas();
    const size_t ncols= getNumCols();
    for(size_t i=1;i<=ncapas;i++)
      {
        for(size_t j=1;j<=nfilas;j++)
          {
            for(size_t k=1;k<=ncols;k++)
	      os << (*this)(i,j,k)->getTag() << " ";
	    os << std::endl;
          }
        os << std::endl;
      }
  }

std::ostream &XC::operator<<(std::ostream &os, const TritrizPtrNod &t)
  {
    t.Print(os);
    return os;
  }

void XC::fix(const XC::TritrizPtrNod::var_ref_caja &ref_caja,const XC::SFreedom_Constraint &spc)
  {
    if(ref_caja.Empty()) return;
    const size_t ncapas= ref_caja.GetCapas();
    const size_t nfilas= ref_caja.getNumFilas();
    const size_t ncols= ref_caja.getNumCols();

    // Obtenemos el domain.
    const Node *n= ref_caja(1,1,1);
    Domain *dom= nullptr;
    dom= n->getDomain();
    if(dom)
      {
        for(size_t i=1;i<=ncapas;i++)
          for(size_t j=1;j<=nfilas;j++)
            for(size_t k=1;k<=ncols;k++)
              {
                const Node *nod= ref_caja(i,j,k);
                if(nod)
                  {
                    const int tag_nod= nod->getTag();
                    SFreedom_Constraint *sp= spc.getCopy();
                    sp->setNodeTag(tag_nod); 
                    dom->addSFreedom_Constraint(sp);
                  }
              }
      } 
  }

//! @brief Returns the indexes of the nodes (j,k),(j+1,k),(j+1,k+1),(j,k+1). 
std::vector<int> XC::getIdNodosQuad4N(const XC::TritrizPtrNod::const_ref_capa_i_cte &nodos,const size_t &j,const size_t &k)
  {
    std::vector<int> retval(4,-1);
    const size_t nfilas= nodos.getNumFilas();
    const size_t ncols= nodos.getNumCols();
    if(j>=nfilas)
      {
        std::cerr << "getIdNodosQuad; row index j= " << j << " out of range.\n";
        return retval;
      }
    if(k>=ncols)
      {
        std::cerr << "getIdNodosQuad; column index k= " << k << " out of range.\n";
        return retval;
      }


    Pos3d p1;
    const Node *ptr= nodos(j,k);
    if(ptr)
      {
        retval[0]= ptr->getTag();
        if(retval[0]<0)
          std::cerr << "getIdNodosQuad; error al obtener el identificador de nodo (" << j << ',' << k << ").\n";
        p1= ptr->getPosInicial3d();
      }

    Pos3d p2;
    ptr= nodos(j,k+1);
    if(ptr)
      {
        retval[1]= ptr->getTag();
        if(retval[1]<0)
          std::cerr << "getIdNodosQuad; error al obtener el identificador de nodo (" << j << ',' << k+1 << ").\n";
        p2= ptr->getPosInicial3d();
      }

    Pos3d p3;
    ptr= nodos(j+1,k+1);
    if(ptr)
      {
        retval[2]= ptr->getTag();
        if(retval[2]<0)
          std::cerr << "getIdNodosQuad; error al obtener el identificador de nodo (" << j+1 << ',' << k+1 << ").\n";
        p3= ptr->getPosInicial3d();
      }

    Pos3d p4;
    ptr= nodos(j+1,k);
    if(ptr)
      {
        retval[3]=ptr->getTag();
        if(retval[3]<0)
          std::cerr << "getIdNodosQuad; error al obtener el identificador de nodo (" << j+1 << ',' << k << ").\n";
        p4= ptr->getPosInicial3d();
      }

//     const Vector3d v2= p2-p1;
//     const Vector3d v3= p3-p2;
//     const Vector3d v4= p4-p3;
//     const Vector3d v1= p1-p4;
//     const double d1= dot(v1,v3);
//     const double d2= dot(v2,v4);
//     if((d1>0))
//       {
//         std::swap(p3,p2);
//         std::swap(retval[2],retval[1]);
//       }
//     if((d2>0))
//       {
// 	std::swap(p3,p4);
//         std::swap(retval[2],retval[3]);
//       }

    std::list<Pos3d> posiciones;
    posiciones.push_back(p1);
    posiciones.push_back(p2);
    posiciones.push_back(p3);
    posiciones.push_back(p4);
    Poligono3d tmp(posiciones.begin(),posiciones.end());
    const double area= tmp.Area();
    if(area<1e-3)
      {
        std::cerr << "Area for (" << j << ',' << k
                  << ") cell is too small (" << area << ").\n";
        std::cerr << " position of the node (j,k) " << p1 << std::endl;
	std::cerr << " position of the node (j+1,k) " << p2 << std::endl;
	std::cerr << " position of the node (j+1,k+1) " << p3 << std::endl;
	std::cerr << " position of the node (1,k+1) " << p4 << std::endl;
      }
    return retval;
  }

//! @brief Returns the indexes of the nodes (j,k),(j+1,k),(j+1,k+1),(j,k+1). 
std::vector<int> XC::getIdNodosQuad9N(const XC::TritrizPtrNod::const_ref_capa_i_cte &nodos,const size_t &j,const size_t &k)
  {
    std::vector<int> retval(9,-1);
    std::cerr << "getIdNodosQuad9N not implemented." << std::endl;
    return retval;
  }
