// $Id: SIMinput.h,v 1.3 2010-10-10 11:20:54 kmo Exp $
//==============================================================================
//!
//! \file SIMinput.h
//!
//! \date Jun 1 2010
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief Base class for simulators with input parsing functionality.
//!
//==============================================================================

#ifndef _SIM_INPUT_H
#define _SIM_INPUT_H

#include <iostream>


/*!
  \brief Base class for NURBS-based FEM simulators with input file parsing.
*/

class SIMinput
{
protected:
  //! \brief The default constructor initializes \a myPid and \a nProc.
  SIMinput();
  //! \brief Empty destructor.
  virtual ~SIMinput() {}

public:
  //! \brief Reads model data from the specified input file \a *fileName.
  bool read(const char* fileName);
  //! \brief Parses a data section from an input stream.
  virtual bool parse(char* keyWord, std::istream& is) = 0;

  static int msgLevel; //!< Controls the amount of console output during solving

protected:
  int myPid; //!< Processor ID in parallel simulations
  int nProc; //!< Number of processors in parallel simulations
};

#endif
