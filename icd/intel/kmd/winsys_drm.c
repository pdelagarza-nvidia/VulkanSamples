/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2012-2014 LunarG, Inc.
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

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#ifndef ETIME
#define ETIME ETIMEDOUT
#endif
#include <assert.h>

#include <xf86drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "icd-instance.h"
#include "icd-utils.h"
#include "winsys.h"

struct intel_winsys {
   const struct icd_instance *instance;
   int fd;
   drm_intel_bufmgr *bufmgr;
   struct intel_winsys_info info;

   drm_intel_context *ctx;
};

static drm_intel_bo *
gem_bo(const struct intel_bo *bo)
{
   return (drm_intel_bo *) bo;
}

static bool
get_param(struct intel_winsys *winsys, int param, int *value)
{
   struct drm_i915_getparam gp;
   int err;

   *value = 0;

   memset(&gp, 0, sizeof(gp));
   gp.param = param;
   gp.value = value;

   err = drmCommandWriteRead(winsys->fd, DRM_I915_GETPARAM, &gp, sizeof(gp));
   if (err) {
      *value = 0;
      return false;
   }

   return true;
}

static bool
test_address_swizzling(struct intel_winsys *winsys)
{
   drm_intel_bo *bo;
   uint32_t tiling = I915_TILING_X, swizzle;
   unsigned long pitch;

   bo = drm_intel_bo_alloc_tiled(winsys->bufmgr,
         "address swizzling test", 64, 64, 4, &tiling, &pitch, 0);
   if (bo) {
      drm_intel_bo_get_tiling(bo, &tiling, &swizzle);
      drm_intel_bo_unreference(bo);
   }
   else {
      swizzle = I915_BIT_6_SWIZZLE_NONE;
   }

   return (swizzle != I915_BIT_6_SWIZZLE_NONE);
}

static bool
test_reg_read(struct intel_winsys *winsys, uint32_t reg)
{
   uint64_t dummy;

   return !drm_intel_reg_read(winsys->bufmgr, reg, &dummy);
}

static bool
probe_winsys(struct intel_winsys *winsys)
{
   struct intel_winsys_info *info = &winsys->info;
   int val;

   /*
    * When we need the Nth vertex from a user vertex buffer, and the vertex is
    * uploaded to, say, the beginning of a bo, we want the first vertex in the
    * bo to be fetched.  One way to do this is to set the base address of the
    * vertex buffer to
    *
    *   bo->offset64 + (vb->buffer_offset - vb->stride * N).
    *
    * The second term may be negative, and we need kernel support to do that.
    *
    * This check is taken from the classic driver.  u_vbuf_upload_buffers()
    * guarantees the term is never negative, but it is good to require a
    * recent kernel.
    */
   get_param(winsys, I915_PARAM_HAS_RELAXED_DELTA, &val);
   if (!val) {
      return false;
   }

   info->devid = drm_intel_bufmgr_gem_get_devid(winsys->bufmgr);

   if (drm_intel_get_aperture_sizes(winsys->fd,
               &info->aperture_mappable, &info->aperture_total)) {
       return false;
   }

   get_param(winsys, I915_PARAM_HAS_LLC, &val);
   info->has_llc = val;
   info->has_address_swizzling = test_address_swizzling(winsys);

   winsys->ctx = drm_intel_gem_context_create(winsys->bufmgr);
   if (!winsys->ctx)
      return false;

   info->has_logical_context = (winsys->ctx != NULL);

   get_param(winsys, I915_PARAM_HAS_ALIASING_PPGTT, &val);
   info->has_ppgtt = val;

   /* test TIMESTAMP read */
   info->has_timestamp = test_reg_read(winsys, 0x2358);

   get_param(winsys, I915_PARAM_HAS_GEN7_SOL_RESET, &val);
   info->has_gen7_sol_reset = val;

   return true;
}

