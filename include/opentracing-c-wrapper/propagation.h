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
#ifndef OPENTRACING_C_WRAPPER_PROPAGATION_H
#define OPENTRACING_C_WRAPPER_PROPAGATION_H

__CPLUSPLUS_DECL_BEGIN

/***
 * span context propagation error codes
 */
typedef enum {
	/***
	 * success
	 */
	otc_propagation_error_code_success = 0,

	/***
	 * not enough information to extract a span context
	 */
	otc_propagation_error_code_span_context_not_found = -2,

	/***
	 * invalid/incompatible span context
	 */
	otc_propagation_error_code_invalid_span_context = -3,

	/***
	 * invalid carrier
	 */
	otc_propagation_error_code_invalid_carrier = -4,

	/***
	 * corrupted carrier
	 */
	otc_propagation_error_code_span_context_corrupted = -5,

	/***
	 * unknown error
	 */
	otc_propagation_error_code_unknown = -6,

	/***
	 * tracer not initialized
	 */
	otc_propagation_error_code_invalid_tracer = -7,

} otc_propagation_error_code_t;


/***
 * Used to set information into a span context for propagation, entries
 * are strings in a map of string pointers
 *
 * set and get implementations should use the same convention
 * for naming the keys they manipulate
 */
struct otc_text_map_writer {
	struct otc_text_map text_map;

	/***
	 * NAME
	 *   set -
	 *
	 * ARGUMENTS
	 *   writer - writer instance
	 *   key    - string key
	 *   value  - string value
	 *
	 * DECRTIPTION
	 *   set a key-value pair
	 *
	 * RETURN VALUE
	 *   otc_propagation_error_code_t - indicates success or failure
	 */
	otc_propagation_error_code_t (*set)(struct otc_text_map_writer *writer, const char *key, const char *value)
		OTC_NONNULL(1);
};

/***
 * Used to get information from a propagated span context
 *
 * set and get implementations should use the same convention
 * for naming the keys they manipulate
 */
struct otc_text_map_reader {
	struct otc_text_map text_map;

	/***
	 * NAME
	 *   foreach_key -
	 *
	 * ARGUMENTS
	 *   reader  - reader instance
	 *   handler - handler function for each key-value pair
	 *   arg     - user-defined contents
	 *
	 * DESCRIPTION
	 *   gets map entries by colling the handler function repeatedly. Returns immediately upon first error
	 *
	 * RETURN VALUE
	 *   - error code indicating success or failure.
	 */
	otc_propagation_error_code_t (*foreach_key)(struct otc_text_map_reader *reader, otc_propagation_error_code_t (*handler)(void *arg, const char *key, const char *value), void *arg)
		OTC_NONNULL(1, 2);
};

/***
 * Used to set HTTP headers
 */
struct otc_http_headers_writer {
	struct otc_text_map text_map;

	otc_propagation_error_code_t (*set)(struct otc_http_headers_writer *writer, const char *key, const char *value)
		OTC_NONNULL(1);
};

/***
 * USed to get HTTP headers
 */
struct otc_http_headers_reader {
	struct otc_text_map text_map;

	otc_propagation_error_code_t (*foreach_key)(struct otc_http_headers_reader *reader, otc_propagation_error_code_t (*handler)(void *arg, const char *key, const char *value), void *arg)
		OTC_NONNULL(1, 2);
};

/***
 * Set/write to a custom format
 */
struct otc_custom_carrier_writer {
	struct otc_binary_data binary_data;

	/***
	 * NAME
	 *   inject -
	 *
	 * ARGUMENTS
	 *   writer       - writer instance
	 *   tracer       - tracer instance
	 *   span_context - span context to write
	 * DESCRIPTION
	 *   write a span context into a custom format
	 *
	 * RETURN VALUE
	 *   - error code indicating success or failure
	 */
	otc_propagation_error_code_t (*inject)(struct otc_custom_carrier_writer *writer, const struct otc_tracer *tracer, const struct otc_span_context *span_context)
		OTC_NONNULL_ALL;
};

/***
 * Get from a custom format
 */
struct otc_custom_carrier_reader {
	struct otc_binary_data binary_data;

	/***
	 * NAME
	 *   extract -
	 *
	 * ARGUMENTS
	 *   reader       - reader instance
	 *   tracer       - tracer instance
	 *   span_context - span context pointer to return the decoded
	 *		            span (can be NULL if propagation failed
	 *                  or OOM)
	 *
	 * DESCRIPTION
	 *   - get a span context from a custom format
	 *
	 * RETURN VALUE
	 *   - error code indicating success or failure
	 */
	otc_propagation_error_code_t (*extract)(struct otc_custom_carrier_reader *reader, const struct otc_tracer *tracer, struct otc_span_context **span_context)
		OTC_NONNULL_ALL;
};

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_PROPAGATION_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
