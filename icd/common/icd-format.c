/*
 * Vulkan
 *
 * Copyright (C) 2014 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <string.h> /* for memcpy */
#include "icd-utils.h"
#include "icd-format.h"

static const struct icd_format_info {
    size_t size;
    uint32_t channel_count;
} icd_format_table[VK_NUM_FMT] = {
    [VK_FMT_UNDEFINED]            = { 0,  0 },
    [VK_FMT_R4G4_UNORM]           = { 1,  2 },
    [VK_FMT_R4G4_USCALED]         = { 1,  2 },
    [VK_FMT_R4G4B4A4_UNORM]       = { 2,  4 },
    [VK_FMT_R4G4B4A4_USCALED]     = { 2,  4 },
    [VK_FMT_R5G6B5_UNORM]         = { 2,  3 },
    [VK_FMT_R5G6B5_USCALED]       = { 2,  3 },
    [VK_FMT_R5G5B5A1_UNORM]       = { 2,  4 },
    [VK_FMT_R5G5B5A1_USCALED]     = { 2,  4 },
    [VK_FMT_R8_UNORM]             = { 1,  1 },
    [VK_FMT_R8_SNORM]             = { 1,  1 },
    [VK_FMT_R8_USCALED]           = { 1,  1 },
    [VK_FMT_R8_SSCALED]           = { 1,  1 },
    [VK_FMT_R8_UINT]              = { 1,  1 },
    [VK_FMT_R8_SINT]              = { 1,  1 },
    [VK_FMT_R8_SRGB]              = { 1,  1 },
    [VK_FMT_R8G8_UNORM]           = { 2,  2 },
    [VK_FMT_R8G8_SNORM]           = { 2,  2 },
    [VK_FMT_R8G8_USCALED]         = { 2,  2 },
    [VK_FMT_R8G8_SSCALED]         = { 2,  2 },
    [VK_FMT_R8G8_UINT]            = { 2,  2 },
    [VK_FMT_R8G8_SINT]            = { 2,  2 },
    [VK_FMT_R8G8_SRGB]            = { 2,  2 },
    [VK_FMT_R8G8B8_UNORM]         = { 3,  3 },
    [VK_FMT_R8G8B8_SNORM]         = { 3,  3 },
    [VK_FMT_R8G8B8_USCALED]       = { 3,  3 },
    [VK_FMT_R8G8B8_SSCALED]       = { 3,  3 },
    [VK_FMT_R8G8B8_UINT]          = { 3,  3 },
    [VK_FMT_R8G8B8_SINT]          = { 3,  3 },
    [VK_FMT_R8G8B8_SRGB]          = { 3,  3 },
    [VK_FMT_R8G8B8A8_UNORM]       = { 4,  4 },
    [VK_FMT_R8G8B8A8_SNORM]       = { 4,  4 },
    [VK_FMT_R8G8B8A8_USCALED]     = { 4,  4 },
    [VK_FMT_R8G8B8A8_SSCALED]     = { 4,  4 },
    [VK_FMT_R8G8B8A8_UINT]        = { 4,  4 },
    [VK_FMT_R8G8B8A8_SINT]        = { 4,  4 },
    [VK_FMT_R8G8B8A8_SRGB]        = { 4,  4 },
    [VK_FMT_R10G10B10A2_UNORM]    = { 4,  4 },
    [VK_FMT_R10G10B10A2_SNORM]    = { 4,  4 },
    [VK_FMT_R10G10B10A2_USCALED]  = { 4,  4 },
    [VK_FMT_R10G10B10A2_SSCALED]  = { 4,  4 },
    [VK_FMT_R10G10B10A2_UINT]     = { 4,  4 },
    [VK_FMT_R10G10B10A2_SINT]     = { 4,  4 },
    [VK_FMT_R16_UNORM]            = { 2,  1 },
    [VK_FMT_R16_SNORM]            = { 2,  1 },
    [VK_FMT_R16_USCALED]          = { 2,  1 },
    [VK_FMT_R16_SSCALED]          = { 2,  1 },
    [VK_FMT_R16_UINT]             = { 2,  1 },
    [VK_FMT_R16_SINT]             = { 2,  1 },
    [VK_FMT_R16_SFLOAT]           = { 2,  1 },
    [VK_FMT_R16G16_UNORM]         = { 4,  2 },
    [VK_FMT_R16G16_SNORM]         = { 4,  2 },
    [VK_FMT_R16G16_USCALED]       = { 4,  2 },
    [VK_FMT_R16G16_SSCALED]       = { 4,  2 },
    [VK_FMT_R16G16_UINT]          = { 4,  2 },
    [VK_FMT_R16G16_SINT]          = { 4,  2 },
    [VK_FMT_R16G16_SFLOAT]        = { 4,  2 },
    [VK_FMT_R16G16B16_UNORM]      = { 6,  3 },
    [VK_FMT_R16G16B16_SNORM]      = { 6,  3 },
    [VK_FMT_R16G16B16_USCALED]    = { 6,  3 },
    [VK_FMT_R16G16B16_SSCALED]    = { 6,  3 },
    [VK_FMT_R16G16B16_UINT]       = { 6,  3 },
    [VK_FMT_R16G16B16_SINT]       = { 6,  3 },
    [VK_FMT_R16G16B16_SFLOAT]     = { 6,  3 },
    [VK_FMT_R16G16B16A16_UNORM]   = { 8,  4 },
    [VK_FMT_R16G16B16A16_SNORM]   = { 8,  4 },
    [VK_FMT_R16G16B16A16_USCALED] = { 8,  4 },
    [VK_FMT_R16G16B16A16_SSCALED] = { 8,  4 },
    [VK_FMT_R16G16B16A16_UINT]    = { 8,  4 },
    [VK_FMT_R16G16B16A16_SINT]    = { 8,  4 },
    [VK_FMT_R16G16B16A16_SFLOAT]  = { 8,  4 },
    [VK_FMT_R32_UINT]             = { 4,  1 },
    [VK_FMT_R32_SINT]             = { 4,  1 },
    [VK_FMT_R32_SFLOAT]           = { 4,  1 },
    [VK_FMT_R32G32_UINT]          = { 8,  2 },
    [VK_FMT_R32G32_SINT]          = { 8,  2 },
    [VK_FMT_R32G32_SFLOAT]        = { 8,  2 },
    [VK_FMT_R32G32B32_UINT]       = { 12, 3 },
    [VK_FMT_R32G32B32_SINT]       = { 12, 3 },
    [VK_FMT_R32G32B32_SFLOAT]     = { 12, 3 },
    [VK_FMT_R32G32B32A32_UINT]    = { 16, 4 },
    [VK_FMT_R32G32B32A32_SINT]    = { 16, 4 },
    [VK_FMT_R32G32B32A32_SFLOAT]  = { 16, 4 },
    [VK_FMT_R64_SFLOAT]           = { 8,  1 },
    [VK_FMT_R64G64_SFLOAT]        = { 16, 2 },
    [VK_FMT_R64G64B64_SFLOAT]     = { 24, 3 },
    [VK_FMT_R64G64B64A64_SFLOAT]  = { 32, 4 },
    [VK_FMT_R11G11B10_UFLOAT]     = { 4,  3 },
    [VK_FMT_R9G9B9E5_UFLOAT]      = { 4,  3 },
    [VK_FMT_D16_UNORM]            = { 2,  1 },
    [VK_FMT_D24_UNORM]            = { 3,  1 },
    [VK_FMT_D32_SFLOAT]           = { 4,  1 },
    [VK_FMT_S8_UINT]              = { 1,  1 },
    [VK_FMT_D16_UNORM_S8_UINT]    = { 3,  2 },
    [VK_FMT_D24_UNORM_S8_UINT]    = { 4,  2 },
    [VK_FMT_D32_SFLOAT_S8_UINT]   = { 4,  2 },
    [VK_FMT_BC1_RGB_UNORM]        = { 8,  4 },
    [VK_FMT_BC1_RGB_SRGB]         = { 8,  4 },
    [VK_FMT_BC1_RGBA_UNORM]       = { 8,  4 },
    [VK_FMT_BC1_RGBA_SRGB]        = { 8,  4 },
    [VK_FMT_BC2_UNORM]            = { 16, 4 },
    [VK_FMT_BC2_SRGB]             = { 16, 4 },
    [VK_FMT_BC3_UNORM]            = { 16, 4 },
    [VK_FMT_BC3_SRGB]             = { 16, 4 },
    [VK_FMT_BC4_UNORM]            = { 8,  4 },
    [VK_FMT_BC4_SNORM]            = { 8,  4 },
    [VK_FMT_BC5_UNORM]            = { 16, 4 },
    [VK_FMT_BC5_SNORM]            = { 16, 4 },
    [VK_FMT_BC6H_UFLOAT]          = { 16, 4 },
    [VK_FMT_BC6H_SFLOAT]          = { 16, 4 },
    [VK_FMT_BC7_UNORM]            = { 16, 4 },
    [VK_FMT_BC7_SRGB]             = { 16, 4 },
    /* TODO: Initialize remaining compressed formats. */
    [VK_FMT_ETC2_R8G8B8_UNORM]    = { 0, 0 },
    [VK_FMT_ETC2_R8G8B8A1_UNORM]  = { 0, 0 },
    [VK_FMT_ETC2_R8G8B8A8_UNORM]  = { 0, 0 },
    [VK_FMT_EAC_R11_UNORM]        = { 0, 0 },
    [VK_FMT_EAC_R11_SNORM]        = { 0, 0 },
    [VK_FMT_EAC_R11G11_UNORM]     = { 0, 0 },
    [VK_FMT_EAC_R11G11_SNORM]     = { 0, 0 },
    [VK_FMT_ASTC_4x4_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_4x4_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_5x4_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_5x4_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_5x5_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_5x5_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_6x5_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_6x5_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_6x6_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_6x6_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_8x5_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_8x5_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_8x6_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_8x6_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_8x8_UNORM]       = { 0, 0 },
    [VK_FMT_ASTC_8x8_SRGB]        = { 0, 0 },
    [VK_FMT_ASTC_10x5_UNORM]      = { 0, 0 },
    [VK_FMT_ASTC_10x5_SRGB]       = { 0, 0 },
    [VK_FMT_ASTC_10x6_UNORM]      = { 0, 0 },
    [VK_FMT_ASTC_10x6_SRGB]       = { 0, 0 },
    [VK_FMT_ASTC_10x8_UNORM]      = { 0, 0 },
    [VK_FMT_ASTC_10x8_SRGB]       = { 0, 0 },
    [VK_FMT_ASTC_10x10_UNORM]     = { 0, 0 },
    [VK_FMT_ASTC_10x10_SRGB]      = { 0, 0 },
    [VK_FMT_ASTC_12x10_UNORM]     = { 0, 0 },
    [VK_FMT_ASTC_12x10_SRGB]      = { 0, 0 },
    [VK_FMT_ASTC_12x12_UNORM]     = { 0, 0 },
    [VK_FMT_ASTC_12x12_SRGB]      = { 0, 0 },
    [VK_FMT_B5G6R5_UNORM]         = { 2, 3 },
    [VK_FMT_B5G6R5_USCALED]       = { 2, 3 },
    [VK_FMT_B8G8R8_UNORM]         = { 3, 3 },
    [VK_FMT_B8G8R8_SNORM]         = { 3, 3 },
    [VK_FMT_B8G8R8_USCALED]       = { 3, 3 },
    [VK_FMT_B8G8R8_SSCALED]       = { 3, 3 },
    [VK_FMT_B8G8R8_UINT]          = { 3, 3 },
    [VK_FMT_B8G8R8_SINT]          = { 3, 3 },
    [VK_FMT_B8G8R8_SRGB]          = { 3, 3 },
    [VK_FMT_B8G8R8A8_UNORM]       = { 4, 4 },
    [VK_FMT_B8G8R8A8_SNORM]       = { 4, 4 },
    [VK_FMT_B8G8R8A8_USCALED]     = { 4, 4 },
    [VK_FMT_B8G8R8A8_SSCALED]     = { 4, 4 },
    [VK_FMT_B8G8R8A8_UINT]        = { 4, 4 },
    [VK_FMT_B8G8R8A8_SINT]        = { 4, 4 },
    [VK_FMT_B8G8R8A8_SRGB]        = { 4, 4 },
    [VK_FMT_B10G10R10A2_UNORM]    = { 4, 4 },
    [VK_FMT_B10G10R10A2_SNORM]    = { 4, 4 },
    [VK_FMT_B10G10R10A2_USCALED]  = { 4, 4 },
    [VK_FMT_B10G10R10A2_SSCALED]  = { 4, 4 },
    [VK_FMT_B10G10R10A2_UINT]     = { 4, 4 },
    [VK_FMT_B10G10R10A2_SINT]     = { 4, 4 },
};

