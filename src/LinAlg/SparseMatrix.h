// $Id: SparseMatrix.h,v 1.17 2011-02-04 17:02:38 kmo Exp $
//==============================================================================
//!
//! \file SparseMatrix.h
//!
//! \date Jan 8 2008
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief Representation of the system matrix on an unstructured sparse format.
//!
//==============================================================================

#ifndef _SPARSE_MATRIX_H
#define _SPARSE_MATRIX_H

#include "SystemMatrix.h"
#include <iostream>
#include <map>

typedef std::pair<size_t,size_t> IJPair;    //!< 1-based matrix indices
typedef std::map<IJPair,real>    ValueMap;  //!< Index to matrix value mapping
typedef ValueMap::const_iterator ValueIter; //!< Iterator over matrix elements

struct SuperLUdata;


/*!
  \brief Class for representing a system matrix on an unstructured sparse form.
  \details The sparse matrix is editable in the sense that non-zero entries may
  be added at arbitrary locations. The class comes with methods for solving a
  linear system of equations based on the current matrix and a given RHS-vector,
  using either the commercial SAMG package or the public domain SuperLU package.
*/

class SparseMatrix : public SystemMatrix
{
public:
  //! \brief Available equation solvers for this matrix type.
  enum SparseSolver { NONE, SUPERLU, S_A_M_G };

  //! \brief Default constructor creating an empty matrix.
  SparseMatrix(SparseSolver eqSolver = NONE, int nt = 1);
  //! \brief Copy constructor.
  SparseMatrix(const SparseMatrix& B);
  //! \brief The destructor frees the dynamically allocated arrays.
  virtual ~SparseMatrix();

  //! \brief Returns the matrix type.
  virtual Type getType() const { return solver == S_A_M_G ? SAMG : SPARSE; }

  //! \brief Creates a copy of the system matrix and returns a pointer to it.
  virtual SystemMatrix* copy() const { return new SparseMatrix(*this); }

  //! \brief Resizes the matrix to dimension \f$r \times c\f$.
  //! \details Will erase previous content, if any.
  void resize(size_t r, size_t c = 0);

  //! \brief Resizes the matrix to dimension \f$r \times c\f$.
  //! \details Will preserve existing matrix content within the new dimension.
  bool redim(size_t r, size_t c);

  //! \brief Query number of matrix rows.
  size_t rows() const { return nrow; }
  //! \brief Query number of matrix columns.
  size_t cols() const { return ncol; }
  //! \brief Query total matrix size in terms of number of non-zero elements.
  size_t size() const { return editable ? elem.size() : A.size(); }

  //! \brief Returns the dimension of the system matrix.
  //! \param[in] idim Which direction to return the dimension in.
  virtual size_t dim(int idim = 1) const;

  //! \brief Index-1 based element access.
  //! \note For editable matrix only.
  real& operator()(size_t r, size_t c);
  //! \brief Index-1 based element reference.
  const real& operator()(size_t r, size_t c) const;

  //! \brief For traversal of the non-zero elements of an editable matrix.
  const ValueMap& getValues() const { return elem; }

  //! \brief Print sparsity pattern - for inspection purposes.
  void printSparsity(std::ostream& os) const;

  //! \brief Print the matrix in full rectangular form.
  //! \warning Not recommended for matrices of nontrivial size.
  void printFull(std::ostream& os) const;

  //! \brief Initializes the element assembly process.
  //! \details Must be called once before the element assembly loop.
  //! \param[in] sam Auxilliary data describing the FE model topology, etc.
  virtual void initAssembly(const SAM& sam);

  //! \brief Initializes the matrix to zero assuming it is properly dimensioned.
  virtual void init();

  //! \brief Adds an element stiffness matrix into the system stiffness matrix.
  //! \param[in] eM  The element stiffness matrix
  //! \param[in] sam Auxilliary data describing the FE model topology,
  //!                nodal DOF status and constraint equations
  //! \param[in] e   Identifier for the element that \a eM belongs to
  //! \return \e true on successful assembly, otherwise \e false
  virtual bool assemble(const Matrix& eM, const SAM& sam, int e);
  //! \brief Adds an element stiffness matrix into the system stiffness matrix.
  //! \details When multi-point constraints are present, contributions from
  //! these are also added into the system right-hand-side load vector.
  //! \param[in] eM  The element stiffness matrix
  //! \param[in] sam Auxilliary data describing the FE model topology,
  //!                nodal DOF status and constraint equations
  //! \param     B   The system right-hand-side load vector
  //! \param[in] e   Identifier for the element that \a eM belongs to
  //! \return \e true on successful assembly, otherwise \e false
  virtual bool assemble(const Matrix& eM, const SAM& sam,
			SystemVector& B, int e);

