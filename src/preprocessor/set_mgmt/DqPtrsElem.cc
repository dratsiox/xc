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
//  Este software se distribuye con la esperanza de que sea útil pero 
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details. 
//
//
// You should have received a copy of the GNU General Public License 
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------

#include "DqPtrsElem.h"
#include "domain/mesh/element/Element.h"
#include "preprocessor/cad/trf/TrfGeom.h"
#include "xc_utils/src/geom/d1/Polilinea3d.h"
#include "domain/mesh/node/Node.h"
#include "domain/mesh/MeshEdges.h"

void XC::DqPtrsElem::create_arbol(void)
  {
    kdtreeElements.clear();
    for(iterator i= begin();i!=end();i++)
      {
        Element *nPtr= *i;
        assert(nPtr);
        kdtreeElements.insert(*nPtr);
      }
  }

//! @brief Constructor.
XC::DqPtrsElem::DqPtrsElem(EntCmd *owr)
  : DqPtrs<Element>(owr) {}

//! @brief Copy constructor.
XC::DqPtrsElem::DqPtrsElem(const DqPtrsElem &otro)
  : DqPtrs<Element>(otro)
  { create_arbol(); }

//! @brief Copy constructor.
XC::DqPtrsElem::DqPtrsElem(const std::deque<Element *> &ts)
  : DqPtrs<Element>(ts)
  { create_arbol(); }

//! @brief Copy constructor.
XC::DqPtrsElem::DqPtrsElem(const std::set<const Element *> &st)
  : DqPtrs<Element>()
  {
    std::set<const Element *>::const_iterator k;
    k= st.begin();
    for(;k!=st.end();k++)
      push_back(const_cast<Element *>(*k));
  }

//! @brief Assignment operator.
XC::DqPtrsElem &XC::DqPtrsElem::operator=(const DqPtrsElem &otro)
  {
    DqPtrs<Element>::operator=(otro);
    kdtreeElements= otro.kdtreeElements;
    return *this;
  }

//! @brief Extend this list with the elements of the container
//! being passed as parameter.
void XC::DqPtrsElem::extend(const DqPtrsElem &otro)
  {
    for(register const_iterator i= otro.begin();i!=otro.end();i++)
      push_back(*i);
  }

// //! @brief Extend this list with the elements of the container
// //! being passed as parameter that fulfill the condition.
// void XC::DqPtrsElem::extend_cond(const DqPtrsElem &otro,const std::string &cond)
//   {
//     bool result= false;
//     for(register const_iterator i= otro.begin();i!=otro.end();i++)
//       {
//         result= (*i)->interpretaBool(cond);
//         if(result)
// 	  push_back(*i);
//       }
//   }

//! @brief Clears out the list of pointers and erases the properties of the object (if any).
void XC::DqPtrsElem::clearAll(void)
  {
    DqPtrs<Element>::clear();
    kdtreeElements.clear();
  }

bool XC::DqPtrsElem::push_back(Element *e)
  {
    bool retval= DqPtrs<Element>::push_back(e);
    if(retval)
      kdtreeElements.insert(*e);
    return retval;
  }

bool XC::DqPtrsElem::push_front(Element *e)
  {
    bool retval= DqPtrs<Element>::push_front(e);
    if(retval)
      kdtreeElements.insert(*e);
    return retval;
  }

//! @brief Returns the element closest to the point being passed as parameter.
XC::Element *XC::DqPtrsElem::getNearestElement(const Pos3d &p)
  {
    Element *retval= const_cast<Element *>(kdtreeElements.getNearestElement(p));
    return retval;
  }

//! @brief Returns the element closest to the point being passed as parameter.
const XC::Element *XC::DqPtrsElem::getNearestElement(const Pos3d &p) const
  {
    DqPtrsElem *this_no_const= const_cast<DqPtrsElem *>(this);
    return this_no_const->getNearestElement(p);
  }

//! @brief Returns (if it exists) a pointer to the element
//! identified by the tag being passed as parameter.
XC::Element *XC::DqPtrsElem::findElement(const int &tag)
  {
    Element *retval= nullptr;
    Element *tmp= nullptr;
    for(iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          {
            if(tag == tmp->getTag())
              {
                retval= tmp;
                break;
              }
          }
      }
    return retval;
  }

//! @brief Returns (if it exists) a pointer to the element
//! identified by the tag being passed as parameter.
const XC::Element *XC::DqPtrsElem::findElement(const int &tag) const
  {
    const Element *retval= nullptr;
    const Element *tmp= nullptr;
    for(const_iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          {
            if(tag == tmp->getTag())
              {
                retval= tmp;
                break;
              }
          }
      }
    return retval;
  }

//! @brief Returns the number of elements of the set which are active.
size_t XC::DqPtrsElem::getNumLiveElements(void) const
  {
    size_t retval= 0;
    const Element *tmp= nullptr;
    for(const_iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          if(tmp->isAlive()) retval++;
      }
    return retval;
  }

//! @brief Returns the number of elements of the set which are inactive.
size_t XC::DqPtrsElem::getNumDeadElements(void) const
  {
    size_t retval= 0;
    const Element *tmp= nullptr;
    for(const_iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          if(tmp->isDead()) retval++;
      }
    return retval;
  }

//!  @brief Set indices to the objects to allow its use in VTK.
void XC::DqPtrsElem::numera(void)
  {
    size_t idx= 0;
    for(iterator i= begin();i!=end();i++,idx++)
      {
	Element *ptr= *i;
        ptr->set_indice(idx);
      }
  }

//! @brief Deactivates the elements.
void XC::DqPtrsElem::kill_elements(void)
  {
    Element *tmp= nullptr;
    for(iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          tmp->kill();
      }
  }

//! @brief Activates the elements.
void XC::DqPtrsElem::alive_elements(void)
  {
    Element *tmp= nullptr;
    for(iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          tmp->alive();
      }
  }

//! @brief Calcula los esfuerzos en cada uno of the elements.
void XC::DqPtrsElem::calc_resisting_force(void)
  {
    Element *tmp= nullptr;
    for(iterator i= begin();i!=end();i++)
      {
        tmp= *i;
        if(tmp)
          tmp->getResistingForce();
      }
  }

//! @brief Returns the tags of the elements.
std::set<int> XC::DqPtrsElem::getTags(void) const
  {
    std::set<int> retval;
    for(const_iterator i= begin();i!=end();i++)
      retval.insert((*i)->getTag());
    return retval;
  }

//! @brief Returns the element set contour.
std::deque<Polilinea3d> XC::DqPtrsElem::getContours(bool undeformedGeometry) const
  {
    typedef std::set<const Element *> ElementConstPtrSet;
    const Element *elem= nullptr;
    MeshEdges edgesContour;
    for(const_iterator i= begin();i!=end();i++)
      {
        elem= *i;
        const int numEdges= elem->getNumEdges();
        for(int j= 0;j<numEdges;j++)
          {
	    MeshEdge meshEdge(elem->getNodesEdge(j));
            ElementConstPtrSet elementsShared= meshEdge.getConnectedElements(*this);
            if(elementsShared.size()==1) //border element.
              if(find(edgesContour.begin(), edgesContour.end(), meshEdge) == edgesContour.end())
                { edgesContour.push_back(meshEdge); }
          }
      }
    return edgesContour.getContours(undeformedGeometry);
  }