bool icd_format_is_ds(VK_FORMAT format)
{
    bool is_ds = false;

    switch (format) {
    case VK_FMT_D16_UNORM:
    case VK_FMT_D24_UNORM:
    case VK_FMT_D32_SFLOAT:
    case VK_FMT_S8_UINT:
    case VK_FMT_D16_UNORM_S8_UINT:
    case VK_FMT_D24_UNORM_S8_UINT:
    case VK_FMT_D32_SFLOAT_S8_UINT:
        is_ds = true;
        break;
    default:
        break;
    }

    return is_ds;
}

bool icd_format_is_norm(VK_FORMAT format)
{
    bool is_norm = false;

    switch (format) {
    case VK_FMT_R4G4_UNORM:
    case VK_FMT_R4G4B4A4_UNORM:
    case VK_FMT_R5G6B5_UNORM:
    case VK_FMT_R5G5B5A1_UNORM:
    case VK_FMT_R8_UNORM:
    case VK_FMT_R8_SNORM:
    case VK_FMT_R8G8_UNORM:
    case VK_FMT_R8G8_SNORM:
    case VK_FMT_R8G8B8_UNORM:
    case VK_FMT_R8G8B8_SNORM:
    case VK_FMT_R8G8B8A8_UNORM:
    case VK_FMT_R8G8B8A8_SNORM:
    case VK_FMT_R10G10B10A2_UNORM:
    case VK_FMT_R10G10B10A2_SNORM:
    case VK_FMT_R16_UNORM:
    case VK_FMT_R16_SNORM:
    case VK_FMT_R16G16_UNORM:
    case VK_FMT_R16G16_SNORM:
    case VK_FMT_R16G16B16_UNORM:
    case VK_FMT_R16G16B16_SNORM:
    case VK_FMT_R16G16B16A16_UNORM:
    case VK_FMT_R16G16B16A16_SNORM:
    case VK_FMT_BC1_RGB_UNORM:
    case VK_FMT_BC2_UNORM:
    case VK_FMT_BC3_UNORM:
    case VK_FMT_BC4_UNORM:
    case VK_FMT_BC4_SNORM:
    case VK_FMT_BC5_UNORM:
    case VK_FMT_BC5_SNORM:
    case VK_FMT_BC7_UNORM:
    case VK_FMT_ETC2_R8G8B8_UNORM:
    case VK_FMT_ETC2_R8G8B8A1_UNORM:
    case VK_FMT_ETC2_R8G8B8A8_UNORM:
    case VK_FMT_EAC_R11_UNORM:
    case VK_FMT_EAC_R11_SNORM:
    case VK_FMT_EAC_R11G11_UNORM:
    case VK_FMT_EAC_R11G11_SNORM:
    case VK_FMT_ASTC_4x4_UNORM:
    case VK_FMT_ASTC_5x4_UNORM:
    case VK_FMT_ASTC_5x5_UNORM:
    case VK_FMT_ASTC_6x5_UNORM:
    case VK_FMT_ASTC_6x6_UNORM:
    case VK_FMT_ASTC_8x5_UNORM:
    case VK_FMT_ASTC_8x6_UNORM:
    case VK_FMT_ASTC_8x8_UNORM:
    case VK_FMT_ASTC_10x5_UNORM:
    case VK_FMT_ASTC_10x6_UNORM:
    case VK_FMT_ASTC_10x8_UNORM:
    case VK_FMT_ASTC_10x10_UNORM:
    case VK_FMT_ASTC_12x10_UNORM:
    case VK_FMT_ASTC_12x12_UNORM:
    case VK_FMT_B5G6R5_UNORM:
    case VK_FMT_B8G8R8_UNORM:
    case VK_FMT_B8G8R8_SNORM:
    case VK_FMT_B8G8R8A8_UNORM:
    case VK_FMT_B8G8R8A8_SNORM:
    case VK_FMT_B10G10R10A2_UNORM:
    case VK_FMT_B10G10R10A2_SNORM:
        is_norm = true;
        break;
    default:
        break;
    }

    return is_norm;
};

