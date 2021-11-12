// Copyright (c) 2017-2018, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory. LLNL-CODE-734707.
// All Rights reserved. See files LICENSE and NOTICE for details.
//
// This file is part of CEED, a collection of benchmarks, miniapps, software
// libraries and APIs for efficient high-order finite element and spectral
// element discretizations for exascale applications. For more information and
// source code availability see http://github.com/ceed.
//
// The CEED research is supported by the Exascale Computing Project 17-SC-20-SC,
// a collaborative effort of two U.S. Department of Energy organizations (Office
// of Science and the National Nuclear Security Administration) responsible for
// the planning and preparation of a capable exascale ecosystem, including
// software, applications, hardware, advanced system engineering and early
// testbed platforms, in support of the nation's exascale computing imperative.

#include <ceed/ceed.h>
#include <ceed/backend.h>
#include "ceed-opt.h"

//------------------------------------------------------------------------------
// Tensor Contract Blocked
//------------------------------------------------------------------------------
int CeedTensorContractApply_Blocked_Opt(CeedTensorContract contract, CeedInt A,
                                        CeedInt B, CeedInt C, CeedInt J,
                                        const CeedScalar *restrict t,
                                        CeedTransposeMode t_mode, const CeedInt add,
                                        const CeedScalar *restrict u,
                                        CeedScalar *restrict v, const CeedInt CC) {
  CeedInt t_stride_0 = B, t_stride_1 = 1;
  if (t_mode == CEED_TRANSPOSE) {
    t_stride_0 = 1; t_stride_1 = J;
  }

  for (CeedInt a=0; a<A; a++)
    for (CeedInt b=0; b<B; b++)
      for (CeedInt j=0; j<J; j++) {
        CeedScalar tq = t[j*t_stride_0 + b*t_stride_1];
        for (CeedInt c=0; c<(C/CC)*CC; c+=CC) // unroll
          for (CeedInt cc=0; cc<CC; cc++)
            v[(a*J+j)*C+(c+cc)] += tq * u[(a*B+b)*C+(c+cc)];
        CeedInt c = (C/CC)*CC;
        for (CeedInt cc=0; cc<C-c; c++)
          v[(a*J+j)*C+(c+cc)] += tq * u[(a*B+b)*C+(c+cc)];
      }

  return CEED_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// Tensor Contract Serial
//------------------------------------------------------------------------------
int CeedTensorContractApply_Serial_Opt(CeedTensorContract contract, CeedInt A,
                                       CeedInt B, CeedInt C, CeedInt J,
                                       const CeedScalar *restrict t,
                                       CeedTransposeMode t_mode, const CeedInt add,
                                       const CeedScalar *restrict u,
                                       CeedScalar *restrict v, const CeedInt JJ) {
  CeedInt t_stride_0 = B, t_stride_1 = 1;
  if (t_mode == CEED_TRANSPOSE) {
    t_stride_0 = 1; t_stride_1 = J;
  }

  for (CeedInt a=0; a<A; a++)
    for (CeedInt b=0; b<B; b++) {
      for (CeedInt j=0; j<(J/JJ)*JJ; j+=JJ)
        for (CeedInt jj=0; jj<JJ; jj++)// unroll
          v[a*J+(j+jj)] += t[(j+jj)*t_stride_0 + b*t_stride_1] * u[a*B+b];
      CeedInt j = (J/JJ)*JJ;
      for (CeedInt jj=0; jj<J-j; jj++)
        v[a*J+(j+jj)] += t[(j+jj)*t_stride_0 + b*t_stride_1] * u[a*B+b];
    }

  return CEED_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// Tensor Contract - Common Sizes
//------------------------------------------------------------------------------
static int CeedTensorContractApply_Blocked_Opt_8(CeedTensorContract contract,
    CeedInt A, CeedInt B, CeedInt C, CeedInt J, const CeedScalar *restrict t,
    CeedTransposeMode t_mode, const CeedInt add, const CeedScalar *restrict u,
    CeedScalar *restrict v) {
  return CeedTensorContractApply_Blocked_Opt(contract, A, B, C, J, t, t_mode, add,
         u,
         v, 8);
}

static int CeedTensorContractApply_Serial_Opt_8(CeedTensorContract contract,
    CeedInt A, CeedInt B, CeedInt C, CeedInt J, const CeedScalar *restrict t,
    CeedTransposeMode t_mode, const CeedInt add, const CeedScalar *restrict u,
    CeedScalar *restrict v) {
  return CeedTensorContractApply_Serial_Opt(contract, A, B, C, J, t, t_mode, add,
         u, v, 8);
}

//------------------------------------------------------------------------------
// Tensor Contract Apply
//------------------------------------------------------------------------------
int CeedTensorContractApply_Opt(CeedTensorContract contract, CeedInt A,
                                CeedInt B, CeedInt C, CeedInt J,
                                const CeedScalar *restrict t,
                                CeedTransposeMode t_mode, const CeedInt add,
                                const CeedScalar *restrict u,
                                CeedScalar *restrict v) {
  if (!add)
    for (CeedInt q=0; q<A*J*C; q++)
      v[q] = (CeedScalar) 0.0;

  if (C == 1)
    return CeedTensorContractApply_Serial_Opt_8(contract, A, B, C, J, t, t_mode,
           add, u, v);
  else
    return CeedTensorContractApply_Blocked_Opt_8(contract, A, B, C, J, t, t_mode,
           add, u, v);

  return CEED_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// Tensor Contract Destroy
//------------------------------------------------------------------------------
static int CeedTensorContractDestroy_Opt(CeedTensorContract contract) {
  return CEED_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// Tensor Contract Create
//------------------------------------------------------------------------------
int CeedTensorContractCreate_Opt(CeedBasis basis, CeedTensorContract contract) {
  int ierr;
  Ceed ceed;
  ierr = CeedTensorContractGetCeed(contract, &ceed); CeedChkBackend(ierr);

  ierr = CeedSetBackendFunction(ceed, "TensorContract", contract, "Apply",
                                CeedTensorContractApply_Opt); CeedChkBackend(ierr);
  ierr = CeedSetBackendFunction(ceed, "TensorContract", contract, "Destroy",
                                CeedTensorContractDestroy_Opt); CeedChkBackend(ierr);

  return CEED_ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
