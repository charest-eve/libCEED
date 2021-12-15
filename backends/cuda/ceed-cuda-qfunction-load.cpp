#include <ceed/ceed.h>
#include <ceed/backend.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include "ceed-cuda.h"

static const char *qReadWrite = QUOTE(
template <int SIZE>
//------------------------------------------------------------------------------
// Read from quadrature points
//------------------------------------------------------------------------------
inline __device__ void readQuads(const CeedInt quad, const CeedInt nquads, const CeedScalar* d_u, CeedScalar* r_u) {
  for(CeedInt comp = 0; comp < SIZE; ++comp) {
    r_u[comp] = d_u[quad + nquads * comp];
  }
}

//------------------------------------------------------------------------------
// Write at quadrature points
//------------------------------------------------------------------------------
template <int SIZE>
inline __device__ void writeQuads(const CeedInt quad, const CeedInt nquads, const CeedScalar* r_v, CeedScalar* d_v) {
  for(CeedInt comp = 0; comp < SIZE; ++comp) {
    d_v[quad + nquads * comp] = r_v[comp];
  }
}
);

//------------------------------------------------------------------------------
// Build QFunction kernel
//------------------------------------------------------------------------------
extern "C" int CeedCudaBuildQFunction(CeedQFunction qf) {
  CeedInt ierr;
  using std::ostringstream;
  using std::string;
  Ceed ceed;
  CeedQFunctionGetCeed(qf, &ceed);
  CeedQFunction_Cuda *data;
  ierr = CeedQFunctionGetData(qf, (void **)&data); CeedChkBackend(ierr);
  // QFunction is built
  if (data->qFunction)
    return CEED_ERROR_SUCCESS;
  if (!data->qFunctionSource)
    return CeedError(ceed, CEED_ERROR_BACKEND, "No QFunction source or CUfunction provided.");

  // QFunction kernel generation
  CeedInt numinputfields, numoutputfields, size;
  CeedQFunctionField *qfinputfields, *qfoutputfields;
  ierr = CeedQFunctionGetFields(qf, &numinputfields, &qfinputfields, &numoutputfields, &qfoutputfields);
  CeedChkBackend(ierr);

  // Build strings for final kernel
  string qFunction(data->qFunctionSource);
  string qReadWriteS(qReadWrite);
  ostringstream code;
  string qFunctionName(data->qFunctionName);
  string kernelName;
  kernelName = "CeedKernel_Cuda_ref_" + qFunctionName;

  // Defintions
  code << "\n#define CEED_QFUNCTION(name) inline __device__ int name\n";
  code << "#define CEED_QFUNCTION_HELPER inline __device__\n";
  code << "#define CeedPragmaSIMD\n";
  code << "#define CEED_ERROR_SUCCESS 0\n";
  code << "#define CEED_Q_VLA 1\n\n";
  code << "typedef struct { const CeedScalar* inputs[16]; CeedScalar* outputs[16]; } Fields_Cuda;\n";
  code << qReadWriteS;
  code << qFunction;
  code << "extern \"C\" __global__ void " << kernelName << "(void *ctx, CeedInt Q, Fields_Cuda fields) {\n";

  // Inputs
  for (CeedInt i = 0; i < numinputfields; i++) {
    code << "// Input field "<<i<<"\n";
    ierr = CeedQFunctionFieldGetSize(qfinputfields[i], &size); CeedChkBackend(ierr);
    code << "  const CeedInt size_in_"<<i<<" = "<<size<<";\n";
    code << "  CeedScalar r_q"<<i<<"[size_in_"<<i<<"];\n";
  }

  // Outputs
  for (CeedInt i = 0; i < numoutputfields; i++) {
    code << "// Output field "<<i<<"\n";
    ierr = CeedQFunctionFieldGetSize(qfoutputfields[i], &size); CeedChkBackend(ierr);
    code << "  const CeedInt size_out_"<<i<<" = "<<size<<";\n";
    code << "  CeedScalar r_qq"<<i<<"[size_out_"<<i<<"];\n";
  }

  // Setup input/output arrays
  code << "  const CeedScalar* in["<<numinputfields<<"];\n";
  for (CeedInt i = 0; i < numinputfields; i++) {
    code << "    in["<<i<<"] = r_q"<<i<<";\n";
  }
  code << "  CeedScalar* out["<<numoutputfields<<"];\n";
  for (CeedInt i = 0; i < numoutputfields; i++) {
    code << "    out["<<i<<"] = r_qq"<<i<<";\n";
  }

  // Loop over quadrature points
  code << "  for (CeedInt q = blockIdx.x * blockDim.x + threadIdx.x; q < Q; q += blockDim.x * gridDim.x) {\n";

  // Load inputs
  for (CeedInt i = 0; i < numinputfields; i++) {
    code << "// Input field "<<i<<"\n";
    code << "  readQuads<size_in_"<<i<<">(q, Q, fields.inputs["<<i<<"], r_q"<<i<<");\n";
  }
  // QFunction
  code << "// QFunction\n";
  code << "    "<<qFunctionName<<"(ctx, 1, in, out);\n";

  // Write outputs
  for (CeedInt i = 0; i < numoutputfields; i++) {
    code << "// Output field "<<i<<"\n";
    code << "  writeQuads<size_out_"<<i<<">(q, Q, r_qq"<<i<<", fields.outputs["<<i<<"]);\n";
  }
  code << "  }\n";
  code << "}\n";

  // View kernel for debugging
  CeedDebug(ceed, code.str().c_str());

  // Compile kernel
  ierr = CeedCompileCuda(ceed, code.str().c_str(), &data->module, 0);
  CeedChkBackend(ierr);
  ierr = CeedGetKernelCuda(ceed, data->module, kernelName.c_str(), &data->qFunction);
  CeedChkBackend(ierr);

  // Cleanup
  ierr = CeedFree(&data->qFunctionSource); CeedChkBackend(ierr);
  return CEED_ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
