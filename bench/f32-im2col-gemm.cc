// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <functional>
#include <random>
#include <vector>

#include <cpuinfo.h>

#include <benchmark/benchmark.h>
#include "bench/conv.h"
#include "bench/utils.h"
#include <xnnpack/AlignedAllocator.h>
#include <xnnpack/common.h>
#include <xnnpack/gemm.h>
#include <xnnpack/im2col.h>
#include <xnnpack/pack.h>
#include <xnnpack/params.h>
#include <xnnpack/requantization.h>


static void Im2ColGEMMBenchmark(benchmark::State& state,
  xnn_f32_gemm_ukernel_function sgemm,
  uint32_t mr, uint32_t nr, uint32_t kr, uint32_t sr)
{
  if (!cpuinfo_initialize()) {
    state.SkipWithError("cpuinfo initialization failed");
    return;
  }

  const size_t input_height = state.range(0);
  const size_t input_width = state.range(1);
  const size_t kernel_height = state.range(2);
  const size_t kernel_width = state.range(3);
  const size_t kernel_size = kernel_height * kernel_width;
  const size_t padding_height = state.range(4);
  const size_t padding_width = state.range(5);
  const size_t subsampling = state.range(6);
  const size_t dilation = state.range(7);
  const size_t group_input_channels = state.range(8);
  const size_t group_output_channels = state.range(9);

  std::random_device random_device;
  auto rng = std::mt19937(random_device());
  auto f32rng = std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f), rng);

  const size_t effective_kernel_height = (kernel_height - 1) * dilation + 1;
  const size_t effective_kernel_width = (kernel_width - 1) * dilation + 1;
  const size_t padding_left = padding_width / 2;
  const size_t padding_top = padding_height / 2;
  const size_t output_height = (input_height + padding_height - effective_kernel_height) / subsampling + 1;
  const size_t output_width = (input_width + padding_width - effective_kernel_width) / subsampling + 1;
  const size_t output_size = output_height * output_width;

  const size_t nc_stride = benchmark::utils::roundUp<size_t>(group_output_channels, nr);
  const size_t kc_stride = benchmark::utils::roundUp<size_t>(group_input_channels, kr);

  std::vector<float> a(input_height * input_width * group_input_channels);
  std::generate(a.begin(), a.end(), std::ref(f32rng));
  std::vector<float> k(group_output_channels * kernel_height * kernel_width * group_input_channels);
  std::generate(k.begin(), k.end(), std::ref(f32rng));
  std::vector<float> b(group_output_channels);
  std::generate(b.begin(), b.end(), std::ref(f32rng));

  const size_t w_elements = (kernel_size * kc_stride + 1) * nc_stride;
  const size_t c_elements = output_size * group_output_channels;
  const size_t num_buffers = 1 +
    benchmark::utils::divideRoundUp<size_t>(benchmark::utils::GetMaxCacheSize(),
      sizeof(float) * (w_elements + c_elements));

  std::vector<float, AlignedAllocator<float, 32>> w(w_elements * num_buffers);
  std::fill(w.begin(), w.end(), 0.0f);
  xnn_pack_f32_gemm_goi_w(1 /* groups */, group_output_channels, group_input_channels * kernel_size,
    nr, kr, sr, k.data(), b.data(), w.data());
  for (size_t n = 1; n < num_buffers; n++) {
    std::copy(w.cbegin(), w.cbegin() + w_elements, w.begin() + n * w_elements);
  }

  std::vector<float> im2col_buffer(output_size * group_input_channels * kernel_size * group_output_channels);

  std::vector<float> c(c_elements * num_buffers);
  std::fill(c.begin(), c.end(), std::nanf(""));

  xnn_f32_output_params output_params =
    xnn_compute_f32_output_params(-std::numeric_limits<float>::infinity(), +std::numeric_limits<float>::infinity());

  size_t buffer_index = 0;
  for (auto _ : state) {
    state.PauseTiming();
    benchmark::utils::prefetchToL1(a.data(), a.size() * sizeof(float));
    buffer_index = (buffer_index + 1) % num_buffers;
    state.ResumeTiming();

    const float* inputData = a.data();
    if (kernel_size != 1 || subsampling != 1) {
      xnn_im2col_conv2d(
        output_height, output_width,
        kernel_height, kernel_width,
        subsampling, subsampling,
        dilation, dilation,
        input_width, padding_top, padding_left,
        group_input_channels * sizeof(float) /* input channels */,
        group_input_channels * sizeof(float) /* input stride */,
        a.data(), im2col_buffer.data());
      inputData = im2col_buffer.data();
    }

    for (uint32_t m = 0; m < output_size; m += mr) {
      const uint32_t mb = min(output_size - m, mr);
      for (uint32_t n = 0; n < group_output_channels; n += nr) {
        const uint32_t nb = min(group_output_channels - n, nr);
        sgemm(
          mb, nb, kernel_size * group_input_channels * sizeof(float),
          inputData + m * kernel_size * group_input_channels, kernel_size * group_input_channels * sizeof(float),
          w.data() + (buffer_index * nc_stride + n) * (kernel_size * kc_stride + 1),
          c.data() + (buffer_index * output_size + m) * group_output_channels + n, group_output_channels * sizeof(float), nr * sizeof(float),
          &output_params);
      }
    }
  }

  state.counters["Freq"] = benchmark::utils::GetCurrentCpuFrequency();
  state.counters["FLOPS"] = benchmark::Counter(
    uint64_t(state.iterations()) * 2 *
      output_height * output_width *
      group_input_channels * group_output_channels *
      kernel_height * kernel_width,
    benchmark::Counter::kIsRate);
}


#if XNN_ARCH_ARM64 && XNN_ENABLE_ASSEMBLY
  static void sgemm_4x8__aarch64_neonfma_cortex_a75(benchmark::State& state, const char* net) {
    Im2ColGEMMBenchmark(state, xnn_f32_gemm_ukernel_4x8__aarch64_neonfma_cortex_a75, 4, 8, 1, 1);
  }

  BENCHMARK_CONV(sgemm_4x8__aarch64_neonfma_cortex_a75)
#endif  // XNN_ARCH_ARM64

#if !XNN_ARCH_WASM && !XNN_ARCH_ASMJS
  static void sgemm_6x8__psimd_loadsplat(benchmark::State& state, const char* net) {
    Im2ColGEMMBenchmark(state, xnn_f32_gemm_ukernel_6x8__psimd_loadsplat, 6, 8, 1, 1);
  }

  BENCHMARK_CONV(sgemm_6x8__psimd_loadsplat)
#endif  // !XNN_ARCH_WASM && !XNN_ARCH_ASMJS

static void sgemm_2x4__scalar(benchmark::State& state, const char* net) {
  Im2ColGEMMBenchmark(state, xnn_f32_gemm_ukernel_2x4__scalar, 2, 4, 1, 1);
}

static void sgemm_4x4__scalar(benchmark::State& state, const char* net) {
  Im2ColGEMMBenchmark(state, xnn_f32_gemm_ukernel_4x4__scalar, 4, 4, 1, 1);
}

BENCHMARK_CONV(sgemm_2x4__scalar)
BENCHMARK_CONV(sgemm_4x4__scalar)


#ifndef XNNPACK_BENCHMARK_NO_MAIN
BENCHMARK_MAIN();
#endif
