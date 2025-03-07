// Copyright (c) Facebook, Inc. and its affiliates.
// All rights reserved.
//
// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <xnnpack/params.h>
#include <xnnpack/common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(fn_name) \
  XNN_INTERNAL void fn_name(                                 \
    size_t channels,                                         \
    size_t output_width,                                     \
    const float** input,                                     \
    const float* weights,                                    \
    float* output,                                           \
    size_t input_stride,                                     \
    size_t output_increment,                                 \
    const union xnn_f32_output_params* params);

DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up1x25__scalar)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up1x4__scalar)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up1x9__scalar)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x25__psimd)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x25__sse)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x4__psimd)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x4__sse)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__aarch64_neonfma)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__aarch64_neonfma_cortex_a55)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__neon)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__neonfma)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__psimd)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up4x9__sse)
DECLARE_F32_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_f32_dwconv_ukernel_up8x9__neonfma)


#define DECLARE_Q8_DWCONV_UNIPASS_UKERNEL_FUNCTION(fn_name) \
  XNN_INTERNAL void fn_name(                                \
    size_t channels,                                        \
    size_t output_width,                                    \
    const uint8_t** input,                                  \
    const void* weights,                                    \
    uint8_t* output,                                        \
    size_t input_stride,                                    \
    size_t output_increment,                                \
    const union xnn_q8_gemm_params* params);

DECLARE_Q8_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_q8_dwconv_ukernel_up1x9__scalar)
DECLARE_Q8_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_q8_dwconv_ukernel_up8x9__aarch32_neon)
DECLARE_Q8_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_q8_dwconv_ukernel_up8x9__neon)
DECLARE_Q8_DWCONV_UNIPASS_UKERNEL_FUNCTION(xnn_q8_dwconv_ukernel_up8x9__sse2)


#define DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(fn_name) \
  XNN_INTERNAL void fn_name(                               \
    size_t m,                                              \
    size_t n,                                              \
    const float* input,                                    \
    const float* weights,                                  \
    float* output,                                         \
    size_t input_tuple_stride,                             \
    size_t output_tuple_stride,                            \
    size_t input_height_stride,                            \
    size_t output_height_stride,                           \
    const union xnn_f32_spchw_params* params);

DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_3x3p1__neonfma)
DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_5x5p2__neonfma)
DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_3x3p1__sse)
DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_3x3s2p1__neonfma)
DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_5x5s2p2__neonfma)
DECLARE_F32_DWCONV_SPCHW_UKERNEL_FUNCTION(xnn_f32_dwconv_spchw_ukernel_3x3s2p1__sse)


#ifdef __cplusplus
}  // extern "C"
#endif
