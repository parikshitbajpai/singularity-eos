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

#ifdef SINGULARITY_TEST_HELMHOLTZ

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <array>
#include <limits>
#include <string>

#ifndef CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"
#endif

#include <ports-of-call/portability.hpp>
#include <ports-of-call/portable_arrays.hpp>
#include <ports-of-call/portable_errors.hpp>
#include <singularity-eos/base/constants.hpp>
#include <singularity-eos/eos/eos.hpp>
#include <test/eos_unit_test_helpers.hpp>

using Catch::Matchers::WithinRel;
using singularity::Helmholtz;
const std::string filename = "../test/helmholtz/helm_table.dat";
SCENARIO("Helmholtz equation of state - Table interpolation (tgiven)", "[HelmholtzEOS]") {
  GIVEN("A Helmholtz EOS") {
    /* We only test the EOS without Coulomb corrections since those are
       implemented in a different way in the reference implementation (cutoff
       vs. butterworth filter)*/
    Helmholtz eos(filename, true, true, false, true, true);
    THEN("We loaded the file!") { REQUIRE(true); }

    /* Density and temperature range evenly sampling the parameter space */
    Real rho_in[4] = {1e-3, 1e1, 1e5, 1e9};
    Real temp_in[4] = {1e4, 1e6, 1e8, 1e10};

    /* Reference values calculated with reference implementation, using
        abar = 4.0, zbar = 2.0 */
    Real ein_ref[16] = {
        9.4224752079613794e+11, 1.0111770497124719e+14, 7.5658628444001809e+20,
        2.0453379618698803e+29, 2.2331233357586930e+13, 1.0034546568448442e+14,
        8.5141422479277632e+16, 2.0453379683965599e+25, 9.9766457026104000e+15,
        1.0007780352178464e+16, 1.5481955850958990e+16, 2.0454039730394418e+21,
        1.2565634708891441e+18, 1.2565943427230589e+18, 1.2597246289336612e+18,
        2.0378412737252767e+18,
    };
    Real press_ref[16] = {
        6.2816411519633424e+08, 6.4881120328592972e+10, 2.5219850299857830e+17,
        6.6317830432079092e+25, 1.4886708268563091e+14, 6.6886462284289912e+14,
        3.1455537853992173e+17, 6.6317832510490260e+25, 6.4994313346826541e+20,
        6.5201769591383707e+20, 1.0064031069579145e+21, 6.6338640383060858e+25,
        4.8618139246583649e+26, 4.8620197225944188e+26, 4.8827453987382303e+26,
        8.5239855740764875e+26,
    };
    Real cv_ref[16] = {
        9.3196340815867558e+07, 1.2382711876365747e+08, 3.0263168156431395e+13,
        8.2533852062378803e+19, 3.3503794880269445e+07, 9.0284767991987824e+07,
        3.1224051937032824e+09, 8.2533852374140800e+15, 3.1184609621499959e+07,
        3.1713660547591802e+07, 7.3637607321062773e+07, 8.2536953970987695e+11,
        3.1176951501280103e+07, 3.1187974351097498e+07, 3.2050127401531246e+07,
        1.3551496573399475e+08,
    };
    Real bulkmod_ref[16] = {
        1.0469389563134364e+09, 1.0474497395736888e+11, 3.3626568149804531e+17,
        8.8885495148523153e+25, 2.4810139724448831e+14, 1.1146333619227326e+15,
        4.2981167651334662e+17, 8.8885498255318413e+25, 1.0640009518727505e+21,
        1.0674571600035824e+21, 1.6449806425795102e+21, 8.8916598362805109e+25,
        6.5290499527564281e+26, 6.5293929349629153e+26, 6.5637946470451621e+26,
        1.1836643294434592e+27,
    };
    Real gruen_ref[16] = {
        6.6666468879298924e-01, 5.8505924930223907e-01, 3.3333433537355273e-01,
        3.2817038792737097e-01, 6.6665979201944436e-01, 6.6645401339924293e-01,
        3.4304497738846229e-01, 3.2817038920598574e-01, 6.6665974133874983e-01,
        6.6598496353702430e-01, 6.3848346162175873e-01, 3.2818317808916397e-01,
        6.6668862862725131e-01, 6.6657508247347175e-01, 6.5774854337088673e-01,
        4.1219845464736737e-01,
    };

    /* Compare test values. Difference should be less than 1e-10 */
    Real lambda[2] = {4.0, 2.0};
    int k = 0;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        Real ein =
            eos.InternalEnergyFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        Real press = eos.PressureFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        Real cv = eos.SpecificHeatFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        Real bulkmod =
            eos.BulkModulusFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        Real gruen =
            eos.GruneisenParamFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        REQUIRE_THAT(ein, WithinRel(ein_ref[k], 1e-10));
        REQUIRE_THAT(press, WithinRel(press_ref[k], 1e-10));
        /* These values are not very accurate, but the difference is
           still less than 1e-6 in most cases. */
        REQUIRE_THAT(cv, WithinRel(cv_ref[k], 1e-6));
        REQUIRE_THAT(bulkmod, WithinRel(bulkmod_ref[k], 1e-8));
        REQUIRE_THAT(gruen, WithinRel(gruen_ref[k], 1e-6));
        k++;
      }
    }
  }
}

