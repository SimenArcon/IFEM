// $Id$
//==============================================================================
//!
//! \file ASMs2Dmx.h
//!
//! \date Nov 18 2010
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief Driver for assembly of structured 2D spline mixed FE models.
//!
//==============================================================================

#ifndef _ASM_S2D_MX_H
#define _ASM_S2D_MX_H

#include "ASMs2D.h"
#include "ASMmxBase.h"


/*!
  \brief Driver for assembly of structured 2D spline mixed FE models.
  \details This class implements a two-field mixed formulation with splines as
  basis functions. The first field is of one order higher than the second field,
  and its basis is obtained by order-elevating the input spline object once.
  By default, the geometry is represented by the second (lower order) basis,
  however, by negating the \a n_f1 argument to the constructor, the first
  basis is used instead.
*/

class ASMs2Dmx : public ASMs2D, private ASMmxBase
{
public:
  //! \brief Constructor creating an instance by reading the given file.
  ASMs2Dmx(const char* fileName, unsigned char n_s = 2,
	   char n_f1 = 2, unsigned char n_f2 = 1);
  //! \brief Constructor creating an instance by reading the given input stream.
  ASMs2Dmx(std::istream& is, unsigned char n_s = 2,
	   char n_f1 = 2, unsigned char n_f2 = 1);
  //! \brief Default constructor creating an empty patch.
  ASMs2Dmx(unsigned char n_s = 2, char n_f1 = 2, unsigned char n_f2 = 1);
  //! \brief Empty destructor.
  virtual ~ASMs2Dmx() {}


  // Methods for model generation
  // ============================

  //! \brief Generates the finite element topology data for the patch.
  //! \details The data generated are the element-to-node connectivity array,
  //! the node-to-IJ-index array, as well as global node and element numbers.
  virtual bool generateFEMTopology();

  //! \brief Clears the contents of the patch, making it empty.
  virtual void clear();

  //! \brief Returns the global coordinates for the given node.
  //! \param[in] inod 1-based node index local to current patch
  virtual Vec3 getCoord(size_t inod) const;

  //! \brief Returns the number of solution fields.
  virtual unsigned char getNoFields(int basis = 0) const;
  //! \brief Returns the number of DOFs per node.
  //! \param[in] inod 1-based node index local to current patch
  virtual unsigned char getNodalDOFs(size_t inod) const;

  //! \brief Initializes the patch level MADOF array for mixed problems.
  virtual void initMADOF(const int* sysMadof);


  // Methods for integration of finite element quantities.
  // These are the main computational methods of the ASM class hierarchy.
  // ====================================================================

  //! \brief Evaluates an integral over the interior patch domain.
  //! \param integrand Object with problem-specific data and methods
  //! \param glbInt The integrated quantity
  //! \param[in] time Parameters for nonlinear/time-dependent simulations
  //! \param locInt Vector of element-wise contributions to \a glbInt
  virtual bool integrate(Integrand& integrand,
			 GlobalIntegral& glbInt, const TimeDomain& time,
			 const LintegralVec& locInt = LintegralVec());

  //! \brief Evaluates a boundary integral over a patch edge.
  //! \param integrand Object with problem-specific data and methods
  //! \param[in] lIndex Local index of the boundary edge
  //! \param glbInt The integrated quantity
  //! \param[in] time Parameters for nonlinear/time-dependent simulations
  //! \param locInt Vector of element-wise contributions to \a glbInt
  virtual bool integrate(Integrand& integrand, int lIndex,
			 GlobalIntegral& glbInt, const TimeDomain& time,
			 const LintegralVec& locInt = LintegralVec());


  // Post-processing methods
  // =======================

  //! \brief Evaluates the primary solution field at the given points.
  //! \param[out] sField Solution field
  //! \param[in] locSol Solution vector local to current patch
  //! \param[in] gpar Parameter values of the result sampling points
  //! \param[in] regular Flag indicating how the sampling points are defined
  //!
  //! \details When \a regular is \e true, it is assumed that the parameter
  //! value array \a gpar forms a regular tensor-product point grid of dimension
  //! \a gpar[0].size() \a X \a gpar[1].size().
  //! Otherwise, we assume that it contains the \a u and \a v parameters
  //! directly for each sampling point.
  virtual bool evalSolution(Matrix& sField, const Vector& locSol,
			    const RealArray* gpar, bool regular = true) const;

  //! \brief Evaluates the secondary solution field at the given points.
  //! \param[out] sField Solution field
  //! \param[in] integrand Object with problem-specific data and methods
  //! \param[in] gpar Parameter values of the result sampling points
  //! \param[in] regular Flag indicating how the sampling points are defined
  //!
  //! \details When \a regular is \e true, it is assumed that the parameter
  //! value array \a gpar forms a regular tensor-product point grid of dimension
  //! \a gpar[0].size() \a X \a gpar[1].size().
  //! Otherwise, we assume that it contains the \a u and \a v parameters
  //! directly for each sampling point.
  virtual bool evalSolution(Matrix& sField, const Integrand& integrand,
			    const RealArray* gpar, bool regular = true) const;

  //! \brief Extracts nodal results for this patch from the global vector.
  //! \param[in] globVec Global solution vector in DOF-order
  //! \param[out] nodeVec Nodal result vector for this patch
  virtual void extractNodeVec(const Vector& globVec, Vector& nodeVec,
			      unsigned char = 0) const;

protected:

  // Internal utility methods
  // ========================

  //! \brief Returns a matrix with nodal coordinates for an element.
  //! \param[in] iel Element index
  //! \param[out] X 3\f$\times\f$n-matrix, where \a n is the number of nodes
  //! in one element
  virtual bool getElementCoordinates(Matrix& X, int iel) const;

  //! \brief Returns the number of nodal points in each parameter direction.
  //! \param[out] n1 Number of nodes in first (u) direction
  //! \param[out] n2 Number of nodes in second (v) direction
  //! \param[in] basis Which basis to return size parameters for
  virtual bool getSize(int& n1, int& n2, int basis = 0) const;

private:
  Go::SplineSurface* basis1; //!< Pointer to spline object for the first basis
  Go::SplineSurface* basis2; //!< Pointer to spline object for the second basis
};

#endif
