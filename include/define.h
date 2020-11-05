/***
 * Copyright 2020 HAProxy Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _OPENTRACING_C_WRAPPER_DEFINE_H_
#define _OPENTRACING_C_WRAPPER_DEFINE_H_

#undef  OT_THREADS_NO_LOCKING

#ifdef USE_THREADS
#  define __THR                     __thread
#else
#  define __THR
#endif

#ifdef DEBUG
#  define OTC_DBG_MEM
#  define OT_IFDEF_DBG(a,b)         a
#  define OT_EXT_MALLOC(s)          otc_ext_malloc(__func__, __LINE__, (s))
#  define OT_EXT_FREE_CLEAR(a)      do { if ((a) != nullptr) { otc_ext_free(__func__, __LINE__, a); (a) = nullptr; } } while (0)
#else
#  define OT_IFDEF_DBG(a,b)         b
#  define OT_EXT_MALLOC(s)          otc_ext_malloc(s)
#  define OT_EXT_FREE_CLEAR(a)      do { if ((a) != nullptr) { otc_ext_free(a); (a) = nullptr; } } while (0)
#endif

#define OT_FREE(a)                  do { if ((a) != nullptr) OTC_DBG_FREE(a); } while (0)
#define OT_FREE_CLEAR(a)            do { if ((a) != nullptr) { OTC_DBG_FREE(a); (a) = nullptr; } } while (0)

#define OT_IN_RANGE(v,a,b)          (((v) >= (a)) && ((v) <= (b)))
#define OT_SPAN_KEY_IS_VALID(a)     OT_IN_RANGE((a)->idx, 0, ot_span.key - 1)
#define OT_SPAN_IS_VALID(a)         (((a) != nullptr) && OT_SPAN_KEY_IS_VALID(a))
#define OT_CTX_KEY_IS_VALID(a)      OT_IN_RANGE((a)->idx, 0, ot_span_context.key - 1)
#define OT_CTX_IS_VALID(a)          (((a) != nullptr) && (OT_SPAN_IS_VALID((a)->span) || OT_CTX_KEY_IS_VALID(a)))

#define OT_CAST_CONST(t,e)          const_cast<t>(e)
#define OT_CAST_STAT(t,e)           static_cast<t>(e)
#define OT_CAST_REINTERPRET(t,e)    reinterpret_cast<t>(e)
#define OT_CAST_TYPEOF(t,e)         OT_CAST_REINTERPRET(typeof(t), (e))

#ifdef __cplusplus
#  define __CPLUSPLUS_DECL_BEGIN    extern "C" {
#  define __CPLUSPLUS_DECL_END      }
#else
#  define __CPLUSPLUS_DECL_BEGIN
#  define __CPLUSPLUS_DECL_END
#endif

#endif /* _OPENTRACING_C_WRAPPER_DEFINE_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