SCENARIO("Helmholtz equation of state - Root finding (egiven)", "[HelmholtzEOS]") {
  GIVEN("A helmholtz EOS") {
    /* Since the reference implementation uses a different root finding algorithm
       than the one used in the test implementation (Newton-Raphson vs. regula falsi),
       we check for internal consistency of the root finding algorithm instead of
       comparing the results to the reference implementation. */

    Helmholtz eos(filename, true, true, false, true, true);
    THEN("We loaded the file!") { REQUIRE(true); }

    /* Density and temperature range evenly sampling the parameter space */
    Real rho_in[4] = {1e-3, 1e1, 1e5, 1e9};
    Real temp_in[4] = {1e4, 1e6, 1e8, 1e10};

    /* Reference values computed with the reference implementation */
    Real ein_ref[16] = {
        9.4224752079613794e+11, 1.0111770497124719e+14, 7.5658628444001809e+20,
        2.0453379618698803e+29, 2.2331233357586930e+13, 1.0034546568448442e+14,
        8.5141422479277632e+16, 2.0453379683965599e+25, 9.9766457026104000e+15,
        1.0007780352178464e+16, 1.5481955850958990e+16, 2.0454039730394418e+21,
        1.2565634708891441e+18, 1.2565943427230589e+18, 1.2597246289336612e+18,
        2.0378412737252767e+18,
    };

    /* Compare test values. Difference should be less than 1e-6 */
    Real lambda[2] = {4.0, 2.0};
    int k = 0;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        /* We only need to check that the temperature returned by the root finding
           algorithm is consistent with the input temperature. If this is correct,
           other quantities will be correct if the table interpolation works
           correctly. The check of the internal energy is only here as an
           additional layer of consistency, but not strictly necessary. */
        Real ein =
            eos.InternalEnergyFromDensityTemperature(rho_in[i], temp_in[j], lambda);
        /* Independent check of the table interpolation in case the table interpolation
           check does not fail already. */
        REQUIRE_THAT(ein, WithinRel(ein_ref[k], 1e-10));
        Real temp_new = eos.TemperatureFromDensityInternalEnergy(rho_in[i], ein, lambda);
        Real ein_new =
            eos.InternalEnergyFromDensityTemperature(rho_in[i], temp_new, lambda);
        REQUIRE_THAT(temp_new, WithinRel(temp_in[j], 1e-10));
        REQUIRE_THAT(ein_new, WithinRel(ein, 1e-10));
        k++;
      }
    }
  }
}

#endif // SINGULARITY_TEST_HELMHOLTZ