struct intel_winsys *
intel_winsys_create_for_fd(const struct icd_instance *instance, int fd)
{
   /* so that we can have enough relocs per bo */
   const int batch_size = sizeof(uint32_t) * 150 * 1024;
   struct intel_winsys *winsys;

   winsys = icd_instance_alloc(instance, sizeof(*winsys), 0,
           VK_SYSTEM_ALLOC_INTERNAL);
   if (!winsys)
      return NULL;

   memset(winsys, 0, sizeof(*winsys));

   winsys->instance = instance;
   winsys->fd = fd;

   winsys->bufmgr = drm_intel_bufmgr_gem_init(winsys->fd, batch_size);
   if (!winsys->bufmgr) {
      icd_instance_free(instance, winsys);
      return NULL;
   }

   if (!probe_winsys(winsys)) {
      drm_intel_bufmgr_destroy(winsys->bufmgr);
      icd_instance_free(instance, winsys);
      return NULL;
   }

   /*
    * No need to implicitly set up a fence register for each non-linear reloc
    * entry.  INTEL_RELOC_FENCE will be set on reloc entries that need them.
    */
   drm_intel_bufmgr_gem_enable_fenced_relocs(winsys->bufmgr);

   drm_intel_bufmgr_gem_enable_reuse(winsys->bufmgr);
   drm_intel_bufmgr_gem_set_vma_cache_size(winsys->bufmgr, -1);

   return winsys;
}

void
intel_winsys_destroy(struct intel_winsys *winsys)
{
   drm_intel_gem_context_destroy(winsys->ctx);
   drm_intel_bufmgr_destroy(winsys->bufmgr);
   icd_instance_free(winsys->instance, winsys);
}

const struct intel_winsys_info *
intel_winsys_get_info(const struct intel_winsys *winsys)
{
   return &winsys->info;
}

int
intel_winsys_read_reg(struct intel_winsys *winsys,
                      uint32_t reg, uint64_t *val)
{
   return drm_intel_reg_read(winsys->bufmgr, reg, val);
}

int
intel_winsys_get_reset_stats(struct intel_winsys *winsys,
                             uint32_t *active_lost,
                             uint32_t *pending_lost)
{
   uint32_t reset_count;

   return drm_intel_get_reset_stats(winsys->ctx,
         &reset_count, active_lost, pending_lost);
}

struct intel_bo *
intel_winsys_alloc_bo(struct intel_winsys *winsys,
                      const char *name,
                      unsigned long size,
                      bool cpu_init)
{
   const unsigned int alignment = 4096; /* always page-aligned */
   drm_intel_bo *bo;

   if (cpu_init) {
      bo = drm_intel_bo_alloc(winsys->bufmgr, name, size, alignment);
   } else {
      bo = drm_intel_bo_alloc_for_render(winsys->bufmgr,
            name, size, alignment);
   }

   return (struct intel_bo *) bo;
}

struct intel_bo *
intel_winsys_import_userptr(struct intel_winsys *winsys,
                            const char *name,
                            void *userptr,
                            unsigned long size,
                            unsigned long flags)
{
   drm_intel_bo *bo;

   bo = drm_intel_bo_alloc_userptr(winsys->bufmgr, name, userptr,
            INTEL_TILING_NONE, 0, size, flags);

   return (struct intel_bo *) bo;
}

struct intel_bo *
intel_winsys_import_handle(struct intel_winsys *winsys,
                           const char *name,
                           const struct intel_winsys_handle *handle,
                           unsigned long height,
                           enum intel_tiling_mode *tiling,
                           unsigned long *pitch)
{
   uint32_t real_tiling, swizzle;
   drm_intel_bo *bo;
   int err;

   switch (handle->type) {
   case INTEL_WINSYS_HANDLE_SHARED:
      {
         const uint32_t gem_name = handle->handle;
         bo = drm_intel_bo_gem_create_from_name(winsys->bufmgr,
               name, gem_name);
      }
      break;
   case INTEL_WINSYS_HANDLE_FD:
      {
         const int fd = (int) handle->handle;
         bo = drm_intel_bo_gem_create_from_prime(winsys->bufmgr,
               fd, height * handle->stride);
      }
      break;
   default:
      bo = NULL;
      break;
   }

   if (!bo)
      return NULL;

   err = drm_intel_bo_get_tiling(bo, &real_tiling, &swizzle);
   if (err) {
      drm_intel_bo_unreference(bo);
      return NULL;
   }

   *tiling = real_tiling;
   *pitch = handle->stride;

   return (struct intel_bo *) bo;
}

