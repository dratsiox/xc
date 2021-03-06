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
//ConstraintLoader.cc

#include "ConstraintLoader.h"
#include "domain/domain/Domain.h"
#include "domain/constraints/SFreedom_Constraint.h"
#include "domain/constraints/MFreedom_Constraint.h"
#include "domain/constraints/MRMFreedom_Constraint.h"
#include "domain/constraints/EqualDOF.h"
#include "domain/constraints/RigidBeam.h"
#include "domain/constraints/RigidRod.h"
#include "domain/constraints/RigidDiaphragm.h"
#include "domain/constraints/GlueNodeToElement.h"


#include "preprocessor/Preprocessor.h"
#include "domain/mesh/node/Node.h"
#include "domain/mesh/element/Element.h"


//! @brief Default constructor.
XC::ConstraintLoader::ConstraintLoader(Preprocessor *owr)
  : Loader(owr), tag_sp_constraint(0), tag_mp_constraint(0) {}

//! @grief Appends a single freedom constraint to the model.
XC::SFreedom_Constraint *XC::ConstraintLoader::addSFreedom_Constraint(const int &tag_nod,const SFreedom_Constraint &semilla)
  {
    SFreedom_Constraint *sp= semilla.getCopy(tag_sp_constraint);
    tag_sp_constraint++;
    if(sp)
      {
        sp->setNodeTag(tag_nod);
        getDomain()->addSFreedom_Constraint(sp);
        preprocessor->UpdateSets(sp);
      }
    else
      std::cerr << "ConstraintLoader::addSFreedom_Constraint; se produjo un error al crear la coacción." << std::endl;
    return sp;    
  }

//! @grief Appends a single freedom constraint to the model.
XC::SFreedom_Constraint *XC::ConstraintLoader::addSFreedom_Constraint(const int &tag_nod,const int &id_gdl,const double &valor)
  {
    SFreedom_Constraint *sp= new SFreedom_Constraint(tag_sp_constraint,tag_nod,id_gdl, valor);
    tag_sp_constraint++;
    if(sp)
      {
        getDomain()->addSFreedom_Constraint(sp);
        preprocessor->UpdateSets(sp);
      }
    else
      std::cerr << "ConstraintLoader::addSFreedom_Constraint; se produjo un error al crear la coacción." << std::endl;
    return sp;    
  }

//! @grief Defines a single freedom constraint.
XC::SFreedom_Constraint *XC::ConstraintLoader::newSPConstraint(const int &tag_nod,const int &id_gdl,const double &valor)
  { return addSFreedom_Constraint(tag_nod,id_gdl,valor); }

//! @grief Appends a multi-freedom constraint to the model.
XC::MFreedom_Constraint *XC::ConstraintLoader::newMPConstraint(const int &masterNode, const int &slaveNode, const ID &constrainedDOF, const ID &retainedDOF)
  {
    MFreedom_Constraint *mp= new MFreedom_Constraint(tag_mp_constraint,masterNode,slaveNode,constrainedDOF,retainedDOF);
    tag_mp_constraint++;
    if(mp)
      {
        getDomain()->addMFreedom_Constraint(mp);
        preprocessor->UpdateSets(mp);
      }
    else
      std::cerr << "ConstraintLoader::newMPConstraint; se produjo un error al crear la coacción." << std::endl;
    return mp;
  }

//! @brief Imposes the same displacements on both nodes for the components
//! specified in the argument.
//! @param masterNode: tag of the master node.
//! @param slaveNode: tag of the slave node.
//! @param dofs: degrees of freedom to impose the constraint on.
XC::MFreedom_Constraint *XC::ConstraintLoader::newEqualDOF(const int &masterNode, const int &slaveNode, const ID &dofs)
  {
    EqualDOF *mp= new EqualDOF(tag_mp_constraint,masterNode,slaveNode,dofs);
    tag_mp_constraint++;
    if(mp)
      {
        mp->setup(getDomain());
        getDomain()->addMFreedom_Constraint(mp);
        preprocessor->UpdateSets(mp);
      }
    else
      std::cerr << "ConstraintLoader::addMFreedom_Constraint; se produjo un error al crear la coacción." << std::endl;
    return mp;
  }

