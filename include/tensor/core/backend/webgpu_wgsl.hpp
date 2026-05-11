// SPDX-License-Identifier: MIT
//
// tensor::core::backend::webgpu::wgsl — WGSL kernel source strings.
//
// Phase 3 P3.M3.1 (per the addendum in docs/impl-plans/2026-05-11_phase-3-webgpu.md).
// This header holds the WGSL source for the WebGPU adapter's compute kernels
// as `constexpr std::string_view` constants. The kernels are written in the
// `{{workgroupSize}}` / `{{precision}}` templated form; the project's own
// `tensor::core::backend::webgpu::detail::substitute_wgsl` in
// `webgpu_detail/dispatch.hpp` performs the substitution at dispatch time
// before the source is passed to Dawn via `wgpu::Device::CreateShaderModule`.
// The templating convention originated with gpu.cpp's `KernelCode` class;
// per ADR-0016 the project does not use gpu.cpp itself, but the syntax is
// kept for legibility and continuity with the design docs.
//
// The dispatch wiring that feeds these strings into Dawn lives in
// `webgpu_detail/dispatch.hpp` (compile + bind + submit + wait); the
// per-operator pseudo-code is documented in
// `docs/detailed-design/webgpu-element-wise-kernels.md`. As of P3.M3.2
// (PR #60) the 8 element-wise methods on `webgpu::Backend` run real
// compute on the GPU for `float`; other dtypes delegate to reference
// per ADR-0012.
//
// Per ADR-0013 (canonical-reference framing), these kernel sources are
// themselves a citable artifact — they are how the named-axis algebra
// projects onto WebGPU's WGSL surface, with one kernel per element-wise
// binary operator.

#pragma once

#include <string_view>

namespace tensor::core::backend::webgpu::wgsl {

// Element-wise binary kernels — one shader per operator.
//
// All four follow the same shape:
//   - inputs: read-only storage buffers `a`, `b`, sized N (the tensor's
//     flat extent).
//   - output: read-write storage buffer `out`, also sized N.
//   - workgroup is 1-D (`{{workgroupSize}}` threads on the x-axis);
//     totalWorkgroups = ceil(N / workgroupSize).
//   - `{{precision}}` is f32 in the Phase 3 MVP per ADR-0012; f16 is a
//     follow-up that WGSL's `enable f16;` prelude supports.
//
// The bodies are intentionally one-liners. Phase 4+ may add fused
// kernels (e.g. `a * b + c`); they would live alongside these.

inline constexpr std::string_view kAddF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] + b[i];
}
)WGSL";

inline constexpr std::string_view kSubF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] - b[i];
}
)WGSL";

inline constexpr std::string_view kMulF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] * b[i];
}
)WGSL";

inline constexpr std::string_view kDivF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] / b[i];
}
)WGSL";

// Element-wise unary kernels — one shader per activation / sign operation.
//
// Same shape as the binary kernels above, but with two storage bindings
// instead of three:
//   - input:  read-only storage buffer `a`, sized N.
//   - output: read-write storage buffer `out`, also sized N.
//
// Mapped to `KernelBackend`'s unary methods: `exp`, `log`, `relu`, `neg`.
// These are the activations Phase 2 autograd already supports on the
// reference + Eigen adapters; P3.M3.2 swaps them to GPU dispatch.

inline constexpr std::string_view kExpF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = exp(a[i]);
}
)WGSL";

inline constexpr std::string_view kLogF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = log(a[i]);
}
)WGSL";

// ReLU expressed via `max(a, 0)` rather than a branch; the WGSL `max`
// built-in maps to a single GPU instruction on every Dawn-supported
// backend (Vulkan, Metal, D3D12). The literal `0.0` is implicitly the
// active precision because the operand type drives it.
inline constexpr std::string_view kReluF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = max(a[i], 0.0);
}
)WGSL";

inline constexpr std::string_view kNegF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = -a[i];
}
)WGSL";

// Default workgroup size for the element-wise kernels above. 256 is the
// canonical Dawn / WebGPU choice — fits in a single subgroup on most
// modern GPUs and balances occupancy against shared-memory pressure.
inline constexpr std::size_t kDefaultWorkgroupSize = 256;

