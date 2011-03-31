// $Id$
//==============================================================================
//!
//! \file ASMs2DLag.h
//!
//! \date Mar 20 2010
//!
//! \author Einar Christensen / SINTEF
//!
//! \brief Driver for assembly of structured 2D Lagrange FE models.
//!
//==============================================================================

#ifndef _ASM_S2D_LAG_H
#define _ASM_S2D_LAG_H

#include "ASMs2D.h"
#include "Vec3.h"


/*!
  \brief Driver for assembly of structured 2D Lagrange FE models.
  \details This class contains methods for structured 2D Lagrange patches.
*/

class ASMs2DLag : public ASMs2D
{
public:
  //! \brief Constructor creating an instance by reading the given file.
  ASMs2DLag(const char* fileName, unsigned char n_s = 2, unsigned char n_f = 2);
  //! \brief Constructor creating an instance by reading the given input stream.
  ASMs2DLag(std::istream& is, unsigned char n_s = 2, unsigned char n_f = 2);
  //! \brief Default constructor creating an empty patch.
  ASMs2DLag(unsigned char n_s = 2, unsigned char n_f = 2) : ASMs2D(n_s,n_f) {}
  //! \brief Empty destructor.
  virtual ~ASMs2DLag() {}


  // Methods for model generation
  // ============================

  //! \brief Generates the finite element topology data for the patch.
  //! \details The data generated are the element-to-node connectivity array,
  //! the nodal coordinate array, as well as global node and element numbers.
  virtual bool generateFEMTopology();

  //! \brief Clears the contents of the patch, making it empty.
  virtual void clear();

  //! \brief Returns the global coordinates for the given node.
  //! \param[in] inod 1-based node index local to current patch
  virtual Vec3 getCoord(size_t inod) const;


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

  //! \brief Creates a quad element model of this patch for visualization.
  //! \param[out] grid The generated quadrilateral grid
  //! \param[in] npe Number of visualization nodes over each knot span
  //! \note The number of element nodes must be set in \a grid on input.
  virtual bool tesselate(ElementBlock& grid, const int* npe) const;

  //! \brief Evaluates the primary solution field at all visualization points.
  //! \details The number of visualization points is the same as the order of
  //! the Lagrange elements by default.
  //! \param[out] sField Solution field
  //! \param[in] locSol Solution vector in DOF-order
  virtual bool evalSolution(Matrix& sField, const Vector& locSol,
			    const int*) const;

  //! \brief Evaluates the primary solution field at the given points.
  //! \param[out] sField Solution field
  //! \param[in] locSol Solution vector local to current patch
  //! \param[in] gpar Parameter values of the result sampling points
  virtual bool evalSolution(Matrix& sField, const Vector& locSol,
			    const RealArray* gpar, bool = true) const;

  //! \brief Evaluates the secondary solution field at all visualization points.
  //! \details The number of visualization points is the same as the order of
  //! the Lagrange elements by default.
  //! \param[out] sField Solution field
  //! \param[in] integrand Object with problem-specific data and methods
  virtual bool evalSolution(Matrix& sField, const Integrand& integrand,
			    const int*) const;

  //! \brief Evaluates the secondary solution field at the given points.
  //! \param[out] sField Solution field
  //! \param[in] integrand Object with problem-specific data and methods
  //! \param[in] gpar Parameter values of the result sampling points
  virtual bool evalSolution(Matrix& sField, const Integrand& integrand,
			    const RealArray* gpar, bool = true) const;

protected:

  // Internal utility methods
  // ========================

  //! \brief Returns a matrix with nodal coordinates for an element.
  //! \param[in] iel Element index
  //! \param[out] X 3\f$\times\f$n-matrix, where \a n is the number of nodes
  //! in one element
  virtual bool getElementCoordinates(Matrix& X, int iel) const;
  //! \brief Returns a matrix with all nodal coordinates within the patch.
  //! \param[out] X 3\f$\times\f$n-matrix, where \a n is the number of nodes
  //! in the patch
  virtual void getNodalCoordinates(Matrix& X) const;

  //! \brief Returns the number of nodal points in each parameter direction.
  //! \param[out] n1 Number of nodes in first (u) direction
  //! \param[out] n2 Number of nodes in second (v) direction
  virtual bool getSize(int& n1, int& n2, int = 0) const;

private:
  size_t            nx;    //!< Number of nodes in first parameter direction
  size_t            ny;    //!< Number of nodes in second parameter direction
  std::vector<Vec3> coord; //!< Nodal coordinates
};

#endif