XC::MFreedom_Constraint *XC::ConstraintLoader::newRigidBeam(const int &masterNode, const int &slaveNode)
  {
    RigidBeam *mp= new RigidBeam(tag_mp_constraint,masterNode,slaveNode);
    tag_mp_constraint++;
    if(mp)
      {
        mp->setup(getDomain());
        getDomain()->addMFreedom_Constraint(mp);
        preprocessor->UpdateSets(mp);
      }
    else
      std::cerr << "ConstraintLoader::newRigidBeam; se produjo un error al crear la coacción." << std::endl;
    return mp;
  }

XC::MFreedom_Constraint *XC::ConstraintLoader::newRigidRod(const int &masterNode, const int &slaveNode)
  {
    RigidRod *mp= new RigidRod(tag_mp_constraint,masterNode,slaveNode);
    tag_mp_constraint++;
    if(mp)
      {
        mp->setup(getDomain());
        getDomain()->addMFreedom_Constraint(mp);
        preprocessor->UpdateSets(mp);
      }
    else
      std::cerr << "ConstraintLoader::newRigidRod; se produjo un error al crear la coacción." << std::endl;
    return mp;
  }

//! @grief Appends a multi-row, multi-freedom constraint to the model.
XC::MRMFreedom_Constraint *XC::ConstraintLoader::newMRMPConstraint(const ID &retainedNodes, const int &constrainedNode, const ID &constrainedDOF)
  {
    MRMFreedom_Constraint *mrmp= new MRMFreedom_Constraint(tag_mrmp_constraint,retainedNodes,constrainedNode,constrainedDOF);
    tag_mrmp_constraint++;
    if(mrmp)
      {
        getDomain()->addMRMFreedom_Constraint(mrmp);
        preprocessor->UpdateSets(mrmp);
      }
    else
      std::cerr << "ConstraintLoader::newMPConstraint; se produjo un error al crear la coacción." << std::endl;
    return mrmp;
  }

XC::MRMFreedom_Constraint *XC::ConstraintLoader::newGlueNodeToElement(const Node &constrainedNode, const Element &e, const ID &constrainedDOF)
  {
    GlueNodeToElement *mrmp= new GlueNodeToElement(tag_mrmp_constraint,constrainedNode,e,constrainedDOF);
    tag_mrmp_constraint++;
    if(mrmp)
      {
        getDomain()->addMRMFreedom_Constraint(mrmp);
        preprocessor->UpdateSets(mrmp);
      }
    else
      std::cerr << "ConstraintLoader::newGlueNodeToElement; se produjo un error al crear la coacción." << std::endl;
    return mrmp;
  }


//! @brief Elimina la coacción del domain.
void XC::ConstraintLoader::removeSPConstraint(const int &tagC)
  {
    bool sp= getDomain()->removeSFreedom_Constraint(tagC);
    if(!sp)
      std::cerr << "ConstraintLoader::removeSPConstraint; se produjo un error al borrar la coacción." << std::endl;
  }

XC::ConstraintLoader::~ConstraintLoader(void)
  { clearAll(); }

//! @brief Clears all the objects.
void XC::ConstraintLoader::clearAll(void)
  {
    tag_sp_constraint= 0;
    tag_mp_constraint= 0;
  }

//! @brief returns number of single node constraints.
int XC::ConstraintLoader::getNumSPs(void) const
  { return getDomain()->getConstraints().getNumSPs(); }

//! @brief returns numbr of multiple node constraints.
int XC::ConstraintLoader::getNumMPs(void) const
  { return getDomain()->getConstraints().getNumMPs(); }

//! @brief returns numbr of multiple retained node constraints.
int XC::ConstraintLoader::getNumMRMPs(void) const
  { return getDomain()->getConstraints().getNumMRMPs(); }

//! @brief returns number of load patterns.
int XC::ConstraintLoader::getNumLPs(void) const
  { return getDomain()->getConstraints().getNumLoadPatterns(); }


