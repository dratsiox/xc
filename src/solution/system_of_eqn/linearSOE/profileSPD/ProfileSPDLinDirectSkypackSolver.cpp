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
                                                                        
// $Revision: 1.2 $
// $Date: 2003/02/14 23:02:03 $
// $Source: /usr/local/cvs/OpenSees/SRC/system_of_eqn/linearSOE/profileSPD/ProfileSPDLinDirectSkypackSolver.cpp,v $
                                                                        
                                                                        
// File: ~/system_of_eqn/linearSOE/ProfileSPD/ProfileSPDLinDirectSkypackSolver.C
//
// Written: fmk 
// Created: 03/98
// Revision: A
//
// Description: This file contains the class definition for 
// ProfileSPDLinDirectSkypackSolver. ProfileSPDLinDirectSkypackSolver 
// is a subclass of LinearSOESOlver. It solves a XC::ProfileSPDLinSOE object using
// the Skypack library developed by Osni Marques, software available at
//   http://www.nersc.gov/research/SCG/Osni/marques_software.html

// What: "@(#) ProfileSPDLinDirectSkypackSolver.C, revA"

#include <solution/system_of_eqn/linearSOE/profileSPD/ProfileSPDLinDirectSkypackSolver.h>
#include <solution/system_of_eqn/linearSOE/profileSPD/ProfileSPDLinSOE.h>
#include <utility/Timer.h>

XC::ProfileSPDLinDirectSkypackSolver::ProfileSPDLinDirectSkypackSolver()
:ProfileSPDLinSolver(SOLVER_TAGS_ProfileSPDLinDirectSkypackSolver),
 mCols(0), mRows(0),rw(0),tw(0), index(0),
 size(0), invD(0)
{

}

XC::ProfileSPDLinDirectSkypackSolver::ProfileSPDLinDirectSkypackSolver(int Mcols, int Mrows)
:ProfileSPDLinSolver(SOLVER_TAGS_ProfileSPDLinDirectSkypackSolver),
 mCols(Mcols), mRows(Mrows),rw(0),tw(0), index(0),
 size(0), invD(0)
{
    if(mCols != 0 && mRows != 0)
      {
	rw= Vector(mRows*mCols);
	tw= Vector(mRows*mRows);
	if(mCols > mRows)
	  index= ID(mCols);
	else
	  index= ID(mRows);
      }
    else
      { // make sure mCols and mRows == 0
	mCols = 0;
	mRows = 0;
      }

  }

    
int XC::ProfileSPDLinDirectSkypackSolver::setSize(void)
  {
    int result = 0;

    if(theSOE == 0)
      {
	std::cerr << nombre_clase() << "::" << __FUNCTION__;
	std::cerr << " No system has been set\n";
	return -1;
      }

    // check for quick return 
    if (theSOE->size == 0)
	return 0;
    
    size = theSOE->size;
    block[0] = 1;
    block[1] = size;
    block[2] = 1;

    //resize the work areas
    invD= Vector(size);
    return result;
}

extern "C" int skysf2_(double *A, double *D, int *DGPNT, int *JMIN, int *JMAX); 

extern "C" int skypf2_(int *index, int *JMAX, int *JMIN, int *MCOLS, int *MROWS, int *DGPNT, 
		       double *A, double *RW, double *TW);

extern "C" int skyss_(int *LDX, int *N, int *NRHS, 
		      double *A, double *D, double *X, int *DGPNT,
		      int *BLOCK, int *NBLOCK,
		      char *FNAME, int *FUNIT, int *INFO);

int XC::ProfileSPDLinDirectSkypackSolver::solve(void)
  {
    // check for XC::quick returns
    if(theSOE == 0)
      {
	std::cerr << nombre_clase() << "::" << __FUNCTION__;
	std::cerr << " - No ProfileSPDSOE has been assigned\n";
	return -1;
      }
    
    if(theSOE->size == 0)
      return 0;    
    

    // check that work area invD has been created
    if(invD.Nulo())
      {
	std::cerr << nombre_clase() << "::" << __FUNCTION__;
	std::cerr << " - no space for invD - has setSize() been called?\n";
	return -1;
      }	

    // set some pointers
    double *A= theSOE->A.getDataPtr();
    double *B= theSOE->getPtrB();
    double *X= theSOE->getPtrX();
    int *iDiagLoc= theSOE->iDiagLoc.getDataPtr();
    int theSize= theSOE->size;
    // copy B into X
    for(int ii=0; ii<theSize; ii++)
      X[ii] = B[ii];

    char FILE[]= "INCORE";
    
    if(theSOE->factored == false)
      {
      
	// FACTOR 
	if (mRows == 0 || mCols == 0) { // factor using skysf2_
	    int JMIN =1;
	    int JMAX = size;
	    skysf2_(A, invD.getDataPtr(), iDiagLoc, &JMIN, &JMAX); 
	}
	else { // factor using skypf2_
	    int JMIN =1;
	    int JMAX = size;
	    int MCOLS = mCols;
	    int MROWS = mRows;
	    double *RW= rw.getDataPtr();
	    double *TW= tw.getDataPtr();
	    int *INDEX= index.getDataPtr();
	    skypf2_(INDEX, &JMAX, &JMIN, &MCOLS, &MROWS, iDiagLoc, A, RW, TW);

	    // set up invD
	    for (int i=0; i<size; i++)
		invD[i] = 1.0/A[iDiagLoc[i]-1]; // iDiagLoc has fortran array indexing
	}
      
	// mark the system as having been factored
	theSOE->factored = true;
	theSOE->numInt = 0;      
    }

    /*
     * now do the forward and back substitution
     */

    int LDX = theSize;
    int NRHS = 1;
    int numBlock = 1;
    int fileFD = 0;
    int INFO;
    A = theSOE->A.getDataPtr();
    X = theSOE->getPtrX();
    int *BLOCK = &block[0];
    iDiagLoc = theSOE->iDiagLoc.getDataPtr();
    
    skyss_(&LDX, &theSize, &NRHS, A, invD.getDataPtr(), X, iDiagLoc, BLOCK, &numBlock, 
	   FILE,  &fileFD, &INFO);
      
    // return
    if (INFO < 0) {
	std::cerr << nombre_clase() << "::" << __FUNCTION__;
	std::cerr << " error value returned from skyss()\n";
    }    

    return INFO;

}


int XC::ProfileSPDLinDirectSkypackSolver::setProfileSOE(ProfileSPDLinSOE &theNewSOE)
{
    theSOE = &theNewSOE;
    return 0;
}
	
int XC::ProfileSPDLinDirectSkypackSolver::sendSelf(CommParameters &cp)
{
    if (size != 0)
	std::cerr << "XC::ProfileSPDLinDirectSkypackSolver::sendSelf - does not send itself YET\n"; 
    return 0;
}


int XC::ProfileSPDLinDirectSkypackSolver::recvSelf(const CommParameters &cp)
{
    return 0;
}


