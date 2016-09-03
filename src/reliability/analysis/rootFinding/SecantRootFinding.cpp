//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  El programa deriva del denominado OpenSees <http://opensees.berkeley.edu>
//  desarrollado por el «Pacific earthquake engineering research center».
//
//  Salvo las restricciones que puedan derivarse del copyright del
//  programa original (ver archivo copyright_opensees.txt) este
//  software es libre: usted puede redistribuirlo y/o modificarlo 
//  bajo los términos de la Licencia Pública General GNU publicada 
//  por la Fundación para el Software Libre, ya sea la versión 3 
//  de la Licencia, o (a su elección) cualquier versión posterior.
//
//  Este software se distribuye con la esperanza de que sea útil, pero 
//  SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita
//  MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
//  Consulte los detalles de la Licencia Pública General GNU para obtener 
//  una información más detallada. 
//
// Debería haber recibido una copia de la Licencia Pública General GNU 
// junto a este programa. 
// En caso contrario, consulte <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 2001, The Regents of the University of California    **
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
** Reliability module developed by:                                   **
**   Terje Haukaas (haukaas@ce.berkeley.edu)                          **
**   Armen Der Kiureghian (adk@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.1 $
// $Date: 2003/03/04 00:39:31 $
// $Source: /usr/local/cvs/OpenSees/SRC/reliability/analysis/rootFinding/SecantRootFinding.cpp,v $


//
// Written by Terje Haukaas (haukaas@ce.berkeley.edu) 
//

#include <reliability/analysis/rootFinding/SecantRootFinding.h>
#include <reliability/analysis/rootFinding/RootFinding.h>
#include <reliability/analysis/gFunction/GFunEvaluator.h>
#include <reliability/analysis/transformation/ProbabilityTransformation.h>
#include <reliability/domain/components/ReliabilityDomain.h>
#include <reliability/domain/components/RandomVariable.h>
#include <cmath>
#include <utility/matrix/Vector.h>


XC::SecantRootFinding::SecantRootFinding(
						ReliabilityDomain *passedReliabilityDomain,
						ProbabilityTransformation *passedProbabilityTransformation,
						GFunEvaluator *passedGFunEvaluator,
						int passedMaxIter,
						double ptol,
						double pmaxStepLength)
:RootFinding()
{
	theReliabilityDomain = passedReliabilityDomain;
	theProbabilityTransformation = passedProbabilityTransformation;
	theGFunEvaluator = passedGFunEvaluator;
	maxIter = passedMaxIter;
	tol = ptol;
	maxStepLength = pmaxStepLength;
}



 XC::Vector
XC::SecantRootFinding::findLimitStateSurface(int space, double g, Vector pDirection, Vector thePoint)
{
	// Set scale factor for 'g' for convergence check
	double scaleG;
	if (fabs(g)>1.0e-4) { scaleG = g;}
	else { 		scaleG = 1.0;}

	// Normalize the direction vector
	Vector Direction = pDirection/pDirection.Norm();

	// Scale 'maxStepLength' by standard deviation
	// (only if the user has specified to "walk" in original space)
	double perturbation;
	double realMaxStepLength = maxStepLength;
	if (space == 1) {

		// Go through direction vector and see which element is biggest
		// compared to its standard deviation
		int nrv = theReliabilityDomain->getNumberOfRandomVariables();
		RandomVariable *theRV;
		double stdv, theStdv= 0.0;
		int theBiggest;
		double maxRatio = 0.0;
		for(int i=0; i<nrv; i++)
                  {
		    theRV = theReliabilityDomain->getRandomVariablePtr(i+1);
		    stdv = theRV->getStdv();
			if (Direction(i)/stdv > maxRatio) {
				maxRatio = Direction(i)/stdv;
				theStdv = stdv;
				theBiggest = i+1;
			}
		}

		// Now scale so that 'maxStepSize' is related to the real stdv
		perturbation = maxStepLength * theStdv;
		realMaxStepLength = perturbation;
	}
	else {
		perturbation = maxStepLength;
	}

	Vector theTempPoint;
	double g_old= 0.0, g_new;
	bool didNotConverge=true;
	double result;
	double tangent;


	int i=0;
	while (i<=maxIter && didNotConverge) {


		// Increment counter right away...
		i++;

		if (i!=1) {

			// Transform the point into x-space if the user has given it in 2-space
			if (space==2) {
				result = theProbabilityTransformation->set_u(thePoint);
				if (result < 0) {
					std::cerr << "XC::GFunVisualizationAnalysis::analyze() - " << std::endl
						<< " could not set u in the xu-transformation." << std::endl;
					//return -1; Comentado LCPT.
				}

				result = theProbabilityTransformation->transform_u_to_x();
				if (result < 0) {
					std::cerr << "XC::GFunVisualizationAnalysis::analyze() - " << std::endl
						<< " could not transform from u to x and compute Jacobian." << std::endl;
					//return -1; Comentado LCPT.
				}
				theTempPoint = theProbabilityTransformation->get_x();
			}
			else {
				theTempPoint = thePoint;
			}


			// Evaluate limit-state function
			result = theGFunEvaluator->runGFunAnalysis(theTempPoint);
			if (result < 0) {
				std::cerr << "XC::GFunVisualizationAnalysis::analyze() - " << std::endl
					<< " could not run analysis to evaluate limit-state function. " << std::endl;
				//return -1; Comentado LCPT.
			}
			result = theGFunEvaluator->evaluateG(theTempPoint);
			if (result < 0) {
				std::cerr << "XC::GFunVisualizationAnalysis::analyze() - " << std::endl
					<< " could not tokenize limit-state function. " << std::endl;
				//return -1; Comentado LCPT.
			}
			g_new = theGFunEvaluator->getG();
		}
		else {
			g_new = g;
		}

		

		// Check convergence
		if (fabs(g_new/scaleG) < tol) {
			didNotConverge = false;
		}
		else {
			if (i==maxIter) {
				std::cerr << "WARNING: Projection scheme failed to find surface..." << std::endl;
			}
			else if (i==1) {
				thePoint = thePoint - perturbation * Direction;
				g_old = g_new;
			}
			else {

				// Take a step
				tangent = (g_new-g_old)/perturbation;
				perturbation = -g_new/tangent;
				if (fabs(perturbation) > realMaxStepLength) {
					perturbation = perturbation/fabs(perturbation)*realMaxStepLength;
				}
				thePoint = thePoint - perturbation * Direction;
				g_old = g_new;
			}
		}
	}

	return thePoint;
}


