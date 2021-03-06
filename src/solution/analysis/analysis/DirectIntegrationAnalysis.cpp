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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.8 $
// $Date: 2005/11/29 23:36:47 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/analysis/DirectIntegrationAnalysis.cpp,v $
                                                                        
                                                                        
// Written: fmk 
// Created: 11/96
// Revision: A
//
// Description: This file contains the implementation of the
// DirectIntegrationAnalysis class.
//
// What: "@(#) DirectIntegrationAnalysis.C, revA"


#include <solution/analysis/analysis/DirectIntegrationAnalysis.h>
#include "solution/analysis/ModelWrapper.h"
#include "solution/SoluMethod.h"
#include <solution/analysis/algorithm/equiSolnAlgo/EquiSolnAlgo.h>
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>
#include <solution/analysis/numberer/DOF_Numberer.h>
#include <solution/analysis/handler/ConstraintHandler.h>
#include <solution/analysis/convergenceTest/ConvergenceTest.h>
#include <solution/analysis/integrator/TransientIntegrator.h>
#include <domain/domain/Domain.h>

// AddingSensitivity:BEGIN //////////////////////////////////
#ifdef _RELIABILITY
#include <reliability/FEsensitivity/SensitivityAlgorithm.h>
#endif
// AddingSensitivity:END ////////////////////////////////////

//! @brief Constructor.
XC::DirectIntegrationAnalysis::DirectIntegrationAnalysis(SoluMethod *metodo_solu)
  :TransientAnalysis(metodo_solu), domainStamp(0)
  {
// AddingSensitivity:BEGIN ////////////////////////////////////
#ifdef _RELIABILITY
	theSensitivityAlgorithm= nullptr;
#endif
// AddingSensitivity:END //////////////////////////////////////
  }

//! @brief Destructor.
XC::DirectIntegrationAnalysis::~DirectIntegrationAnalysis(void)
  {
    // we don't invoke the destructors in case user switching
    // from a static to a direct integration analysis 
    // clearAll() must be invoked if user wishes to invoke destructor
  }

//! @brief Clears all object members (constraint handler, analysis model,...).
void XC::DirectIntegrationAnalysis::clearAll(void)
  {
// AddingSensitivity:BEGIN ////////////////////////////////////
#ifdef _RELIABILITY
    delete theSensitivityAlgorithm;
#endif
// AddingSensitivity:END //////////////////////////////////////
  }    

int XC::DirectIntegrationAnalysis::initialize(void)
  {
    assert(metodo_solu);
    Domain *the_Domain = metodo_solu->getDomainPtr();

    // check if domain has undergone change
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
        domainStamp = stamp;	
        if(this->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; domainChanged() failed\n";
	    return -1;
          }	
      }
    if(metodo_solu->getTransientIntegratorPtr()->initialize() < 0)
      {
	std::cerr << nombre_clase() << "::" << __FUNCTION__
		  << "; integrator initialize() failed\n";
	return -2;
      }
    else
      metodo_solu->getTransientIntegratorPtr()->commit();
    return 0;
  }

