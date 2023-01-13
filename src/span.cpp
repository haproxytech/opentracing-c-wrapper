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
#include "include.h"


#ifdef OT_THREADS_NO_LOCKING
thread_local Handle<opentracing::Span>        ot_span_handle;
thread_local Handle<opentracing::SpanContext> ot_span_context_handle;
struct HandleData                             ot_span;
struct HandleData                             ot_span_context;
#else
struct Handle<opentracing::Span>              ot_span;
struct Handle<opentracing::SpanContext>       ot_span_context;
#endif /* OT_THREADS_NO_LOCKING */


/***
 * NAME
 *   ot_span_finish_with_options -
 *
 * ARGUMENTS
 *   span    -
 *   options -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_finish_with_options(struct otc_span *span, const struct otc_finish_span_options *options)
{
	OT_LOCK_GUARD(span);

	if (!OT_SPAN_IS_VALID(span))
		return;

	if (options == nullptr) {
		ot_span_handle.at(span->idx)->Finish();
	} else {
		struct opentracing::FinishSpanOptions span_options;

		if (options->finish_time.value.tv_sec > 0) {
			auto dt = timespec_to_duration(&(options->finish_time.value));

			span_options.finish_steady_timestamp = std::chrono::time_point<std::chrono::steady_clock>(dt);
		}

		if (options->log_records != nullptr) {
			for (int i = 0; i < options->num_log_records; i++) {
				struct opentracing::LogRecord record;

				if (options->log_records[i].timestamp.value.tv_sec > 0) {
#ifdef __clang__
					auto dt = timespec_to_duration_us(&(options->log_records[i].timestamp.value));
#else
					auto dt = timespec_to_duration(&(options->log_records[i].timestamp.value));
#endif

					record.timestamp = std::chrono::time_point<std::chrono::system_clock>(dt);
				}

				for (int j = 0; j < options->log_records[i].num_fields; j++)
					if (options->log_records[i].fields[j].value.type == otc_value_bool) {
						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, options->log_records[i].fields[j].value.value.bool_value));
					}
					else if (options->log_records[i].fields[j].value.type == otc_value_double) {
						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, options->log_records[i].fields[j].value.value.double_value));
					}
					else if (options->log_records[i].fields[j].value.type == otc_value_int64) {
						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, options->log_records[i].fields[j].value.value.int64_value));
					}
					else if (options->log_records[i].fields[j].value.type == otc_value_uint64) {
						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, options->log_records[i].fields[j].value.value.uint64_value));
					}
					else if (options->log_records[i].fields[j].value.type == otc_value_string) {
						std::string str_value = options->log_records[i].fields[j].value.value.string_value;

						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, str_value));
					}
					else if (options->log_records[i].fields[j].value.type == otc_value_null) {
						record.fields.push_back(std::make_pair(options->log_records[i].fields[j].key, nullptr));
					}

				span_options.log_records.push_back(record);
			}
		}

		ot_span_handle.at(span->idx)->FinishWithOptions(span_options);
	}

	ot_nolock_span_destroy(&span);
}


/***
 * NAME
 *   ot_span_finish -
 *
 * ARGUMENTS
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_finish(struct otc_span *span)
{
	ot_span_finish_with_options(span, nullptr);
}


/***
 * NAME
 *   ot_span_get_context -
 *
 * ARGUMENTS
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static struct otc_span_context *ot_span_get_context(struct otc_span *span)
{
	OT_LOCK(span, span_context);

	if (!OT_SPAN_IS_VALID(span))
		return nullptr;

	return ot_span_context_new(span);
}


/***
 * NAME
 *   ot_span_set_operation_name -
 *
 * ARGUMENTS
 *   span           -
 *   operation_name -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_set_operation_name(struct otc_span *span, const char *operation_name)
{
	OT_LOCK_GUARD(span);

	if (!OT_SPAN_IS_VALID(span) || (operation_name == nullptr))
		return;

	ot_span_handle.at(span->idx)->SetOperationName(operation_name);
}


/***
 * NAME
 *   ot_span_set_tag -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_set_tag(struct otc_span *span, const char *key, const struct otc_value *value)
{
	OT_LOCK_GUARD(span);

	if (!OT_SPAN_IS_VALID(span) || (key == nullptr) || (value == nullptr))
		return;

	if (value->type == otc_value_bool) {
		ot_span_handle.at(span->idx)->SetTag(key, value->value.bool_value);
	}
	else if (value->type == otc_value_double) {
		ot_span_handle.at(span->idx)->SetTag(key, value->value.double_value);
	}
	else if (value->type == otc_value_int64) {
		ot_span_handle.at(span->idx)->SetTag(key, value->value.int64_value);
	}
	else if (value->type == otc_value_uint64) {
		ot_span_handle.at(span->idx)->SetTag(key, value->value.uint64_value);
	}
	else if (value->type == otc_value_string) {
		std::string str_value = value->value.string_value;

		ot_span_handle.at(span->idx)->SetTag(key, str_value);
	}
	else if (value->type == otc_value_null) {
		ot_span_handle.at(span->idx)->SetTag(key, nullptr);
	}
	else {
		/* Do nothing. */
	}
}


