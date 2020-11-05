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
#ifndef OPENTRACING_C_WRAPPER_VALUE_H
#define OPENTRACING_C_WRAPPER_VALUE_H

__CPLUSPLUS_DECL_BEGIN

/***
 * value types
 */
typedef enum {
	otc_value_bool = 0,
	otc_value_double,
	otc_value_int64,
	otc_value_uint64,
	otc_value_string,
	otc_value_null,
} otc_value_type_t;


/***
 * union for representing various value types
 */
struct otc_value {

	otc_value_type_t type;

	union {
		otc_bool_t bool_value;
		double double_value;
		int64_t int64_value;
		uint64_t uint64_value;
		const char *string_value;
	} value;
};

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_VALUE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
