//------------------------------------------------------------------------------
// © 2021-2023. Triad National Security, LLC. All rights reserved.  This
// program was produced under U.S. Government contract 89233218CNA000001
// for Los Alamos National Laboratory (LANL), which is operated by Triad
// National Security, LLC for the U.S.  Department of Energy/National
// Nuclear Security Administration. All rights in the program are
// reserved by Triad National Security, LLC, and the U.S. Department of
// Energy/National Nuclear Security Administration. The Government is
// granted for itself and others acting on its behalf a nonexclusive,
// paid-up, irrevocable worldwide license in this material to reproduce,
// prepare derivative works, distribute copies to the public, perform
// publicly and display publicly, and to permit others to do so.
//------------------------------------------------------------------------------

#ifndef _SINGULARITY_EOS_EOS_EOS_JWL_HPP_
#define _SINGULARITY_EOS_EOS_EOS_JWL_HPP_

// stdlib
#include <cmath>
#include <cstdio>
#include <string>

// Ports-of-call
#include <ports-of-call/portability.hpp>

// Base stuff
#include <singularity-eos/base/constants.hpp>
#include <singularity-eos/base/eos_error.hpp>
#include <singularity-eos/base/root-finding-1d/root_finding.hpp>
#include <singularity-eos/eos/eos_base.hpp>

