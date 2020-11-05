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
#ifndef OPENTRACING_C_WRAPPER_DEFINE_H
#define OPENTRACING_C_WRAPPER_DEFINE_H

#define OTC_MAXLOGFIELDS           8

#ifdef __cplusplus
#  define __CPLUSPLUS_DECL_BEGIN   extern "C" {
#  define __CPLUSPLUS_DECL_END     }
#else
#  define __CPLUSPLUS_DECL_BEGIN
#  define __CPLUSPLUS_DECL_END
#endif

#ifdef __GNUC__
#  define OTC_NONNULL(...)         __attribute__((nonnull(__VA_ARGS__)))
#  define OTC_NONNULL_ALL          __attribute__((nonnull))
#else
#  define OTC_NONNULL(...)
#  define OTC_NONNULL_ALL
#endif

#endif /* OPENTRACING_C_WRAPPER_DEFINE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