//! @brief Performs the analysis.
//!
//! @param numSteps: number of steps in the analysis.
//! @param dT: time increment.
int XC::DirectIntegrationAnalysis::analyze(int numSteps, double dT)
  {
    int result= 0;
    assert(metodo_solu);
    EntCmd *old= metodo_solu->Owner();
    metodo_solu->set_owner(this);
    Domain *the_Domain = metodo_solu->getDomainPtr();

    for(int i=0; i<numSteps; i++)
      {
        if(newStepDomain(metodo_solu->getModelWrapperPtr()->getAnalysisModelPtr(),dT) < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; the AnalysisModel failed"
		      << " at time "
		      << the_Domain->getTimeTracker().getCurrentTime()
		      << std::endl;
	    the_Domain->revertToLastCommit();
	    return -2;
          }

        // check if domain has undergone change
        int stamp = the_Domain->hasDomainChanged();
        if(stamp != domainStamp)
          {
	    domainStamp = stamp;	
	    if(this->domainChanged() < 0)
              {
	        std::cerr << nombre_clase() << "::" << __FUNCTION__
			  << "; domainChanged() failed\n";
	        return -1;
              }	
          }

        if(metodo_solu->getTransientIntegratorPtr()->newStep(dT) < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; the XC::Integrator failed at time "
		      << the_Domain->getTimeTracker().getCurrentTime()
		      << std::endl;
	    the_Domain->revertToLastCommit();
	    return -2;
          }

        result = metodo_solu->getEquiSolutionAlgorithmPtr()->solveCurrentStep();
        if(result < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; the Algorithm failed at time "
		      << the_Domain->getTimeTracker().getCurrentTime()
		      << std::endl;
	    the_Domain->revertToLastCommit();	    
	    metodo_solu->getTransientIntegratorPtr()->revertToLastStep();
	    return -3;
          }    

// AddingSensitivity:BEGIN ////////////////////////////////////
#ifdef _RELIABILITY
        if(theSensitivityAlgorithm != 0)
          {
	    result = theSensitivityAlgorithm->computeSensitivities();
	    if(result < 0)
              {
                std::cerr << nombre_clase() << "::" << __FUNCTION__
			  << "; the SensitivityAlgorithm failed"
			  << " at iteration: " << i
			  << " with domain at load factor "
			  << the_Domain->getTimeTracker().getCurrentTime()
			  << std::endl;
	        the_Domain->revertToLastCommit();	    
	        metodo_solu->getTransientIntegratorPtr()->revertToLastStep();
	        return -5;
	      }
          }
#endif
// AddingSensitivity:END //////////////////////////////////////
      
        result= metodo_solu->getTransientIntegratorPtr()->commit();
        if(result < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; the Integrator failed to commit at time "
		      << the_Domain->getTimeTracker().getCurrentTime()
		      << std::endl;
	    the_Domain->revertToLastCommit();	    
	    metodo_solu->getTransientIntegratorPtr()->revertToLastStep();
	    return -4;
          }
      }    
    metodo_solu->set_owner(old);
    return result;
  }

//! @brief Ejecuta los cambios que implica un cambio en el domain del problema.
int XC::DirectIntegrationAnalysis::domainChanged(void)
  {
    assert(metodo_solu);
    Domain *the_Domain = metodo_solu->getDomainPtr();
    int stamp = the_Domain->hasDomainChanged();
    domainStamp = stamp;
   
    metodo_solu->getModelWrapperPtr()->getAnalysisModelPtr()->clearAll();    
    metodo_solu->getModelWrapperPtr()->getConstraintHandlerPtr()->clearAll();
    
    // now we invoke handle() on the constraint handler which
    // causes the creation of XC::FE_Element and XC::DOF_Group objects
    // and their addition to the XC::AnalysisModel.

    metodo_solu->getModelWrapperPtr()->getConstraintHandlerPtr()->handle();
    // we now invoke number() on the numberer which causes
    // equation numbers to be assigned to all the DOFs in the
    // AnalysisModel.


    metodo_solu->getModelWrapperPtr()->getDOF_NumbererPtr()->numberDOF();

    metodo_solu->getModelWrapperPtr()->getConstraintHandlerPtr()->doneNumberingDOF();

    // we invoke setGraph() on the XC::LinearSOE which
    // causes that object to determine its size

    metodo_solu->getLinearSOEPtr()->setSize(metodo_solu->getModelWrapperPtr()->getAnalysisModelPtr()->getDOFGraph());

    // we invoke domainChange() on the integrator and algorithm
    metodo_solu->getTransientIntegratorPtr()->domainChanged();
    metodo_solu->getEquiSolutionAlgorithmPtr()->domainChanged();


    return 0;
  }    

// AddingSensitivity:BEGIN //////////////////////////////
#ifdef _RELIABILITY
int XC::DirectIntegrationAnalysis::setSensitivityAlgorithm(SensitivityAlgorithm *passedSensitivityAlgorithm)
  {
    int result = 0;

    // invoke the destructor on the old one
    if(theSensitivityAlgorithm)
      { delete theSensitivityAlgorithm; }

    theSensitivityAlgorithm = passedSensitivityAlgorithm;
    return 0;
  }