  //! \brief Adds a nodal vector into columns of a non-symmetric sparse matrix.
  //! \param[in] V   The nodal vector
  //! \param[in] sam Auxilliary data describing the FE model topology,
  //!                nodal DOF status and constraint equations
  //! \param[in] n   Identifier for the node that \a V belongs to
  //! \param[in] col Index of first column which should receive contributions
  //! \return \e true on successful assembly, otherwise \e false
  //!
  //! \details This method can be used for rectangular matrices whose rows
  //! correspond to the equation ordering og the provided \a sam object.
  bool assembleCol(const RealArray& V, const SAM& sam, int n, size_t col);

  //! \brief Adds a scalar value into columns of a non-symmetric sparse matrix.
  //! \param[in] val The value to add for each DOF of the specified node
  //! \param[in] sam Auxilliary data describing the FE model topology,
  //!                nodal DOF status and constraint equations
  //! \param[in] n   Identifier for the node that \a val belongs to
  //! \param[in] col Index of first column which should receive contributions
  //! \return \e true on successful assembly, otherwise \e false
  //!
  //! \details This method can be used for rectangular matrices whose rows
  //! correspond to the equation ordering og the provided \a sam object.
  bool assembleCol(real val, const SAM& sam, int n, size_t col)
  {
    return this->assembleCol(RealArray(1,val),sam,n,col);
  }

  //! \brief Augments a similar matrix symmetrically to the current matrix.
  //! \param[in] B  The matrix to be augmented
  //! \param[in] r0 Row offset for the augmented matrix
  //! \param[in] c0 Column offset for the augmented matrix
  virtual bool augment(const SystemMatrix& B, size_t r0, size_t c0);

  //! \brief Truncates all small matrix elements to zero.
  //! \param[in] threshold Zero tolerance relative to largest diagonal element
  virtual bool truncate(real threshold = real(1.0e-16));

  //! \brief Adds a matrix with similar sparsity pattern to the current matrix.
  //! \param[in] B     The matrix to be added
  //! \param[in] alpha Scale factor for matrix \b B
  virtual bool add(const SystemMatrix& B, real alpha = real(1));

  //! \brief Adds the diagonal matrix \f$\sigma\f$\b I to the current matrix.
  virtual bool add(real sigma);

  //! \brief Performs the matrix-vector multiplication \b C = \a *this * \b B.
  virtual bool multiply(const SystemVector& B, SystemVector& C);

  //! \brief Solves the linear system of equations for a given right-hand-side.
  //! \param B Right-hand-side vector on input, solution vector on output
  //! \param newLHS \e true if the left-hand-side matrix has been updated
  virtual bool solve(SystemVector& B, bool newLHS = true);

protected:
  //! \brief Converts the matrix to an optimized row-oriented format.
  //! \details The optimized format is suitable for the SAMG equation solver.
  bool optimiseSAMG(bool transposed = false);

  //! \brief Converts the matrix to an optimized column-oriented format.
  //! \details The optimized format is suitable for the SuperLU equation solver.
  bool optimiseSLU();

  //! \brief Invokes the SAMG equation solver for a given right-hand-side.
  //! \param[in] isFirstRHS Should be \e true when the coefficient matrix is new
  //! \param B Right-hand-side vector on input, solution vector on output
  bool solveSAMG(bool isFirstRHS, Vector& B);

  //! \brief Invokes the SuperLU equation solver for a given right-hand-side.
  //! \details This method uses the simple driver \a dgssv.
  //! \param[in] isFirstRHS Should be \e true when the coefficient matrix is new
  //! \param B Right-hand-side vector on input, solution vector on output
  bool solveSLU(bool isFirstRHS, Vector& B);

  //! \brief Invokes the SuperLU equation solver for a given right-hand-side.
  //! \details This method uses the expert driver \a dgssvx.
  //! \param[in] isFirstRHS Should be \e true when the coefficient matrix is new
  //! \param B Right-hand-side vector on input, solution vector on output
  bool solveSLUx(bool isFirstRHS, Vector& B);

  //! \brief Writes the system matrix to the given output stream.
  virtual std::ostream& write(std::ostream& os) const;

public:
  static bool printSLUstat; //!< Print solution statistics for SuperLU?

private:
  bool editable; //!< \e true during element assembly, \e false after optimized
  size_t nrow;   //!< Number of matrix rows
  size_t ncol;   //!< Number of matrix columns

  std::vector<int> IA; //!< Identifies the beginning of each row or column
  std::vector<int> JA; //!< Specifies column/row index of each nonzero element
  std::vector<real> A; //!< Stores nonzero matrix elements
  ValueMap       elem; //!< Stores nonzero matrix elements with index pairs
  SparseSolver solver; //!< Which equation solver to use
  SuperLUdata*    slu; //!< Matrix data for the SuperLU equation solver
  int      numThreads; //!< Number of threads to use for the SuperLU_MT solver
};

#endif
