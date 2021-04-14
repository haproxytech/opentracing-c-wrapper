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
#ifndef _OPENTRACING_C_WRAPPER_UTIL_H_
#define _OPENTRACING_C_WRAPPER_UTIL_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


/* Parameter 'p' must not be in parentheses! */
#define OT_TEXT_MAP_SIZE(p,n)   (sizeof(text_map->p) * (text_map->size + (n)))


extern otc_ext_malloc_t otc_ext_malloc;
extern otc_ext_free_t   otc_ext_free;


std::chrono::nanoseconds  timespec_to_duration(const struct timespec *ts);
const char               *otc_strerror(int errnum);

#endif /* _OPENTRACING_C_WRAPPER_UTIL_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