// ─── Broadcast element-wise kernels (P3.M5) ────────────────────────────────
//
// Generalised element-wise binary kernels that consume a `BroadcastPlan`
// in the form of a uniform Params buffer. Used by `broadcast_add` /
// `broadcast_sub` / `broadcast_mul` (the Einstein-style broadcast that
// `a_i + b_j → c_{ij}` etc).
//
// Bindings:
//   0 = a    (storage, read-only)
//   1 = b    (storage, read-only)
//   2 = out  (storage, read-write)
//   3 = uniform `BroadcastParams`
//
// BroadcastParams (mirrors detail::BroadcastParams in dispatch.hpp):
//   result_rank, a_rank, b_rank, _pad      (16 bytes header)
//   result_extents: array<u32, 8>           (32 bytes)
//   a_extents:      array<u32, 8>
//   b_extents:      array<u32, 8>
//   a_source:       array<u32, 8>           (npos sentinel = 0xFFFFFFFF)
//   b_source:       array<u32, 8>
//
// Max supported rank is 8 — well above the project's typical rank ≤ 4
// shapes. ADR-0012 §Decision Outcome point 5 says non-float delegates
// to reference; the same applies here.
//
// Algorithm per thread:
//   gid.x = result flat index
//   delinearize to result multi-index using result_extents (row-major)
//   project to a multi-index via a_source (npos → 0)
//   linearize a multi-index using a_extents → a flat
//   same for b
//   out[gid.x] = a[a_flat] <op> b[b_flat]
//
// The pattern uses `kBroadcastBodyF32` as a shared body string with the
// operator substituted; each operator-specific constant prepends the
// per-op header to keep the constants self-contained for users who read
// just one source.

namespace detail_wgsl {
// Shared body. Each per-op constant prepends a one-line header that
// renames `OP` via WGSL macro-like text substitution.
inline constexpr std::string_view kBroadcastBodyF32 = R"WGSL(
struct BroadcastParams {
    result_rank    : u32,
    a_rank         : u32,
    b_rank         : u32,
    padding        : u32,
    result_extents : array<u32, 8>,
    a_extents      : array<u32, 8>,
    b_extents      : array<u32, 8>,
    a_source       : array<u32, 8>,
    b_source       : array<u32, 8>,
};

@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;
// Use storage (read-only) instead of uniform so the u32 arrays pack
// tightly per WGSL's std430 layout. Uniform's std140-style layout would
// pad each u32 in an array to 16 bytes, which the C++ BroadcastParams
// struct does not do.
@group(0) @binding(3) var<storage, read>       p   : BroadcastParams;

const NPOS : u32 = 0xFFFFFFFFu;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i : u32 = gid.x;

    // Delinearise i to result multi-index (row-major: rightmost is innermost).
    var result_idx : array<u32, 8>;
    var rem : u32 = i;
    for (var ki : u32 = 0u; ki < p.result_rank; ki = ki + 1u) {
        let k : u32 = p.result_rank - 1u - ki;
        result_idx[k] = rem % p.result_extents[k];
        rem = rem / p.result_extents[k];
    }

    // Project to a multi-index, then linearise using a_extents.
    var a_idx : array<u32, 8>;
    for (var x : u32 = 0u; x < p.a_rank; x = x + 1u) { a_idx[x] = 0u; }
    for (var r : u32 = 0u; r < p.result_rank; r = r + 1u) {
        let s : u32 = p.a_source[r];
        if (s != NPOS) { a_idx[s] = result_idx[r]; }
    }
    var a_flat : u32 = 0u;
    var a_stride : u32 = 1u;
    for (var ki : u32 = 0u; ki < p.a_rank; ki = ki + 1u) {
        let k : u32 = p.a_rank - 1u - ki;
        a_flat = a_flat + a_idx[k] * a_stride;
        a_stride = a_stride * p.a_extents[k];
    }

    // Same for b.
    var b_idx : array<u32, 8>;
    for (var x : u32 = 0u; x < p.b_rank; x = x + 1u) { b_idx[x] = 0u; }
    for (var r : u32 = 0u; r < p.result_rank; r = r + 1u) {
        let s : u32 = p.b_source[r];
        if (s != NPOS) { b_idx[s] = result_idx[r]; }
    }
    var b_flat : u32 = 0u;
    var b_stride : u32 = 1u;
    for (var ki : u32 = 0u; ki < p.b_rank; ki = ki + 1u) {
        let k : u32 = p.b_rank - 1u - ki;
        b_flat = b_flat + b_idx[k] * b_stride;
        b_stride = b_stride * p.b_extents[k];
    }

    out[i] = a[a_flat] {{op}} b[b_flat];
}
)WGSL";
}  // namespace detail_wgsl

inline constexpr std::string_view kBroadcastAddF32 = detail_wgsl::kBroadcastBodyF32;
inline constexpr std::string_view kBroadcastSubF32 = detail_wgsl::kBroadcastBodyF32;
inline constexpr std::string_view kBroadcastMulF32 = detail_wgsl::kBroadcastBodyF32;