int
intel_winsys_export_handle(struct intel_winsys *winsys,
                           struct intel_bo *bo,
                           enum intel_tiling_mode tiling,
                           unsigned long pitch,
                           unsigned long height,
                           struct intel_winsys_handle *handle)
{
   int err = 0;

   switch (handle->type) {
   case INTEL_WINSYS_HANDLE_SHARED:
      {
         uint32_t name;

         err = drm_intel_bo_flink(gem_bo(bo), &name);
         if (!err)
            handle->handle = name;
      }
      break;
   case INTEL_WINSYS_HANDLE_KMS:
      handle->handle = gem_bo(bo)->handle;
      break;
   case INTEL_WINSYS_HANDLE_FD:
      {
         int fd;

         err = drm_intel_bo_gem_export_to_prime(gem_bo(bo), &fd);
         if (!err)
            handle->handle = fd;
      }
      break;
   default:
      err = -EINVAL;
      break;
   }

   if (err)
      return err;

   handle->stride = pitch;

   return 0;
}

bool
intel_winsys_can_submit_bo(struct intel_winsys *winsys,
                           struct intel_bo **bo_array,
                           int count)
{
   return !drm_intel_bufmgr_check_aperture_space((drm_intel_bo **) bo_array,
                                                 count);
}

int
intel_winsys_submit_bo(struct intel_winsys *winsys,
                       enum intel_ring_type ring,
                       struct intel_bo *bo, int used,
                       unsigned long flags)
{
   const unsigned long exec_flags = (unsigned long) ring | flags;
   drm_intel_context *ctx;

   /* logical contexts are only available for the render ring */
   ctx = (ring == INTEL_RING_RENDER) ? winsys->ctx : NULL;

   if (ctx) {
      return drm_intel_gem_bo_context_exec(gem_bo(bo),
            ctx, used, exec_flags);
   }
   else {
      return drm_intel_bo_mrb_exec(gem_bo(bo),
            used, NULL, 0, 0, exec_flags);
   }
}

void
intel_winsys_decode_bo(struct intel_winsys *winsys,
                       struct intel_bo *bo, int used)
{
   struct drm_intel_decode *decode;
   void *ptr;

   ptr = intel_bo_map(bo, false);
   if (!ptr) {
      return;
   }

   decode = drm_intel_decode_context_alloc(winsys->info.devid);
   if (!decode) {
      intel_bo_unmap(bo);
      return;
   }

   drm_intel_decode_set_output_file(decode, stderr);

   /* in dwords */
   used /= 4;

   drm_intel_decode_set_batch_pointer(decode,
         ptr, gem_bo(bo)->offset64, used);

   drm_intel_decode(decode);
   free(decode);
   intel_bo_unmap(bo);
}

struct intel_bo *
intel_bo_ref(struct intel_bo *bo)
{
   if (bo)
      drm_intel_bo_reference(gem_bo(bo));

   return bo;
}

void
intel_bo_unref(struct intel_bo *bo)
{
   if (bo)
      drm_intel_bo_unreference(gem_bo(bo));
}

int
intel_bo_set_tiling(struct intel_bo *bo,
                    enum intel_tiling_mode tiling,
                    unsigned long pitch)
{
   uint32_t real_tiling = tiling;
   int err;

   switch (tiling) {
   case INTEL_TILING_X:
      if (pitch % 512)
         return -1;
      break;
   case INTEL_TILING_Y:
      if (pitch % 128)
         return -1;
      break;
   default:
      break;
   }

   err = drm_intel_bo_set_tiling(gem_bo(bo), &real_tiling, pitch);
   if (err || real_tiling != tiling) {
      assert(!"tiling mismatch");
      return -1;
   }

   return 0;
}

