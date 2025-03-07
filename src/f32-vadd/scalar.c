// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <xnnpack/common.h>
#include <xnnpack/math.h>
#include <xnnpack/vadd.h>


void xnn_f32_vadd_ukernel__scalar(
    size_t n,
    const float* a,
    const float* b,
    float* y,
    const union xnn_f32_output_params params[restrict static 1])
{
  assert(n != 0);
  assert(n % sizeof(float) == 0);

  const float vy_min = params->scalar.min;
  const float vy_max = params->scalar.max;

  for (; n >= 2 * sizeof(float); n -= 2 * sizeof(float)) {
    const float va0 = a[0];
    const float va1 = a[1];
    a += 2;

    const float vb0 = b[0];
    const float vb1 = b[1];
    b += 2;

    float vy0 = va0 + vb0;
    float vy1 = va1 + vb1;
    vy0 = math_max_f32(vy0, vy_min);
    vy1 = math_max_f32(vy1, vy_min);
    vy0 = math_min_f32(vy0, vy_max);
    vy1 = math_min_f32(vy1, vy_max);

    y[0] = vy0;
    y[1] = vy1;
    y += 2;
  }
  if XNN_UNLIKELY(n != 0) {
    const float va = *a;
    const float vb = *b;
    float vy = va + vb;
    vy = math_max_f32(vy, vy_min);
    vy = math_min_f32(vy, vy_max);
    *y = vy;
  }
}
