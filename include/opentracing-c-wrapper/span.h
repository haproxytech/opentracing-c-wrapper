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
#ifndef OPENTRACING_C_WRAPPER_SPAN_H
#define OPENTRACING_C_WRAPPER_SPAN_H

__CPLUSPLUS_DECL_BEGIN

/***
 * Encode a key-value for logging
 */
struct otc_log_field {
	/***
	 * key string
	 */
	const char *key;

	/***
	 * representation of the value
	 */
	struct otc_value value;
};

/***
 * A log entry for events that occured during the span lifetime
 */
struct otc_log_record {
	/***
	 * event timestamp
	 */
	struct otc_timestamp timestamp;

	/***
	 * array of log fields
	 */
	struct otc_log_field *fields;

	/***
	 * number of log fields, must be set to zero if fields is NULL
	 */
	int num_fields;
};

typedef enum {
	/***
	 * refers to the parent span that caused it and somehow depends on the new child span
	 */
	otc_span_reference_child_of = 1,

	/***
	 * parent span that doesn't depend in any way on the result of the new child span
	 *
	 * more details in http://opentracing.io/spec/
	 */
	otc_span_reference_follows_from = 2
} otc_span_reference_type_t;

struct otc_span_reference {
	otc_span_reference_type_t  type;
	struct otc_span_context   *referenced_context;
};

struct otc_finish_span_options {
	/***
	 * time when the span finished (monotonic clock)
	 */
	struct otc_duration finish_time;

	/***
	 * array with otc_log_record entries
	 */
	const struct otc_log_record *log_records;

	/***
	 * number of log records, must be set to zero if log_records is NULL
	 */
	int num_log_records;
};

/***
 * The span interface
 */
struct otc_span {
	int64_t idx;

	/***
	 * NAME
	 *   finish -
	 *
	 * ARGUMENTS
	 *   span - span instance
	 *
	 * DESCRIPTION
	 *   sets the ending timestamp and finalizes span. Must be the last call for a span instance (except calls to context)
	 */
	void (*finish)(struct otc_span *span)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   finish_with_options -
	 *
	 * ARGUMENTS
	 *   span    - span instance
	 *   options - override span completion with these options
	 *
	 * DESCRIPTION
	 *   finalizes the span but allows for customizing the timestamp and log data
	 */
	void (*finish_with_options)(struct otc_span *span, const struct otc_finish_span_options *options)
		OTC_NONNULL(1);

	/***
	 * NAME
	 *   otc_span_context -
	 *
	 * ARGUMENTS
	 *   span - span instance
	 *
	 * DESCRIPTION
	 *   returns an otc_span_context for the span
	 *
	 * RETURN VALUE
	 *  span - context for this span
	 */
	struct otc_span_context *(*span_context)(struct otc_span *span)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   set_operation_name -
	 *
	 * ARGUMENTS
	 *   span           - span instance
	 *   operation_name - new operation name
	 *
	 * DESCRIPTION
	 *   set or change the operation name
	 */
	void (*set_operation_name)(struct otc_span *span, const char *operation_name)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   set_tag
	 *
	 * ARGUMENTS
	 *   span  - span instance
	 *   key   - tag key to copy into the span
	 *   value - tag value to copy into the span
	 *
	 * DESCRIPTION
	 *   add a tag to a span, overwriting any existing tag with the
	 *   same key. Tag values supported are strings, numbers and bools
	 */
	void (*set_tag)(struct otc_span *span, const char *key, const struct otc_value *value)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   log_fields -
	 *
	 * ARGUMENTS
	 *   span       - span instance
	 *   fields     - log fields as an array, values should be copied by the implementation
	 *   num_fields - number of log fields in the log field array
	 *
	 * DESCRIPTION
	 *   store log data for a span
	 */
	void (*log_fields)(struct otc_span *span, const struct otc_log_field *fields, int num_fields)
		OTC_NONNULL(1);

	/***
	 * NAME
	 *   set_baggage_item -
	 *
	 * ARGUMENTS
	 *   span   - span instance
	 *   key    - baggage key
	 *   value  - baggage value
	 *
	 * DESCRIPTION
	 *   store a key-value pair to this span and the associated span
	 *   context, also propagating to descendents
	 */
	void (*set_baggage_item)(struct otc_span *span, const char *key, const char *value)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   baggage_item -
	 *
	 * ARGUMENTS
	 *   span - span instance
	 *   key  - baggage key
	 *
	 * DESCRIPTION
	 *   get the baggage for a baggage key
	 *
	 * RETURN VALUE
	 *   baggage value for the key, or empty string if the key doesn't exist
	 */
	const char *(*baggage_item)(const struct otc_span *span, const char *key)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   tracer -
	 *
	 * ARGUMENTS
	 *   span - span instance
	 *
	 * DESCRIPTION
	 *   returns the tracer that created the span
	 *
	 * RETURN VALUE
	 *   tracer instance that creaated the span
	 */
	struct otc_tracer *(*tracer)(const struct otc_span *span)
		OTC_NONNULL_ALL;

	/***
	 * NAME
	 *   destroy -
	 *
	 * ARGUMENTS
	 *   span - span instance
	 */
	void (*destroy)(struct otc_span **span)
		OTC_NONNULL_ALL;
};

/***
 * interface for span context
 */
struct otc_span_context {
	int64_t idx;
	const struct otc_span *span;

	/***
	 * NAME
	 *   destroy -
	 *
	 * ARGUMENTS
	 *   context - instance of span context
	 */
	void (*destroy)(struct otc_span_context **context)
		OTC_NONNULL_ALL;
};

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_SPAN_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