void *
intel_bo_map(struct intel_bo *bo, bool write_enable)
{
   int err;

   err = drm_intel_bo_map(gem_bo(bo), write_enable);
   if (err) {
      return NULL;
   }

   return gem_bo(bo)->virtual;
}

void *
intel_bo_map_async(struct intel_bo *bo)
{
   int err;

   err = drm_intel_gem_bo_map_unsynchronized_non_gtt(gem_bo(bo));
   if (err) {
      return NULL;
   }

   return gem_bo(bo)->virtual;
}

void *
intel_bo_map_gtt(struct intel_bo *bo)
{
   int err;

   err = drm_intel_gem_bo_map_gtt(gem_bo(bo));
   if (err) {
      return NULL;
   }

   return gem_bo(bo)->virtual;
}

void *
intel_bo_map_gtt_async(struct intel_bo *bo)
{
   int err;

   err = drm_intel_gem_bo_map_unsynchronized(gem_bo(bo));
   if (err) {
      return NULL;
   }

   return gem_bo(bo)->virtual;
}

void
intel_bo_unmap(struct intel_bo *bo)
{
   int err U_ASSERT_ONLY;

   err = drm_intel_bo_unmap(gem_bo(bo));
   assert(!err);
}

int
intel_bo_pwrite(struct intel_bo *bo, unsigned long offset,
                unsigned long size, const void *data)
{
   return drm_intel_bo_subdata(gem_bo(bo), offset, size, data);
}

int
intel_bo_pread(struct intel_bo *bo, unsigned long offset,
               unsigned long size, void *data)
{
   return drm_intel_bo_get_subdata(gem_bo(bo), offset, size, data);
}

int
intel_bo_add_reloc(struct intel_bo *bo, uint32_t offset,
                   struct intel_bo *target_bo, uint32_t target_offset,
                   uint32_t flags, uint64_t *presumed_offset)
{
   uint32_t read_domains, write_domain;
   int err;

   if (flags & INTEL_RELOC_WRITE) {
      /*
       * Because of the translation to domains, INTEL_RELOC_GGTT should only
       * be set on GEN6 when the bo is written by MI_* or PIPE_CONTROL.  The
       * kernel will translate it back to INTEL_RELOC_GGTT.
       */
      write_domain = (flags & INTEL_RELOC_GGTT) ?
         I915_GEM_DOMAIN_INSTRUCTION : I915_GEM_DOMAIN_RENDER;
      read_domains = write_domain;
   } else {
      write_domain = 0;
      read_domains = I915_GEM_DOMAIN_RENDER |
                     I915_GEM_DOMAIN_SAMPLER |
                     I915_GEM_DOMAIN_INSTRUCTION |
                     I915_GEM_DOMAIN_VERTEX;
   }

   if (flags & INTEL_RELOC_FENCE) {
      err = drm_intel_bo_emit_reloc_fence(gem_bo(bo), offset,
            gem_bo(target_bo), target_offset,
            read_domains, write_domain);
   } else {
      err = drm_intel_bo_emit_reloc(gem_bo(bo), offset,
            gem_bo(target_bo), target_offset,
            read_domains, write_domain);
   }

   *presumed_offset = gem_bo(target_bo)->offset64 + target_offset;

   return err;
}

int
intel_bo_get_reloc_count(struct intel_bo *bo)
{
   return drm_intel_gem_bo_get_reloc_count(gem_bo(bo));
}

void
intel_bo_truncate_relocs(struct intel_bo *bo, int start)
{
   drm_intel_gem_bo_clear_relocs(gem_bo(bo), start);
}

bool
intel_bo_has_reloc(struct intel_bo *bo, struct intel_bo *target_bo)
{
   return drm_intel_bo_references(gem_bo(bo), gem_bo(target_bo));
}

int
intel_bo_wait(struct intel_bo *bo, int64_t timeout)
{
   int err = 0;

   if (timeout >= 0)
       err = drm_intel_gem_bo_wait(gem_bo(bo), timeout);
   else
       drm_intel_bo_wait_rendering(gem_bo(bo));

   /* consider the bo idle on errors */
   if (err && err != -ETIME)
      err = 0;

   return err;
}