/***
 * NAME
 *   ot_span_log_fields -
 *
 * ARGUMENTS
 *   span       -
 *   fields     -
 *   num_fields -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_log_fields(struct otc_span *span, const struct otc_log_field *fields, int num_fields)
{
	OT_LOCK_GUARD(span);
	std::string str_value[OTC_MAXLOGFIELDS];

	if (!OT_SPAN_IS_VALID(span) || (fields == nullptr) || !OT_IN_RANGE(num_fields, 1, OTC_MAXLOGFIELDS))
		return;

	/* XXX  The only data type supported in this function is string. */
	for (int i = 0; (i < num_fields) && (i < OTC_MAXLOGFIELDS); i++) {
		if (fields[i].value.type != otc_value_string)
			str_value[i] = "invalid data type";
		else if (fields[i].value.value.string_value != nullptr)
			str_value[i] = fields[i].value.value.string_value;
		else
			str_value[i] = "";
	}

	if (num_fields == 1)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0) });
	else if (num_fields == 2)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1) });
	else if (num_fields == 3)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2) });
	else if (num_fields == 4)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2), OT_LF(3) });
	else if (num_fields == 5)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2), OT_LF(3), OT_LF(4) });
	else if (num_fields == 6)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2), OT_LF(3), OT_LF(4), OT_LF(5) });
	else if (num_fields == 7)
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2), OT_LF(3), OT_LF(4), OT_LF(5), OT_LF(6) });
	else
		ot_span_handle.at(span->idx)->Log({ OT_LF(0), OT_LF(1), OT_LF(2), OT_LF(3), OT_LF(4), OT_LF(5), OT_LF(6), OT_LF(7) });
}


/***
 * NAME
 *   ot_span_set_baggage_item -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_set_baggage_item(struct otc_span *span, const char *key, const char *value)
{
	OT_LOCK_GUARD(span);

	if (!OT_SPAN_IS_VALID(span) || (key == nullptr) || (value == nullptr))
		return;

	ot_span_handle.at(span->idx)->SetBaggageItem(key, value);
}


/***
 * NAME
 *   ot_span_baggage_item -
 *
 * ARGUMENTS
 *   span -
 *   key  -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static const char *ot_span_baggage_item(const struct otc_span *span, const char *key)
{
	OT_LOCK_GUARD(span);
	const char *retptr = "";

	if (!OT_SPAN_IS_VALID(span) || (key == nullptr))
		return retptr;

	auto baggage = ot_span_handle.at(span->idx)->BaggageItem(key);
	if (!baggage.empty())
		retptr = OTC_DBG_STRDUP(baggage.c_str());

	return retptr;
}


/***
 * NAME
 *   ot_span_tracer -
 *
 * ARGUMENTS
 *   span - NOT USED
 *
 * DESCRIPTION
 *   - NOT IMPLEMENTED
 *
 * RETURN VALUE
 *   -
 */