bool icd_format_is_int(VK_FORMAT format)
{
    bool is_int = false;

    switch (format) {
    case VK_FMT_R8_UINT:
    case VK_FMT_R8_SINT:
    case VK_FMT_R8G8_UINT:
    case VK_FMT_R8G8_SINT:
    case VK_FMT_R8G8B8_UINT:
    case VK_FMT_R8G8B8_SINT:
    case VK_FMT_R8G8B8A8_UINT:
    case VK_FMT_R8G8B8A8_SINT:
    case VK_FMT_R10G10B10A2_UINT:
    case VK_FMT_R10G10B10A2_SINT:
    case VK_FMT_R16_UINT:
    case VK_FMT_R16_SINT:
    case VK_FMT_R16G16_UINT:
    case VK_FMT_R16G16_SINT:
    case VK_FMT_R16G16B16_UINT:
    case VK_FMT_R16G16B16_SINT:
    case VK_FMT_R16G16B16A16_UINT:
    case VK_FMT_R16G16B16A16_SINT:
    case VK_FMT_R32_UINT:
    case VK_FMT_R32_SINT:
    case VK_FMT_R32G32_UINT:
    case VK_FMT_R32G32_SINT:
    case VK_FMT_R32G32B32_UINT:
    case VK_FMT_R32G32B32_SINT:
    case VK_FMT_R32G32B32A32_UINT:
    case VK_FMT_R32G32B32A32_SINT:
    case VK_FMT_B8G8R8_UINT:
    case VK_FMT_B8G8R8_SINT:
    case VK_FMT_B8G8R8A8_UINT:
    case VK_FMT_B8G8R8A8_SINT:
    case VK_FMT_B10G10R10A2_UINT:
    case VK_FMT_B10G10R10A2_SINT:
        is_int = true;
        break;
    default:
        break;
    }

    return is_int;
}

