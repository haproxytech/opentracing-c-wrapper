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
#ifndef OPENTRACING_C_WRAPPER_COMMON_H
#define OPENTRACING_C_WRAPPER_COMMON_H

__CPLUSPLUS_DECL_BEGIN

/***
 * boolean type
 */
typedef enum {
	otc_false = 0,
	otc_true  = 1,
} otc_bool_t;

/***
 * duration type for calculating intervals (monotonic)
 */
struct otc_duration {
	struct timespec value;
};

/***
 * timestamp type for absolute time
 */
struct otc_timestamp {
	struct timespec value;
};

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_COMMON_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