static struct otc_tracer *ot_span_tracer(const struct otc_span *span)
{
	struct otc_tracer *retptr = nullptr;

	if (!OT_SPAN_IS_VALID(span))
		return retptr;

	return retptr;
}


/***
 * NAME
 *   ot_nolock_span_destroy -
 *
 * ARGUMENTS
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_nolock_span_destroy(struct otc_span **span)
{
	if ((span == nullptr) || ((*span) == nullptr))
		return;

	if (OT_SPAN_KEY_IS_VALID(*span)) {
		ot_span_handle.erase((*span)->idx);
		ot_span.erase_cnt++;
	}

	ot_span.destroy_cnt++;

	OT_EXT_FREE_CLEAR(*span);
}


/***
 * NAME
 *   ot_span_destroy -
 *
 * ARGUMENTS
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_destroy(struct otc_span **span)
{
	OT_LOCK_GUARD(span);

	ot_nolock_span_destroy(span);
}


/***
 * NAME
 *   ot_span_new -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span *ot_span_new(void)
{
	const static struct otc_span span_init = {
		.idx                 = 0,
		.finish              = ot_span_finish,              /* lock span */
		.finish_with_options = ot_span_finish_with_options, /* lock span */
		.span_context        = ot_span_get_context,         /* lock span and span_context */
		.set_operation_name  = ot_span_set_operation_name,  /* lock span */
		.set_tag             = ot_span_set_tag,             /* lock span */
		.log_fields          = ot_span_log_fields,          /* lock span */
		.set_baggage_item    = ot_span_set_baggage_item,    /* lock span */
		.baggage_item        = ot_span_baggage_item,        /* lock span */
		.tracer              = ot_span_tracer,              /* NOT IMPLEMENTED */
		.destroy             = ot_span_destroy              /* lock span */
	};
	int64_t          idx = ot_span.key++;
	struct otc_span *retptr;

	if (idx == 0) {
		ot_span_handle.clear();
		ot_span_handle.reserve(8192);
	}

	if ((retptr = OT_CAST_TYPEOF(retptr, OT_EXT_MALLOC(sizeof(*retptr)))) != nullptr) {
		(void)memcpy(retptr, &span_init, sizeof(*retptr));
		retptr->idx = idx;
	} else {
		ot_span.alloc_fail_cnt++;
	}

	return retptr;
}


/***
 * NAME
 *   ot_nolock_span_context_destroy -
 *
 * ARGUMENTS
 *   context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_nolock_span_context_destroy(struct otc_span_context **context)
{
	if ((context == nullptr) || (*context == nullptr))
		return;

	if (OT_CTX_KEY_IS_VALID(*context)) {
		ot_span_context_handle.erase((*context)->idx);
		ot_span_context.erase_cnt++;
	}

	ot_span_context.destroy_cnt++;

	OT_EXT_FREE_CLEAR(*context);
}


/***
 * NAME
 *   ot_span_context_destroy -
 *
 * ARGUMENTS
 *   context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_span_context_destroy(struct otc_span_context **context)
{
	OT_LOCK_GUARD(span_context);

	ot_nolock_span_context_destroy(context);
}


/***
 * NAME
 *   ot_span_context_new -
 *
 * ARGUMENTS
 *   span -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_span_context_new(const struct otc_span *span)
{
	int64_t                  idx = ot_span_context.key++;
	struct otc_span_context *retptr;

	if (idx == 0) {
		ot_span_context_handle.clear();
		ot_span_context_handle.reserve(8192);
	}

	if ((retptr = OT_CAST_TYPEOF(retptr, OT_EXT_MALLOC(sizeof(*retptr)))) == nullptr) {
		ot_span_context.alloc_fail_cnt++;

		return retptr;
	}

	retptr->idx     = (span == nullptr) ? idx : -1;
	retptr->span    = span;
	retptr->destroy = ot_span_context_destroy; /* lock span_context */

	return retptr;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