bool icd_format_is_float(VK_FORMAT format)
{
    bool is_float = false;

    switch (format) {
    case VK_FMT_R16_SFLOAT:
    case VK_FMT_R16G16_SFLOAT:
    case VK_FMT_R16G16B16_SFLOAT:
    case VK_FMT_R16G16B16A16_SFLOAT:
    case VK_FMT_R32_SFLOAT:
    case VK_FMT_R32G32_SFLOAT:
    case VK_FMT_R32G32B32_SFLOAT:
    case VK_FMT_R32G32B32A32_SFLOAT:
    case VK_FMT_R64_SFLOAT:
    case VK_FMT_R64G64_SFLOAT:
    case VK_FMT_R64G64B64_SFLOAT:
    case VK_FMT_R64G64B64A64_SFLOAT:
    case VK_FMT_R11G11B10_UFLOAT:
    case VK_FMT_R9G9B9E5_UFLOAT:
    case VK_FMT_BC6H_UFLOAT:
    case VK_FMT_BC6H_SFLOAT:
        is_float = true;
        break;
    default:
        break;
    }

    return is_float;
}

bool icd_format_is_srgb(VK_FORMAT format)
{
    bool is_srgb = false;

    switch (format) {
    case VK_FMT_R8_SRGB:
    case VK_FMT_R8G8_SRGB:
    case VK_FMT_R8G8B8_SRGB:
    case VK_FMT_R8G8B8A8_SRGB:
    case VK_FMT_BC1_RGB_SRGB:
    case VK_FMT_BC2_SRGB:
    case VK_FMT_BC3_SRGB:
    case VK_FMT_BC7_SRGB:
    case VK_FMT_ASTC_4x4_SRGB:
    case VK_FMT_ASTC_5x4_SRGB:
    case VK_FMT_ASTC_5x5_SRGB:
    case VK_FMT_ASTC_6x5_SRGB:
    case VK_FMT_ASTC_6x6_SRGB:
    case VK_FMT_ASTC_8x5_SRGB:
    case VK_FMT_ASTC_8x6_SRGB:
    case VK_FMT_ASTC_8x8_SRGB:
    case VK_FMT_ASTC_10x5_SRGB:
    case VK_FMT_ASTC_10x6_SRGB:
    case VK_FMT_ASTC_10x8_SRGB:
    case VK_FMT_ASTC_10x10_SRGB:
    case VK_FMT_ASTC_12x10_SRGB:
    case VK_FMT_ASTC_12x12_SRGB:
    case VK_FMT_B8G8R8_SRGB:
    case VK_FMT_B8G8R8A8_SRGB:
        is_srgb = true;
        break;
    default:
        break;
    }

    return is_srgb;
}

