#include <ceed.h>
#include "../include/structs.h"
#include "../include/setup-libceed.h"
#include "../problems/problems.h"
#include "../problems/neo-hookean.h"
#include "../qfunctions/common.h"
#include "../qfunctions/finite-strain-neo-hookean-current-1.h"

static const char *const field_names[] = {"gradu"};
static CeedInt field_sizes[] = {9};

ProblemData finite_strain_neo_Hookean_current_1 = {
  .setup_geo = SetupGeo,
  .setup_geo_loc = SetupGeo_loc,
  .q_data_size = 10,
  .quadrature_mode = CEED_GAUSS,
  .residual = ElasFSCurrentNH1F,
  .residual_loc = ElasFSCurrentNH1F_loc,
  .number_fields_stored = 1,
  .field_names = field_names,
  .field_sizes = field_sizes,
  .jacobian = ElasFSCurrentNH1dF,
  .jacobian_loc = ElasFSCurrentNH1dF_loc,
  .energy = ElasFSCurrentNH1Energy,
  .energy_loc = ElasFSCurrentNH1Energy_loc,
  .diagnostic = ElasFSCurrentNH1Diagnostic,
  .diagnostic_loc = ElasFSCurrentNH1Diagnostic_loc,
};

PetscErrorCode SetupLibceedFineLevel_ElasFSCurrentNH1(DM dm, DM dm_energy,
    DM dm_diagnostic, Ceed ceed, AppCtx app_ctx, CeedQFunctionContext phys_ctx,
    PetscInt fine_level, PetscInt num_comp_u, PetscInt U_g_size,
    PetscInt U_loc_size, CeedVector force_ceed, CeedVector neumann_ceed,
    CeedData *data) {
  PetscErrorCode ierr;

  PetscFunctionBegin;

  ierr = SetupLibceedFineLevel(dm, dm_energy, dm_diagnostic, ceed, app_ctx,
                               phys_ctx, finite_strain_neo_Hookean_current_1,
                               fine_level, num_comp_u, U_g_size, U_loc_size,
                               force_ceed, neumann_ceed, data); CHKERRQ(ierr);

  PetscFunctionReturn(0);
};

PetscErrorCode SetupLibceedLevel_ElasFSCurrentNH1(DM dm, Ceed ceed,
    AppCtx app_ctx, PetscInt level, PetscInt num_comp_u, PetscInt U_g_size,
    PetscInt U_loc_size, CeedVector fine_mult, CeedData *data) {
  PetscErrorCode ierr;

  PetscFunctionBegin;

  ierr = SetupLibceedLevel(dm, ceed, app_ctx,
                           finite_strain_neo_Hookean_current_1,
                           level, num_comp_u, U_g_size, U_loc_size, fine_mult, data);
  CHKERRQ(ierr);

  PetscFunctionReturn(0);
};
