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

#ifndef _ceed_cuda_gen_h
#define _ceed_cuda_gen_h

#include <ceed/ceed.h>
#include <ceed/backend.h>
#include <cuda.h>
#include "../cuda/ceed-cuda-common.h"

typedef struct { const CeedScalar *in[16]; CeedScalar *out[16]; } CudaFields;
typedef struct { const float *in[16]; float *out[16]; } CudaFields_float;
typedef struct { CeedInt *in[16]; CeedInt *out[16]; } CudaFieldsInt;

typedef struct {
  CeedInt dim;
  CeedInt Q1d;
  CeedInt maxP1d;
  CUmodule module;
  CUfunction op;
  CudaFieldsInt indices;
  CudaFields fields;
  CudaFields_float B;
  CudaFields_float G;
  float *W;
} CeedOperator_Cuda_gen;

typedef struct {
  char *qFunctionName;
  char *qFunctionSource;
  void *d_c;
} CeedQFunction_Cuda_gen;

CEED_INTERN int CeedQFunctionCreate_Cuda_gen(CeedQFunction qf);

CEED_INTERN int CeedOperatorCreate_Cuda_gen(CeedOperator op);

CEED_INTERN int CeedCompositeOperatorCreate_Cuda_gen(CeedOperator op);

#endif // _ceed_cuda_gen_h