bool icd_format_is_compressed(VK_FORMAT format)
{
    switch (format) {
    case VK_FMT_BC1_RGB_UNORM:
    case VK_FMT_BC1_RGB_SRGB:
    case VK_FMT_BC2_UNORM:
    case VK_FMT_BC2_SRGB:
    case VK_FMT_BC3_UNORM:
    case VK_FMT_BC3_SRGB:
    case VK_FMT_BC4_UNORM:
    case VK_FMT_BC4_SNORM:
    case VK_FMT_BC5_UNORM:
    case VK_FMT_BC5_SNORM:
    case VK_FMT_BC6H_UFLOAT:
    case VK_FMT_BC6H_SFLOAT:
    case VK_FMT_BC7_UNORM:
    case VK_FMT_BC7_SRGB:
    case VK_FMT_ETC2_R8G8B8_UNORM:
    case VK_FMT_ETC2_R8G8B8A1_UNORM:
    case VK_FMT_ETC2_R8G8B8A8_UNORM:
    case VK_FMT_EAC_R11_UNORM:
    case VK_FMT_EAC_R11_SNORM:
    case VK_FMT_EAC_R11G11_UNORM:
    case VK_FMT_EAC_R11G11_SNORM:
    case VK_FMT_ASTC_4x4_UNORM:
    case VK_FMT_ASTC_4x4_SRGB:
    case VK_FMT_ASTC_5x4_UNORM:
    case VK_FMT_ASTC_5x4_SRGB:
    case VK_FMT_ASTC_5x5_UNORM:
    case VK_FMT_ASTC_5x5_SRGB:
    case VK_FMT_ASTC_6x5_UNORM:
    case VK_FMT_ASTC_6x5_SRGB:
    case VK_FMT_ASTC_6x6_UNORM:
    case VK_FMT_ASTC_6x6_SRGB:
    case VK_FMT_ASTC_8x5_UNORM:
    case VK_FMT_ASTC_8x5_SRGB:
    case VK_FMT_ASTC_8x6_UNORM:
    case VK_FMT_ASTC_8x6_SRGB:
    case VK_FMT_ASTC_8x8_UNORM:
    case VK_FMT_ASTC_8x8_SRGB:
    case VK_FMT_ASTC_10x5_UNORM:
    case VK_FMT_ASTC_10x5_SRGB:
    case VK_FMT_ASTC_10x6_UNORM:
    case VK_FMT_ASTC_10x6_SRGB:
    case VK_FMT_ASTC_10x8_UNORM:
    case VK_FMT_ASTC_10x8_SRGB:
    case VK_FMT_ASTC_10x10_UNORM:
    case VK_FMT_ASTC_10x10_SRGB:
    case VK_FMT_ASTC_12x10_UNORM:
    case VK_FMT_ASTC_12x10_SRGB:
    case VK_FMT_ASTC_12x12_UNORM:
    case VK_FMT_ASTC_12x12_SRGB:
        return true;
    default:
        return false;
    }
}