#endif
// AddingSensitivity:END ///////////////////////////////

//! @brief Sets the renumerador to use in the analysis.
int XC::DirectIntegrationAnalysis::setNumberer(DOF_Numberer &theNewNumberer) 
  {
    int result= TransientAnalysis::setNumberer(theNewNumberer);

    // invoke domainChanged() either indirectly or directly
    Domain *the_Domain = metodo_solu->getDomainPtr();
    int stamp = the_Domain->hasDomainChanged();
    domainStamp = stamp;
    result = this->domainChanged();    
    if(result < 0)
      {
        std::cerr << nombre_clase() << "::" << __FUNCTION__
		  << "; setNumberer() failed";
        return -1;
      }	
    return 0;
  }


//! @brief Sets the solutio algorithm to use in the analysis.
int XC::DirectIntegrationAnalysis::setAlgorithm(EquiSolnAlgo &theNewAlgorithm) 
  {
    // invoke the destructor on the old one
    TransientAnalysis::setAlgorithm(theNewAlgorithm);

    // invoke domainChanged() either indirectly or directly
    assert(metodo_solu);
    Domain *the_Domain = metodo_solu->getDomainPtr();
    // check if domain has undergone change
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;	    
	if(this->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; domainChanged failed";
	    return -1;
	  }	
      }
    else
      {
	if(metodo_solu->getEquiSolutionAlgorithmPtr()->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; algorithm::domainChanged() failed";
	    return -2;
	  }
      }
    return 0;
  }

//! @brief Sets the integrator to use in the analysis.
int XC::DirectIntegrationAnalysis::setIntegrator(TransientIntegrator &theNewIntegrator)
  {
    // set the links needed by the other objects in the aggregation
    TransientAnalysis::setIntegrator(theNewIntegrator);

    // invoke domainChanged() either indirectly or directly
    int stamp = metodo_solu->getDomainPtr()->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;	    
	if(this->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; domainChanged failed";
	    return -1;
          }	
      }
    else
      {
        if(metodo_solu->getTransientIntegratorPtr()->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; Integrator::domainChanged failed";
	    return -1;
          }	
      }
    return 0;
  }

//! @brief Sets the linear system of equations to use in the analysis.
int XC::DirectIntegrationAnalysis::setLinearSOE(LinearSOE &theNewSOE)
  {
    // invoke the destructor on the old one
    TransientAnalysis::setLinearSOE(theNewSOE);

    // set the links needed by the other objects in the aggregation

    // set the size either indirectly or directly
    assert(metodo_solu);
    Domain *the_Domain= metodo_solu->getDomainPtr();
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
        domainStamp = stamp;	    
        if(this->domainChanged() < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; domainChanged failed";
	    return -1;
          }	
      }
    else
      {
        Graph &theGraph = metodo_solu->getModelWrapperPtr()->getAnalysisModelPtr()->getDOFGraph();
        if(metodo_solu->getLinearSOEPtr()->setSize(theGraph) < 0)
          {
	    std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; LinearSOE::setSize() failed";
	    return -2;	
          }
      }
    return 0;
  }

//! @brief Sets the convergence test to use in the analysis.
int XC::DirectIntegrationAnalysis::setConvergenceTest(ConvergenceTest &theNewTest)
  {
    if(metodo_solu)
      metodo_solu->setConvergenceTest(theNewTest);
    return 0;
  }

//! @brief Comprueba si el domain ha cambiado.
int XC::DirectIntegrationAnalysis::checkDomainChange(void)
  {
    assert(metodo_solu);
    Domain *the_Domain = metodo_solu->getDomainPtr();

    // check if domain has undergone change
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
        domainStamp = stamp;	
        if(this->domainChanged() < 0)
          {
            std::cerr << nombre_clase() << "::" << __FUNCTION__
		      << "; domainChanged() failed\n";
            return -1;
          }	
      }
    return 0;
  }