// Note: kBroadcastAdd/Sub/MulF32 all point at the same template; the
// dispatcher in webgpu_detail/dispatch.hpp does the {{op}} substitution
// (`+` / `-` / `*`) at call time, mirroring how `dispatch_element_wise`
// substitutes `{{precision}}` and `{{workgroupSize}}`. This keeps the
// readable WGSL text appearing once instead of three times.

// Maximum rank supported by the broadcast kernel's Params buffer. The
// project's tests and tutorials use rank ≤ 3 in practice; this gives
// headroom without paying a meaningful uniform-buffer size cost.
inline constexpr std::size_t kBroadcastMaxRank = 8;

// ─── GEMM kernel (P3.M4) ───────────────────────────────────────────────────
//
// Tiled GEMM expressed as one readable kernel that covers both the matvec
// (rank-2 × rank-1) and matmul (rank-2 × rank-2) cases of `contract()`
// with one shared axis. Higher-rank or multi-shared-axis contractions
// delegate to reference per ADR-0012 §Decision Outcome point 6.
//
// Logical operation:
//     out[m, n] = Σ_k a[m, k] * b[k, n]
//                 (m: 0..M-1, k: 0..K-1, n: 0..N-1)
//
// The matvec case is matmul with N = 1: the same kernel runs without
// special-casing. A more efficient matvec-only kernel could use a 1-D
// workgroup, but the canonical-reference framing (ADR-0013) prefers one
// readable kernel over two specialised ones at this stage.
//
// Tiling parameters (compile-time constants in the WGSL source so they
// inline into shared-memory array bounds):
//
//   TILE_M × TILE_N — workgroup tile of the output matrix (16 × 16 = 256
//                     threads, matching kDefaultWorkgroupSize).
//   TILE_K          — number of K columns of A and K rows of B cached
//                     into shared memory per outer iteration (16).
//
// Uniform `Params` buffer carries the runtime M / N / K extents (set
// up by `detail::dispatch_gemm` in `dispatch.hpp` per ADR-0016 — the
// project no longer uses gpu.cpp's `Bindings<N>` helper).
//
// Boundary handling: threads whose (row, col) fall outside (M, N) still
// participate in cooperative shared-memory loads (writing 0.0 if their
// tile cell is out of range) but skip the final `out[]` write.

inline constexpr std::size_t kGemmTileM = 16;
inline constexpr std::size_t kGemmTileN = 16;
inline constexpr std::size_t kGemmTileK = 16;

inline constexpr std::string_view kGemmF32 = R"WGSL(
struct Params {
    M : u32,
    N : u32,
    K : u32,
};

@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;
@group(0) @binding(3) var<uniform>             p   : Params;

const TILE_M : u32 = 16u;
const TILE_N : u32 = 16u;
const TILE_K : u32 = 16u;

var<workgroup> shA : array<array<{{precision}}, TILE_K>, TILE_M>;
var<workgroup> shB : array<array<{{precision}}, TILE_N>, TILE_K>;

@compute @workgroup_size(TILE_N, TILE_M, 1)
fn main(@builtin(global_invocation_id) gid : vec3<u32>,
        @builtin(local_invocation_id)  lid : vec3<u32>) {
    let row : u32 = gid.y;
    let col : u32 = gid.x;
    let lrow : u32 = lid.y;
    let lcol : u32 = lid.x;

    var acc : {{precision}} = 0.0;

    let nTiles : u32 = (p.K + TILE_K - 1u) / TILE_K;
    for (var t : u32 = 0u; t < nTiles; t = t + 1u) {
        // Cooperatively load one TILE_M × TILE_K tile of A and one
        // TILE_K × TILE_N tile of B into shared memory. Each thread
        // loads exactly one cell of each. Out-of-range cells become 0.0
        // so the inner-product loop below can run unconditionally.
        let aRow : u32 = row;
        let aCol : u32 = t * TILE_K + lcol;
        let bRow : u32 = t * TILE_K + lrow;
        let bCol : u32 = col;

        if (aRow < p.M && aCol < p.K) {
            shA[lrow][lcol] = a[aRow * p.K + aCol];
        } else {
            shA[lrow][lcol] = 0.0;
        }
        if (bRow < p.K && bCol < p.N) {
            shB[lrow][lcol] = b[bRow * p.N + bCol];
        } else {
            shB[lrow][lcol] = 0.0;
        }

        workgroupBarrier();

        for (var k : u32 = 0u; k < TILE_K; k = k + 1u) {
            acc = acc + shA[lrow][k] * shB[k][lcol];
        }

        workgroupBarrier();
    }

    if (row < p.M && col < p.N) {
        out[row * p.N + col] = acc;
    }
}
)WGSL";

}  // namespace tensor::core::backend::webgpu::wgsl