size_t icd_format_get_size(VK_FORMAT format)
{
    return icd_format_table[format].size;
}

VK_IMAGE_FORMAT_CLASS icd_format_get_class(VK_FORMAT format)
{
    if (icd_format_is_undef(format))
        assert(!"undefined format");
    if (icd_format_is_compressed(format)) {
        switch (icd_format_get_size(format)) {
        case 8:
            return VK_IMAGE_FORMAT_CLASS_64_BIT_BLOCK;
        case 16:
            return VK_IMAGE_FORMAT_CLASS_128_BIT_BLOCK;
        default:
            assert(!"illegal compressed format");
        }
    } else if (icd_format_is_ds(format)) {
        switch (icd_format_get_size(format)) {
        case 1:
            return VK_IMAGE_FORMAT_CLASS_S8;
        case 2:
            return VK_IMAGE_FORMAT_CLASS_D16;
        case 3:
            switch (icd_format_get_channel_count(format)) {
            case 1:
                return VK_IMAGE_FORMAT_CLASS_D24;
            case 2:
                return VK_IMAGE_FORMAT_CLASS_D16S8;
            default:
                assert(!"illegal depth stencil format channels");
            }
        case 4:
            switch (icd_format_get_channel_count(format)) {
            case 1:
                return VK_IMAGE_FORMAT_CLASS_D32;
            case 2:
                return VK_IMAGE_FORMAT_CLASS_D24S8;
            default:
                assert(!"illegal depth stencil format channels");
            }
        case 5:
            return VK_IMAGE_FORMAT_CLASS_D32S8;
        default:
            assert(!"illegal depth stencil format");
        }
    } else { /* uncompressed color format */
        switch (icd_format_get_size(format)) {
        case 1:
            return VK_IMAGE_FORMAT_CLASS_8_BITS;
        case 2:
            return VK_IMAGE_FORMAT_CLASS_16_BITS;
        case 3:
            return VK_IMAGE_FORMAT_CLASS_24_BITS;
        case 4:
            return VK_IMAGE_FORMAT_CLASS_32_BITS;
        case 6:
            return VK_IMAGE_FORMAT_CLASS_48_BITS;
        case 8:
            return VK_IMAGE_FORMAT_CLASS_64_BITS;
        case 12:
            return VK_IMAGE_FORMAT_CLASS_96_BITS;
        case 16:
            return VK_IMAGE_FORMAT_CLASS_128_BITS;
        default:
            assert(!"illegal uncompressed color format");
        }
    }
}

unsigned int icd_format_get_channel_count(VK_FORMAT format)
{
    return icd_format_table[format].channel_count;
}

/**
 * Convert a raw RGBA color to a raw value.  \p value must have at least
 * icd_format_get_size(format) bytes.
 */
