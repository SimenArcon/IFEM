// $Id$
//==============================================================================
//!
//! \file TestNewmark.C
//!
//! \date May 14 2016
//!
//! \author Knut Morten Okstad / SINTEF
//!
//! \brief Tests the Newmark time integrator.
//!
//==============================================================================

#include "SAM.h"
#include "IntegrandBase.h"
#include "SIMoutput.h"

#include "GenAlphaSIM.h"
#include "NewmarkMats.h"
#include "AlgEqSystem.h"
#include "TimeStep.h"

#include "gtest/gtest.h"


// SAM class representing a single-DOF system.
class SAM1DOF : public SAM
{
public:
  SAM1DOF()
  {
    nmmnpc = nel = nnod = ndof = neq = 1;
    mmnpc  = new int[1]; mmnpc[0] = 1;
    mpmnpc = new int[2]; std::iota(mpmnpc,mpmnpc+2,1);
    madof  = new int[2]; std::iota(madof,madof+2,1);
    msc    = new int[1]; msc[0] = 1;
    EXPECT_TRUE(this->initSystemEquations());
  }
  virtual ~SAM1DOF() {}
};


// Dummy integrand class, for time integration scheme testing.
class Problem : public IntegrandBase
{
public:
  Problem() : IntegrandBase(1) {}
  virtual ~Problem() {}
  virtual void setIntegrationPrm(unsigned short int i, double p) { prm[i] = p; }
  virtual double getIntegrationPrm(unsigned short int i) const
  { return m_mode == SIM::DYNAMIC ? prm[i] : 0.0; }
  const double* getIntPrm() { return prm; }
private:
  double prm[5];
};


// Simulator wrapper implementing dummy versions of the pure virtual methods.
class SIMwrapper : public SIMoutput
{
public:
  SIMwrapper(IntegrandBase* p) : SIMoutput(p) {}
  virtual ~SIMwrapper() {}
  virtual unsigned short int getNoParamDim() const { return 0; }
  virtual bool addConstraint(int,int,int,int,int,int&,char)
  { return false; }
  virtual ASMbase* readPatch(std::istream&,int,const CharVec&) const
  { return nullptr; }
  virtual bool readPatches(std::istream&,SIMdependency::PatchVec&,const char*)
  { return false; }
  virtual ASMbase* createDefaultGeometry(const TiXmlElement*) const
  { return nullptr; }
};


// Simulator class for a single-DOF oscillator.
class SIM1DOF : public SIMwrapper
{
public:
  SIM1DOF(IntegrandBase* p) : SIMwrapper(p) { mySam = new SAM1DOF(); }
  virtual ~SIM1DOF() {}
  virtual bool assembleSystem(const TimeDomain& time,
                              const Vectors& prevSol,
                              bool newLHSmatrix, bool)
  {
    const double M = 10.0;   // Mass of the oscillator
    const double K = 1000.0; // Stiffness of the oscillator
    const double F = 1.0;    // External load (constant)

    myEqSys->initialize(newLHSmatrix);

    bool ok;
    if (myProblem->getMode() == SIM::MASS_ONLY) {
      ElmMats elm;
      elm.resize(1,1); elm.redim(1);
      elm.A[0].fill(M); // Mass Matrix
      ok = myEqSys->assemble(&elm,1);
    }
    else {
      const double* intPrm = static_cast<Problem*>(myProblem)->getIntPrm();
      NewmarkMats elm(intPrm[0],intPrm[1],intPrm[2],intPrm[3]);
      elm.resize(3,1); elm.redim(1); elm.vec.resize(3);
      elm.setStepSize(time.dt,0);
      elm.A[1].fill(M); // Mass matrix
      elm.A[2].fill(K); // Stiffness matrix
      elm.b[0] = -K*prevSol.front(); // Elastic forces
      for (int i = 0; i < 3; i++) elm.vec[i] = prevSol[i];
      ok = myEqSys->assemble(&elm,1);
    }

    // Add in the external load
    ok &= mySam->assembleSystem(*myEqSys->getVector(),&F,1);

    return ok && myEqSys->finalize(newLHSmatrix);
  }
};


// Newmark time integrator with numerical damping (alpha_H = -0.1).
class Newmark : public NewmarkSIM
{
public:
  Newmark(SIMbase& sim) : NewmarkSIM(sim)
  {
    beta = 0.3025; gamma = 0.6;
    this->initPrm();
    this->initSol(3);
  }
  virtual ~Newmark() {}
};


// Generalized-alpha time integrator with numerical damping (alpha_H = -0.1).
class GenAlpha : public GenAlphaSIM
{
public:
  GenAlpha(SIMbase& sim) : GenAlphaSIM(sim)
  {
    this->initPrm();
    this->initSol(3);
  }
  virtual ~GenAlpha() {}
};


void runSingleDof (SIMbase& model, NewmarkSIM& solver, double rtol = 0.5e-11)
{
  TimeStep tp;
  tp.time.dt = 0.01;
  tp.stopTime = 0.65;

  ASSERT_TRUE(model.initSystem(0));
  ASSERT_TRUE(solver.initAcc());
  EXPECT_FLOAT_EQ(solver.getAcceleration().front(),0.1);

  //               at t=0.1           at t=0.25         at t=0.5
  double u[3] = { 0.000457484252515, 0.00178698471292, 0.000732016593476 };
  double v[3] = { 0.008368445734720, 0.00592764975245,-0.009365075630580 };
  double a[3] = { 0.054251574748500,-0.07869847129160, 0.026798340652400 };

  while (solver.advanceStep(tp))
  {
    ASSERT_TRUE(solver.solveStep(tp) == SIM::CONVERGED);
    double dis = solver.getSolution().front();
    double vel = solver.getVelocity().front();
    double acc = solver.getAcceleration().front();

    // Check the response at three randomly selected time steps
    if (tp.step == 10) {
      EXPECT_NEAR(dis,u[0], (dis+u[0])*rtol);
      EXPECT_NEAR(vel,v[0], (vel+v[0])*rtol);
      EXPECT_NEAR(acc,a[0], (acc+a[0])*rtol);
    }
    else if (tp.step == 25) {
      EXPECT_NEAR(dis,u[1], (dis+u[1])*rtol);
      EXPECT_NEAR(vel,v[1], (vel+v[1])*rtol);
      EXPECT_NEAR(acc,a[1],-(acc+a[1])*rtol);
    }
    else if (tp.step == 50) {
      EXPECT_NEAR(dis,u[2], (dis+u[2])*rtol);
      EXPECT_NEAR(vel,v[2],-(vel+v[2])*rtol);
      EXPECT_NEAR(acc,a[2], (acc+a[2])*rtol);
    }
  }
}


TEST(TestNewmark, SingleDOF)
{
  SIM1DOF simulator(new Problem());
  Newmark integrator(simulator);
  runSingleDof(simulator,integrator);
}


TEST(TestGenAlpha, SingleDOF)
{
  SIM1DOF simulator(new Problem());
  GenAlpha integrator(simulator);
  runSingleDof(simulator,integrator,0.02);
}