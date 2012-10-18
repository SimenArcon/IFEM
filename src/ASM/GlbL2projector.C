// $Id$
//==============================================================================
//!
//! \file GlbL2projector.C
//!
//! \date Oct 16 2012
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief General integrand for L2-projection of secondary solutions.
//!
//==============================================================================

#include "GlbL2projector.h"
#include "GlobalIntegral.h"
#include "LocalIntegral.h"
#include "FiniteElement.h"
#include "TimeDomain.h"
#include "ASMbase.h"
#include "Profiler.h"


/*!
  \brief Local integral container class for L2-projections.
*/

class L2Mats : public LocalIntegral
{
public:
  //! \brief The constructor initializes pointers and references.
  //! \param[in] p The global L2 integrand object containing projection matrices
  //! \param[in] q Pointer to element data associated with the problem integrand
  L2Mats(GlbL2& p, LocalIntegral* q = NULL) : gl2Int(p), elmData(q) {}
  //! \brief Empty destructor.
  virtual ~L2Mats() {}

  //! \brief Destruction method to clean up after numerical integration.
  virtual void destruct() { delete elmData; delete this; }

  GlbL2&         gl2Int;  //! The global L2 projection integrand
  LocalIntegral* elmData; //! Element data associated with the problem integrand
  IntVec         mnpc;    //! Matrix of element nodal correspondance
};


GlbL2::GlbL2 (IntegrandBase& p, size_t n) : problem(p), A(SparseMatrix::SUPERLU)
{
  A.redim(n,n);
  B.redim(n*p.getNoFields(2));
}


LocalIntegral* GlbL2::getLocalIntegral (size_t nen, size_t iEl,
                                        bool neumann) const
{
  return new L2Mats(*const_cast<GlbL2*>(this),
		    problem.getLocalIntegral(nen,iEl,neumann));
}


bool GlbL2::initElement (const IntVec& MNPC, const Vec3& Xc, size_t nPt,
                         LocalIntegral& elmInt)
{
  L2Mats& gl2 = static_cast<L2Mats&>(elmInt);

  gl2.mnpc = MNPC;
  return problem.initElement(MNPC,Xc,nPt,*gl2.elmData);
}


bool GlbL2::evalInt (LocalIntegral& elmInt,
                     const FiniteElement& fe,
                     const Vec3& X) const

{
  L2Mats& gl2 = static_cast<L2Mats&>(elmInt);

  Vector solPt;
  if (!problem.evalSol(solPt,fe,X,gl2.mnpc))
    return false;

  size_t a, b, nnod = A.dim();
  for (a = 0; a < fe.N.size(); a++)
  {
    int inod = gl2.mnpc[a]+1;
    for (b = 0; b < fe.N.size(); b++)
    {
      int jnod = gl2.mnpc[b]+1;
      A(inod,jnod) += fe.N[a]*fe.N[b]*fe.detJxW;
    }
    for (b = 0; b < solPt.size(); b++)
      B(inod+b*nnod) += fe.N[a]*solPt[b]*fe.detJxW;
  }

  return true;
}


void GlbL2::preAssemble (const std::vector<IntVec>& MMNPC, size_t nel)
{
  A.preAssemble(MMNPC,nel);
}


bool GlbL2::solve (Matrix& sField)
{
  // Solve the patch-global equation system
  if (!A.solve(B)) return false;

  // Store the nodal values of the projected field
  size_t nnod = A.dim();
  size_t ncomp = B.dim() / nnod;
  sField.resize(ncomp,nnod);
  for (size_t i = 1; i <= nnod; i++)
    for (size_t j = 1; j <= ncomp; j++)
      sField(j,i) = B(i+(j-1)*nnod);

  return true;
}


bool ASMbase::L2projection (Matrix& sField, IntegrandBase& integrand)
{
  PROFILE2("ASMbase::L2projection");

  GlbL2 gl2(integrand,this->getNoNodes(1));
  GlobalIntegral dummy;

  gl2.preAssemble(MNPC,this->getNoElms(true));
  return this->integrate(gl2,dummy,TimeDomain()) && gl2.solve(sField);
}