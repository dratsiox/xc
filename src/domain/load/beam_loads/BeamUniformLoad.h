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
//BeamUniformLoad.h
                                              
#ifndef BeamUniformLoad_h
#define BeamUniformLoad_h

#include "domain/load/beam_loads/BeamMecLoad.h"

namespace XC {

//! @ingroup ElemLoads
//
//! @brief Carga uniforme sobre elementos barra.
class BeamUniformLoad : public BeamMecLoad
  {
  public:
    BeamUniformLoad(int tag, int classTag, double wTrans, double wAxial,const ID &theElementTags);
    BeamUniformLoad(int tag, int classTag);
    BeamUniformLoad(int classTag);

    std::string Categoria(void) const;
    //! @brief Returns distributed force vectors (one for each element) expressed in elemnt local coordinates. Is simply a convenience function that makes the distributedness more explicit.
    inline const Matrix &getDistributedLocalForces(void) const
      { return getLocalForces(); }
    //! @brief Returns distributed moments (one for each element) expressed in element local coordinates. Is simply a convenience function that makes the distributedness more explicit.
    inline const Matrix &getDistributedLocalMoments(void) const
      { return getLocalMoments(); }
    //! @brief Returns distributed force vectors (one for each element) expressed in global coordinates. Is simply a convenience function that makes the distributedness more explicit.
    inline const Matrix &getDistributedGlobalForces(void) const
      { return getGlobalForces(); }
    //! @brief Returns distributed moments (one for each element) expressed in global coordinates. Is simply a convenience function that makes the distributedness more explicit.
    inline const Matrix &getDistributedGlobalMoments(void) const
      { return getGlobalMoments(); }
    virtual SVD3d getResultant(const Pos3d &p= Pos3d(), bool initialGeometry= true) const;
  };
} // end of XC namespace

#endif