namespace singularity {

using namespace eos_base;

// COMMENT: This is meant to be an implementation of the "standard" JWL as
// implemented in xRAGE for eostype(1).  It does not include any energy shifting
class JWL : public EosBase<JWL> {
 public:
  JWL() = default;
  PORTABLE_INLINE_FUNCTION JWL(const Real A, const Real B, const Real R1, const Real R2,
                               const Real w, const Real rho0, const Real Cv)
      : _A(A), _B(B), _R1(R1), _R2(R2), _w(w), _rho0(rho0), _Cv(Cv) {}
  JWL GetOnDevice() { return *this; }
  PORTABLE_INLINE_FUNCTION Real TemperatureFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real InternalEnergyFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real PressureFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real PressureFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real EntropyFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real EntropyFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real SpecificHeatFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real SpecificHeatFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real BulkModulusFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real BulkModulusFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real GruneisenParamFromDensityTemperature(
      const Real rho, const Real temperature, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION Real GruneisenParamFromDensityInternalEnergy(
      const Real rho, const Real sie, Real *lambda = nullptr) const;
  PORTABLE_INLINE_FUNCTION void FillEos(Real &rho, Real &temp, Real &energy, Real &press,
                                        Real &cv, Real &bmod, const unsigned long output,
                                        Real *lambda = nullptr) const;
  // Generic functions provided by the base class. These contain e.g. the vector
  // overloads that use the scalar versions declared here
  SG_ADD_BASE_CLASS_USINGS(JWL)
  PORTABLE_INLINE_FUNCTION
  int nlambda() const noexcept { return 0; }
  PORTABLE_INLINE_FUNCTION
  void ValuesAtReferenceState(Real &rho, Real &temp, Real &sie, Real &press, Real &cv,
                              Real &bmod, Real &dpde, Real &dvdt,
                              Real *lambda = nullptr) const;
  static constexpr unsigned long PreferredInput() { return _preferred_input; }
  static inline unsigned long scratch_size(std::string method, unsigned int nelements) {
    return 0;
  }
  static inline unsigned long max_scratch_size(unsigned int nelements) { return 0; }
  PORTABLE_INLINE_FUNCTION void PrintParams() const {
    static constexpr char s1[]{"JWL Params: "};
    printf("%sA:%e B:%e R1: %e\nR2:%e w:%e rho0:%e\nCv:%e\n", s1, _A, _B, _R1, _R2, _w,
           _rho0, _Cv);
  }
  PORTABLE_INLINE_FUNCTION void
  DensityEnergyFromPressureTemperature(const Real press, const Real temp, Real *lambda,
                                       Real &rho, Real &sie) const;
  inline void Finalize() {}
  static std::string EosType() { return std::string("JWL"); }
  static std::string EosPyType() { return EosType(); }

 private:
  Real _A, _B, _R1, _R2, _w, _rho0, _Cv;
  PORTABLE_INLINE_FUNCTION Real ReferenceEnergy(const Real rho) const;
  PORTABLE_INLINE_FUNCTION Real ReferencePressure(const Real rho) const;
  // static constexpr const char _eos_type[] = "JWL";
  static constexpr const unsigned long _preferred_input =
      thermalqs::density | thermalqs::specific_internal_energy;
};

PORTABLE_INLINE_FUNCTION Real JWL::ReferencePressure(const Real rho) const {
  const Real x = _rho0 / rho;
  return _A * std::exp(-_R1 * x) + _B * std::exp(-_R2 * x);
}
PORTABLE_INLINE_FUNCTION Real JWL::ReferenceEnergy(const Real rho) const {
  const Real x = _rho0 / rho;
  return _A / (_rho0 * _R1) * std::exp(-_R1 * x) +
         _B / (_rho0 * _R2) * std::exp(-_R2 * x);
}
PORTABLE_INLINE_FUNCTION Real JWL::InternalEnergyFromDensityTemperature(
    const Real rho, const Real temp, Real *lambda) const {
  return ReferenceEnergy(rho) + _Cv * temp;
}
PORTABLE_INLINE_FUNCTION Real JWL::PressureFromDensityInternalEnergy(const Real rho,
                                                                     const Real sie,
                                                                     Real *lambda) const {
  return ReferencePressure(rho) + _w * rho * (sie - ReferenceEnergy(rho));
}
PORTABLE_INLINE_FUNCTION Real JWL::EntropyFromDensityInternalEnergy(const Real rho,
                                                                    const Real sie,
                                                                    Real *lambda) const {
  EntropyIsNotEnabled("JWL");
  return 1.0;
}
PORTABLE_INLINE_FUNCTION Real JWL::TemperatureFromDensityInternalEnergy(
    const Real rho, const Real sie, Real *lambda) const {
  return (sie - ReferenceEnergy(rho)) / _Cv;
}
PORTABLE_INLINE_FUNCTION Real JWL::SpecificHeatFromDensityInternalEnergy(
    const Real rho, const Real sie, Real *lambda) const {
  return _Cv;
}
PORTABLE_INLINE_FUNCTION Real JWL::BulkModulusFromDensityInternalEnergy(
    const Real rho, const Real sie, Real *lambda) const {
  const Real x = _rho0 / rho;
  // return
  // (_w+1)*(PressureFromDensityInternalEnergy(rho,sie)-ReferencePressure(rho))+x*(_A*_R1*std::exp(-_R1*x)+_B*_R2*std::exp(-_R2*x));
  return (_w + 1) * _w * rho * (sie - ReferenceEnergy(rho)) +
         x * (_A * _R1 * std::exp(-_R1 * x) + _B * _R2 * std::exp(-_R2 * x));
}
PORTABLE_INLINE_FUNCTION
Real JWL::GruneisenParamFromDensityInternalEnergy(const Real rho, const Real sie,
                                                  Real *lambda) const {
  return _w;
}
// Below are "unimplemented" routines
PORTABLE_INLINE_FUNCTION Real JWL::PressureFromDensityTemperature(const Real rho,
                                                                  const Real temp,
                                                                  Real *lambda) const {
  return PressureFromDensityInternalEnergy(
      rho, InternalEnergyFromDensityTemperature(rho, temp));
}
PORTABLE_INLINE_FUNCTION Real JWL::EntropyFromDensityTemperature(const Real rho,
                                                                 const Real temp,
                                                                 Real *lambda) const {
  EntropyIsNotEnabled("JWL");
  return 1.0;
}
PORTABLE_INLINE_FUNCTION Real JWL::SpecificHeatFromDensityTemperature(
    const Real rho, const Real temp, Real *lambda) const {
  return SpecificHeatFromDensityInternalEnergy(
      rho, InternalEnergyFromDensityTemperature(rho, temp));
}
PORTABLE_INLINE_FUNCTION Real JWL::BulkModulusFromDensityTemperature(const Real rho,
                                                                     const Real temp,
                                                                     Real *lambda) const {
  return BulkModulusFromDensityInternalEnergy(
      rho, InternalEnergyFromDensityTemperature(rho, temp));
}
PORTABLE_INLINE_FUNCTION
Real JWL::GruneisenParamFromDensityTemperature(const Real rho, const Real temp,
                                               Real *lambda) const {
  return _w;
}
PORTABLE_INLINE_FUNCTION void
JWL::DensityEnergyFromPressureTemperature(const Real press, const Real temp, Real *lambda,
                                          Real &rho, Real &sie) const {
  // sie = sie_r + cv*T;  Thus sie-sie_r = cv*T
  // Thus P = P_r +_w*rho*cv*T ==> Invertable?
  // Turns out not to be exactly invertible
  Real rhoguess = (rho < 1e-8) ? _rho0 : rho;
  auto PofRatT = [&](const Real r) { return _Cv * temp * r * _w + ReferencePressure(r); };
  using RootFinding1D::regula_falsi;
  using RootFinding1D::Status;
  RootFinding1D::RootCounts counts;
  auto status =
      regula_falsi(PofRatT, press, rhoguess, 1.0e-5, 1.0e3, 1.0e-8, 1.0e-8, rho, counts);
  if (status != Status::SUCCESS) {
    // Root finder failed even though the solution was bracketed... this is an error
    EOS_ERROR("JWL::DensityEnergyFromPressureTemperature: "
              "Root find failed to find a solution given P, T\n");
  }
  sie = InternalEnergyFromDensityTemperature(rho, temp);
}
PORTABLE_INLINE_FUNCTION void JWL::FillEos(Real &rho, Real &temp, Real &sie, Real &press,
                                           Real &cv, Real &bmod,
                                           const unsigned long output,
                                           Real *lambda) const {
  if (output & thermalqs::pressure) press = PressureFromDensityInternalEnergy(rho, sie);
  if (output & thermalqs::temperature)
    temp = TemperatureFromDensityInternalEnergy(rho, sie);
  if (output & thermalqs::bulk_modulus)
    bmod = BulkModulusFromDensityInternalEnergy(rho, sie);
  if (output & thermalqs::specific_heat)
    cv = SpecificHeatFromDensityInternalEnergy(rho, sie);
}

// TODO(JMM): pre-cache these rather than recomputing them each time
// TODO: Chad, please decide if STP is actually right here. Should it be
// based on the reference energy and pressure instead?
PORTABLE_INLINE_FUNCTION
void JWL::ValuesAtReferenceState(Real &rho, Real &temp, Real &sie, Real &press, Real &cv,
                                 Real &bmod, Real &dpde, Real &dvdt, Real *lambda) const {
  rho = _rho0;
  temp = ROOM_TEMPERATURE;
  sie = InternalEnergyFromDensityTemperature(rho, temp, lambda);
  press = ATMOSPHERIC_PRESSURE;
  cv = _Cv;
  bmod = BulkModulusFromDensityInternalEnergy(rho, sie, lambda);
  dpde = _w * _rho0;
  // TODO: chad please fix this one for me. This is wrong.
  Real gm1 = GruneisenParamFromDensityInternalEnergy(rho, sie, lambda) * rho;
  dvdt = gm1 * cv / bmod;
}

} // namespace singularity

#endif // _SINGULARITY_EOS_EOS_EOS_JWL_HPP_