void icd_format_get_raw_value(VK_FORMAT format,
                              const uint32_t color[4],
                              void *value)
{
    /* assume little-endian */
    switch (format) {
    case VK_FMT_UNDEFINED:
        break;
    case VK_FMT_R4G4_UNORM:
    case VK_FMT_R4G4_USCALED:
        ((uint8_t *) value)[0]  = (color[0] & 0xf) << 0   |
                                  (color[1] & 0xf) << 4;
        break;
    case VK_FMT_R4G4B4A4_UNORM:
    case VK_FMT_R4G4B4A4_USCALED:
        ((uint16_t *) value)[0] = (color[0] & 0xf) << 0   |
                                  (color[1] & 0xf) << 4   |
                                  (color[2] & 0xf) << 8   |
                                  (color[3] & 0xf) << 12;
        break;
    case VK_FMT_R5G6B5_UNORM:
    case VK_FMT_R5G6B5_USCALED:
        ((uint16_t *) value)[0] = (color[0] & 0x1f) << 0  |
                                  (color[1] & 0x3f) << 5  |
                                  (color[2] & 0x1f) << 11;
        break;
    case VK_FMT_B5G6R5_UNORM:
        ((uint16_t *) value)[0] = (color[2] & 0x1f) << 0  |
                                  (color[1] & 0x3f) << 5  |
                                  (color[0] & 0x1f) << 11;
        break;
    case VK_FMT_R5G5B5A1_UNORM:
    case VK_FMT_R5G5B5A1_USCALED:
        ((uint16_t *) value)[0] = (color[0] & 0x1f) << 0  |
                                  (color[1] & 0x1f) << 5  |
                                  (color[2] & 0x1f) << 10 |
                                  (color[3] & 0x1)  << 15;
        break;
    case VK_FMT_R8_UNORM:
    case VK_FMT_R8_SNORM:
    case VK_FMT_R8_USCALED:
    case VK_FMT_R8_SSCALED:
    case VK_FMT_R8_UINT:
    case VK_FMT_R8_SINT:
    case VK_FMT_R8_SRGB:
        ((uint8_t *) value)[0]  = (uint8_t) color[0];
        break;
    case VK_FMT_R8G8_UNORM:
    case VK_FMT_R8G8_SNORM:
    case VK_FMT_R8G8_USCALED:
    case VK_FMT_R8G8_SSCALED:
    case VK_FMT_R8G8_UINT:
    case VK_FMT_R8G8_SINT:
    case VK_FMT_R8G8_SRGB:
        ((uint8_t *) value)[0]  = (uint8_t) color[0];
        ((uint8_t *) value)[1]  = (uint8_t) color[1];
        break;
    case VK_FMT_R8G8B8A8_UNORM:
    case VK_FMT_R8G8B8A8_SNORM:
    case VK_FMT_R8G8B8A8_USCALED:
    case VK_FMT_R8G8B8A8_SSCALED:
    case VK_FMT_R8G8B8A8_UINT:
    case VK_FMT_R8G8B8A8_SINT:
    case VK_FMT_R8G8B8A8_SRGB:
        ((uint8_t *) value)[0]  = (uint8_t) color[0];
        ((uint8_t *) value)[1]  = (uint8_t) color[1];
        ((uint8_t *) value)[2]  = (uint8_t) color[2];
        ((uint8_t *) value)[3]  = (uint8_t) color[3];
        break;
    case VK_FMT_B8G8R8A8_UNORM:
    case VK_FMT_B8G8R8A8_SRGB:
        ((uint8_t *) value)[0]  = (uint8_t) color[2];
        ((uint8_t *) value)[1]  = (uint8_t) color[1];
        ((uint8_t *) value)[2]  = (uint8_t) color[0];
        ((uint8_t *) value)[3]  = (uint8_t) color[3];
        break;
    case VK_FMT_R11G11B10_UFLOAT:
        ((uint32_t *) value)[0] = (color[0] & 0x7ff) << 0  |
                                  (color[1] & 0x7ff) << 11 |
                                  (color[2] & 0x3ff) << 22;
        break;
    case VK_FMT_R10G10B10A2_UNORM:
    case VK_FMT_R10G10B10A2_SNORM:
    case VK_FMT_R10G10B10A2_USCALED:
    case VK_FMT_R10G10B10A2_SSCALED:
    case VK_FMT_R10G10B10A2_UINT:
    case VK_FMT_R10G10B10A2_SINT:
        ((uint32_t *) value)[0] = (color[0] & 0x3ff) << 0  |
                                  (color[1] & 0x3ff) << 10 |
                                  (color[2] & 0x3ff) << 20 |
                                  (color[3] & 0x3)   << 30;
        break;
    case VK_FMT_R16_UNORM:
    case VK_FMT_R16_SNORM:
    case VK_FMT_R16_USCALED:
    case VK_FMT_R16_SSCALED:
    case VK_FMT_R16_UINT:
    case VK_FMT_R16_SINT:
    case VK_FMT_R16_SFLOAT:
        ((uint16_t *) value)[0] = (uint16_t) color[0];
        break;
    case VK_FMT_R16G16_UNORM:
    case VK_FMT_R16G16_SNORM:
    case VK_FMT_R16G16_USCALED:
    case VK_FMT_R16G16_SSCALED:
    case VK_FMT_R16G16_UINT:
    case VK_FMT_R16G16_SINT:
    case VK_FMT_R16G16_SFLOAT:
        ((uint16_t *) value)[0] = (uint16_t) color[0];
        ((uint16_t *) value)[1] = (uint16_t) color[1];
        break;
    case VK_FMT_R16G16B16A16_UNORM:
    case VK_FMT_R16G16B16A16_SNORM:
    case VK_FMT_R16G16B16A16_USCALED:
    case VK_FMT_R16G16B16A16_SSCALED:
    case VK_FMT_R16G16B16A16_UINT:
    case VK_FMT_R16G16B16A16_SINT:
    case VK_FMT_R16G16B16A16_SFLOAT:
        ((uint16_t *) value)[0] = (uint16_t) color[0];
        ((uint16_t *) value)[1] = (uint16_t) color[1];
        ((uint16_t *) value)[2] = (uint16_t) color[2];
        ((uint16_t *) value)[3] = (uint16_t) color[3];
        break;
    case VK_FMT_R32_UINT:
    case VK_FMT_R32_SINT:
    case VK_FMT_R32_SFLOAT:
        ((uint32_t *) value)[0] = color[0];
        break;
    case VK_FMT_R32G32_UINT:
    case VK_FMT_R32G32_SINT:
    case VK_FMT_R32G32_SFLOAT:
        ((uint32_t *) value)[0] = color[0];
        ((uint32_t *) value)[1] = color[1];
        break;
    case VK_FMT_R32G32B32_UINT:
    case VK_FMT_R32G32B32_SINT:
    case VK_FMT_R32G32B32_SFLOAT:
        ((uint32_t *) value)[0] = color[0];
        ((uint32_t *) value)[1] = color[1];
        ((uint32_t *) value)[2] = color[2];
        break;
    case VK_FMT_R32G32B32A32_UINT:
    case VK_FMT_R32G32B32A32_SINT:
    case VK_FMT_R32G32B32A32_SFLOAT:
        ((uint32_t *) value)[0] = color[0];
        ((uint32_t *) value)[1] = color[1];
        ((uint32_t *) value)[2] = color[2];
        ((uint32_t *) value)[3] = color[3];
        break;
    case VK_FMT_D16_UNORM_S8_UINT:
        ((uint16_t *) value)[0] = (uint16_t) color[0];
        ((char *) value)[2] = (uint8_t) color[1];
        break;
    case VK_FMT_D32_SFLOAT_S8_UINT:
        ((uint32_t *) value)[0] = (uint32_t) color[0];
        ((char *) value)[4] = (uint8_t) color[1];
        break;
    case VK_FMT_R9G9B9E5_UFLOAT:
        ((uint32_t *) value)[0] = (color[0] & 0x1ff) << 0  |
                                  (color[1] & 0x1ff) << 9  |
                                  (color[2] & 0x1ff) << 18 |
                                  (color[3] & 0x1f)  << 27;
        break;
    case VK_FMT_BC1_RGB_UNORM:
    case VK_FMT_BC1_RGB_SRGB:
    case VK_FMT_BC4_UNORM:
    case VK_FMT_BC4_SNORM:
        memcpy(value, color, 8);
        break;
    case VK_FMT_BC2_UNORM:
    case VK_FMT_BC2_SRGB:
    case VK_FMT_BC3_UNORM:
    case VK_FMT_BC3_SRGB:
    case VK_FMT_BC5_UNORM:
    case VK_FMT_BC5_SNORM:
    case VK_FMT_BC6H_UFLOAT:
    case VK_FMT_BC6H_SFLOAT:
    case VK_FMT_BC7_UNORM:
    case VK_FMT_BC7_SRGB:
        memcpy(value, color, 16);
        break;
    case VK_FMT_R8G8B8_UNORM:
    case VK_FMT_R8G8B8_SNORM:
    case VK_FMT_R8G8B8_USCALED:
    case VK_FMT_R8G8B8_SSCALED:
    case VK_FMT_R8G8B8_UINT:
    case VK_FMT_R8G8B8_SINT:
    case VK_FMT_R8G8B8_SRGB:
        ((uint8_t *) value)[0]  = (uint8_t) color[0];
        ((uint8_t *) value)[1]  = (uint8_t) color[1];
        ((uint8_t *) value)[2]  = (uint8_t) color[2];
        break;
    case VK_FMT_R16G16B16_UNORM:
    case VK_FMT_R16G16B16_SNORM:
    case VK_FMT_R16G16B16_USCALED:
    case VK_FMT_R16G16B16_SSCALED:
    case VK_FMT_R16G16B16_UINT:
    case VK_FMT_R16G16B16_SINT:
    case VK_FMT_R16G16B16_SFLOAT:
        ((uint16_t *) value)[0] = (uint16_t) color[0];
        ((uint16_t *) value)[1] = (uint16_t) color[1];
        ((uint16_t *) value)[2] = (uint16_t) color[2];
        break;
    case VK_FMT_B10G10R10A2_UNORM:
    case VK_FMT_B10G10R10A2_SNORM:
    case VK_FMT_B10G10R10A2_USCALED:
    case VK_FMT_B10G10R10A2_SSCALED:
    case VK_FMT_B10G10R10A2_UINT:
    case VK_FMT_B10G10R10A2_SINT:
        ((uint32_t *) value)[0] = (color[2] & 0x3ff) << 0  |
                                  (color[1] & 0x3ff) << 10 |
                                  (color[0] & 0x3ff) << 20 |
                                  (color[3] & 0x3)   << 30;
        break;
    case VK_FMT_R64_SFLOAT:
        /* higher 32 bits always 0 */
        ((uint64_t *) value)[0] = color[0];
        break;
    case VK_FMT_R64G64_SFLOAT:
        ((uint64_t *) value)[0] = color[0];
        ((uint64_t *) value)[1] = color[1];
        break;
    case VK_FMT_R64G64B64_SFLOAT:
        ((uint64_t *) value)[0] = color[0];
        ((uint64_t *) value)[1] = color[1];
        ((uint64_t *) value)[2] = color[2];
        break;
    case VK_FMT_R64G64B64A64_SFLOAT:
        ((uint64_t *) value)[0] = color[0];
        ((uint64_t *) value)[1] = color[1];
        ((uint64_t *) value)[2] = color[2];
        ((uint64_t *) value)[3] = color[3];
        break;
    default:
        assert(!"unknown format");
        break;
    }
}
