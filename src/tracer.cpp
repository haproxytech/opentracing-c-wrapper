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


static std::unique_ptr<const opentracing::DynamicTracingLibraryHandle> ot_dynlib = nullptr;
static std::shared_ptr<opentracing::Tracer>                            ot_tracer = nullptr;


/***
 * NAME
 *   ot_tracer_load -
 *
 * ARGUMENTS
 *   library   -
 *   errbuf    -
 *   errbufsiz -
 *   handle    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static int ot_tracer_load(const char *library, char *errbuf, int errbufsiz, opentracing::DynamicTracingLibraryHandle &handle)
{
	std::string errmsg;

	/* Load the tracer library. */
	auto handle_maybe = opentracing::DynamicallyLoadTracingLibrary(library, errmsg);
	if (!handle_maybe) {
		(void)snprintf(errbuf, errbufsiz, "Failed to load tracing library: %s", errmsg.empty() ? handle_maybe.error().message().c_str() : errmsg.c_str());

		return -1;
	}

	handle = std::move(*handle_maybe);

	return 0;
}


/***
 * NAME
 *   ot_tracer_start -
 *
 * ARGUMENTS
 *   config    -
 *   errbuf    -
 *   errbufsiz -
 *   tracer    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static int ot_tracer_start(const char *config, char *errbuf, int errbufsiz, std::shared_ptr<opentracing::Tracer> &tracer)
{
	std::string errmsg;

	auto &tracer_factory = ot_dynlib->tracer_factory();

      /* Create a tracer with the requested configuration. */
	auto tracer_maybe = tracer_factory.MakeTracer(config, errmsg);
	if (!tracer_maybe) {
		(void)snprintf(errbuf, errbufsiz, "Failed to construct tracer: %s", errmsg.empty() ? tracer_maybe.error().message().c_str() : errmsg.c_str());

		return -1;
	}

	tracer = std::move(*tracer_maybe);

	return 0;
}


/***
 * NAME
 *   ot_tracer_close -
 *
 * ARGUMENTS
 *   tracer -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_tracer_close(struct otc_tracer *tracer)
{
	if (tracer == nullptr)
		return;

	if (ot_dynlib == nullptr)
		return;

	if (ot_tracer != nullptr)
		ot_tracer->Close();
	tracer->destroy(&tracer);
}


/***
 * NAME
 *   ot_tracer_start_span_with_options -
 *
 * ARGUMENTS
 *   tracer         - NOT USED
 *   operation_name -
 *   options        -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static struct otc_span *ot_tracer_start_span_with_options(struct otc_tracer *tracer, const char *operation_name, const struct otc_start_span_options *options)
{
	OT_LOCK_GUARD(span);
	std::unique_ptr<opentracing::Span>  span_maybe = nullptr;
	struct otc_span            *retptr = nullptr;

	if (ot_tracer == nullptr)
		return retptr;
	else if ((tracer == nullptr) || (operation_name == nullptr))
		return retptr;

	/* Allocating memory for the span. */
	if ((retptr = ot_span_new()) == nullptr)
		return retptr;

	if (options == nullptr) {
		span_maybe = ot_tracer->StartSpan(operation_name);
	} else {
		struct opentracing::StartSpanOptions span_options;

		if (options->start_time_steady.value.tv_sec > 0) {
			auto dt = timespec_to_duration(&(options->start_time_steady.value));

			span_options.start_steady_timestamp = std::chrono::time_point<std::chrono::steady_clock>(dt);
		}

		if (options->start_time_system.value.tv_sec > 0) {
#ifdef __clang__
			auto dt = timespec_to_duration_us(&(options->start_time_system.value));
#else
			auto dt = timespec_to_duration(&(options->start_time_system.value));
#endif

			span_options.start_system_timestamp = std::chrono::time_point<std::chrono::system_clock>(dt);
		}

		if (options->references != nullptr) {
			int lock_cnt = 0;

			for (int i = 0; i < options->num_references; i++) {
				const opentracing::SpanContext *context = nullptr;

				if (OT_SPAN_IS_VALID(options->references[i].referenced_context->span)) {
					context = &(ot_span_handle.at(options->references[i].referenced_context->span->idx)->context());
				}
				else if (OT_CTX_KEY_IS_VALID(options->references[i].referenced_context)) {
#ifdef OT_THREADS_NO_LOCKING
					lock_cnt++;
#else
					if (lock_cnt++ == 0)
						ot_span_context.mutex.lock();
#endif

					context = ot_span_context_handle.at(options->references[i].referenced_context->idx).get();
				}

				if (options->references[i].type == otc_span_reference_child_of)
					span_options.references.push_back(std::make_pair(opentracing::SpanReferenceType::ChildOfRef, context));
				else if (options->references[i].type == otc_span_reference_follows_from)
					span_options.references.push_back(std::make_pair(opentracing::SpanReferenceType::FollowsFromRef, context));
			}

#ifndef OT_THREADS_NO_LOCKING
			if (lock_cnt > 0)
				ot_span_context.mutex.unlock();
#endif
		}

		if (options->tags != nullptr) {
			for (int i = 0; i < options->num_tags; i++)
				if (options->tags[i].value.type == otc_value_bool) {
					span_options.tags.push_back(std::make_pair(options->tags[i].key, options->tags[i].value.value.bool_value));
				}
				else if (options->tags[i].value.type == otc_value_double) {
					span_options.tags.push_back(std::make_pair(options->tags[i].key, options->tags[i].value.value.double_value));
				}
				else if (options->tags[i].value.type == otc_value_int64) {
					span_options.tags.push_back(std::make_pair(options->tags[i].key, options->tags[i].value.value.int64_value));
				}
				else if (options->tags[i].value.type == otc_value_uint64) {
					span_options.tags.push_back(std::make_pair(options->tags[i].key, options->tags[i].value.value.uint64_value));
				}
				else if (options->tags[i].value.type == otc_value_string) {
					std::string str_value = options->tags[i].value.value.string_value;

					span_options.tags.push_back(std::make_pair(options->tags[i].key, str_value));
				}
				else if (options->tags[i].value.type == otc_value_null) {
					span_options.tags.push_back(std::make_pair(options->tags[i].key, nullptr));
				}
		}

		span_maybe = ot_tracer->StartSpanWithOptions(operation_name, span_options);
	}

	if (span_maybe != nullptr)
		ot_span_handle.emplace(retptr->idx, std::move(span_maybe));
	else
		ot_nolock_span_destroy(&retptr);

	return retptr;
}


/***
 * NAME
 *   ot_tracer_start_span -
 *
 * ARGUMENTS
 *   tracer         - NOT USED
 *   operation_name -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static struct otc_span *ot_tracer_start_span(struct otc_tracer *tracer, const char *operation_name)
{
	return ot_tracer_start_span_with_options(tracer, operation_name, nullptr);
}


/***
 * NAME
 *   ot_tracer_inject_text_map -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_inject_text_map(struct otc_tracer *tracer, struct otc_text_map_writer *carrier, const struct otc_span_context *span_context)
{
	TextMap                     text_map;
	TextMapCarrier              text_map_carrier(text_map);
	opentracing::expected<void> rc;

	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (!OT_CTX_IS_VALID(span_context))
		return otc_propagation_error_code_span_context_corrupted;

	if (OT_SPAN_IS_VALID(span_context->span)) {
		OT_LOCK_GUARD(span);

		rc = ot_tracer->Inject(ot_span_handle.at(span_context->span->idx)->context(), text_map_carrier);
	}
	else if (OT_CTX_KEY_IS_VALID(span_context)) {
		OT_LOCK_GUARD(span_context);

		rc = ot_tracer->Inject(*(ot_span_context_handle.at(span_context->idx)), text_map_carrier);
	}

	if (!rc)
		return otc_propagation_error_code_unknown;
	else if (text_map.empty())
		return otc_propagation_error_code_unknown;
	else if (otc_text_map_new(&(carrier->text_map), text_map.size()) == nullptr)
		return otc_propagation_error_code_unknown;

	for (auto const &it : text_map)
		if (carrier->set != nullptr) {
			otc_propagation_error_code_t retval = carrier->set(carrier, it.first.c_str(), it.second.c_str());
			if (retval != otc_propagation_error_code_success)
				return retval;
		}
		else if (otc_text_map_add(&(carrier->text_map), it.first.c_str(), 0, it.second.c_str(), 0, OT_CAST_STAT(otc_text_map_flags_t, OTC_TEXT_MAP_DUP_KEY | OTC_TEXT_MAP_DUP_VALUE)) == -1)
			return otc_propagation_error_code_unknown;

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_tracer_inject_http_headers -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_inject_http_headers(struct otc_tracer *tracer, struct otc_http_headers_writer *carrier, const struct otc_span_context *span_context)
{
	TextMap                     text_map;
	HTTPHeadersCarrier          http_headers_carrier(text_map);
	opentracing::expected<void> rc;

	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (!OT_CTX_IS_VALID(span_context))
		return otc_propagation_error_code_span_context_corrupted;

	if (OT_SPAN_IS_VALID(span_context->span)) {
		OT_LOCK_GUARD(span);

		rc = ot_tracer->Inject(ot_span_handle.at(span_context->span->idx)->context(), http_headers_carrier);
	}
	else if (OT_CTX_KEY_IS_VALID(span_context)) {
		OT_LOCK_GUARD(span_context);

		rc = ot_tracer->Inject(*(ot_span_context_handle.at(span_context->idx)), http_headers_carrier);
	}

	if (!rc)
		return otc_propagation_error_code_unknown;
	else if (text_map.empty())
		return otc_propagation_error_code_unknown;
	else if (otc_text_map_new(&(carrier->text_map), text_map.size()) == nullptr)
		return otc_propagation_error_code_unknown;

	for (auto const &it : text_map)
		if (carrier->set != nullptr) {
			otc_propagation_error_code_t retval = carrier->set(carrier, it.first.c_str(), it.second.c_str());
			if (retval != otc_propagation_error_code_success)
				return retval;
		}
		else if (otc_text_map_add(&(carrier->text_map), it.first.c_str(), 0, it.second.c_str(), 0, OT_CAST_STAT(otc_text_map_flags_t, OTC_TEXT_MAP_DUP_KEY | OTC_TEXT_MAP_DUP_VALUE)) == -1)
			return otc_propagation_error_code_unknown;

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_tracer_inject_binary -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_inject_binary(struct otc_tracer *tracer, struct otc_custom_carrier_writer *carrier, const struct otc_span_context *span_context)
{
	std::ostringstream          oss(std::ios::binary);
	opentracing::expected<void> rc;

	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (!OT_CTX_IS_VALID(span_context))
		return otc_propagation_error_code_span_context_corrupted;

	if (OT_SPAN_IS_VALID(span_context->span)) {
		OT_LOCK_GUARD(span);

		rc = ot_tracer->Inject(ot_span_handle.at(span_context->span->idx)->context(), oss);
	}
	else if (OT_CTX_KEY_IS_VALID(span_context)) {
		OT_LOCK_GUARD(span_context);

		rc = ot_tracer->Inject(*(ot_span_context_handle.at(span_context->idx)), oss);
	}

	if (rc)
		if (otc_binary_data_new(&(carrier->binary_data), oss.str().c_str(), oss.str().size()) != nullptr)
			return otc_propagation_error_code_success;

	return otc_propagation_error_code_unknown;
}


/***
 * NAME
 *   ot_tracer_inject_custom -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      - NOT USED
 *   span_context - NOT USED
 *
 * DESCRIPTION
 *   - NOT IMPLEMENTED
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_inject_custom(struct otc_tracer *tracer, struct otc_custom_carrier_writer *carrier, const struct otc_span_context *span_context)
{
	if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (!OT_CTX_IS_VALID(span_context))
		return otc_propagation_error_code_span_context_corrupted;

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_span_context_add -
 *
 * ARGUMENTS
 *   span_context       -
 *   span_context_maybe -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_span_context_add(struct otc_span_context **span_context, std::unique_ptr<opentracing::SpanContext> &span_context_maybe)
{
	OT_LOCK_GUARD(span_context);

	if ((*span_context = ot_span_context_new(nullptr)) == nullptr) {
		span_context_maybe.reset(nullptr);

		return otc_propagation_error_code_unknown;
	}

	ot_span_context_handle.emplace((*span_context)->idx, std::move(span_context_maybe));

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_tracer_text_map_add -
 *
 * ARGUMENTS
 *   arg   -
 *   key   -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_text_map_add(void *arg, const char *key, const char *value)
{
	TextMap *text_map = OT_CAST_REINTERPRET(TextMap *, arg);

	if ((arg == nullptr) || (key == nullptr) || (value == nullptr))
		return otc_propagation_error_code_unknown;

	text_map->emplace(key, value);

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_tracer_extract_text_map -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_extract_text_map(struct otc_tracer *tracer, const struct otc_text_map_reader *carrier, struct otc_span_context **span_context)
{
	TextMap        text_map;
	TextMapCarrier text_map_carrier(text_map);

	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (span_context == nullptr)
		return otc_propagation_error_code_invalid_span_context;

	if (carrier->foreach_key != nullptr) {
		otc_propagation_error_code_t rc = carrier->foreach_key(OT_CAST_CONST(struct otc_text_map_reader *, carrier), ot_tracer_text_map_add, &text_map);
		if (rc != otc_propagation_error_code_success)
			return rc;
	} else {
		for (size_t i = 0; i < carrier->text_map.count; i++)
			text_map[carrier->text_map.key[i]] = carrier->text_map.value[i];
	}

	auto span_context_maybe = ot_tracer->Extract(text_map_carrier);
	if (!span_context_maybe)
		return otc_propagation_error_code_span_context_not_found;

	return ot_span_context_add(span_context, *span_context_maybe);
}


/***
 * NAME
 *   ot_tracer_extract_http_headers -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_extract_http_headers(struct otc_tracer *tracer, const struct otc_http_headers_reader *carrier, struct otc_span_context **span_context)
{
	TextMap            text_map;
	HTTPHeadersCarrier http_headers_carrier(text_map);

	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (span_context == nullptr)
		return otc_propagation_error_code_invalid_span_context;

	if (carrier->foreach_key != nullptr) {
		otc_propagation_error_code_t rc = carrier->foreach_key(OT_CAST_CONST(struct otc_http_headers_reader *, carrier), ot_tracer_text_map_add, &text_map);
		if (rc != otc_propagation_error_code_success)
			return rc;
	} else {
		for (size_t i = 0; i < carrier->text_map.count; i++)
			text_map[carrier->text_map.key[i]] = carrier->text_map.value[i];
	}

	auto span_context_maybe = ot_tracer->Extract(http_headers_carrier);
	if (!span_context_maybe)
		return otc_propagation_error_code_span_context_not_found;

	return ot_span_context_add(span_context, *span_context_maybe);
}


/***
 * NAME
 *   ot_tracer_extract_binary -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      -
 *   span_context -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_extract_binary(struct otc_tracer *tracer, const struct otc_custom_carrier_reader *carrier, struct otc_span_context **span_context)
{
	if (ot_tracer == nullptr)
		return otc_propagation_error_code_invalid_tracer;
	else if ((tracer == nullptr) || (carrier == nullptr))
		return otc_propagation_error_code_invalid_carrier;
	else if (span_context == nullptr)
		return otc_propagation_error_code_invalid_span_context;

	if ((carrier->binary_data.data == nullptr) || (carrier->binary_data.size == 0))
		return otc_propagation_error_code_invalid_carrier;

	std::string        iss_data(OT_CAST_REINTERPRET(const char *, carrier->binary_data.data), carrier->binary_data.size);
	std::istringstream iss(iss_data, std::ios::binary);

	auto span_context_maybe = ot_tracer->Extract(iss);
	if (!span_context_maybe)
		return otc_propagation_error_code_span_context_not_found;

	return ot_span_context_add(span_context, *span_context_maybe);
}


/***
 * NAME
 *   ot_tracer_extract_custom -
 *
 * ARGUMENTS
 *   tracer       - NOT USED
 *   carrier      - NOT USED
 *   span_context - NOT USED
 *
 * DESCRIPTION
 *   - NOT IMPLEMENTED
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_tracer_extract_custom(struct otc_tracer *tracer, const struct otc_custom_carrier_reader *carrier, struct otc_span_context **span_context)
{
	if ((tracer == nullptr) || (carrier == nullptr) || (span_context == nullptr))
		return otc_propagation_error_code_unknown;

	return otc_propagation_error_code_success;
}


/***
 * NAME
 *   ot_tracer_destroy -
 *
 * ARGUMENTS
 *   tracer -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void ot_tracer_destroy(struct otc_tracer **tracer)
{
	if ((tracer == nullptr) || (*tracer == nullptr))
		return;

	OT_FREE_CLEAR(*tracer);
}


/***
 * NAME
 *   ot_tracer_new -
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
struct otc_tracer *ot_tracer_new(void)
{
	const static struct otc_tracer tracer_init = {
		.close                   = ot_tracer_close,                   /* lock not required */
		.start_span              = ot_tracer_start_span,              /* lock span */
		.start_span_with_options = ot_tracer_start_span_with_options, /* lock span */
		.inject_text_map         = ot_tracer_inject_text_map,         /* lock span and/or span_context */
		.inject_http_headers     = ot_tracer_inject_http_headers,     /* lock span and/or span_context */
		.inject_binary           = ot_tracer_inject_binary,           /* lock span and/or span_context */
		.inject_custom           = ot_tracer_inject_custom,           /* NOT IMPLEMENTED */
		.extract_text_map        = ot_tracer_extract_text_map,        /* lock span_context */
		.extract_http_headers    = ot_tracer_extract_http_headers,    /* lock span_context */
		.extract_binary          = ot_tracer_extract_binary,          /* lock span_context */
		.extract_custom          = ot_tracer_extract_custom,          /* NOT IMPLEMENTED */
		.destroy                 = ot_tracer_destroy                  /* lock not required */
	};
	struct otc_tracer *retptr;

	if ((retptr = OT_CAST_TYPEOF(retptr, OTC_DBG_CALLOC(1, sizeof(*retptr)))) != nullptr)
		(void)memcpy(retptr, &tracer_init, sizeof(*retptr));

	return retptr;
}


/***
 * NAME
 *   otc_tracer_load -
 *
 * ARGUMENTS
 *   library   -
 *   errbuf    -
 *   errbufsiz -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_tracer *otc_tracer_load(const char *library, char *errbuf, int errbufsiz)
{
	std::unique_ptr<opentracing::DynamicTracingLibraryHandle> handle {
		new opentracing::DynamicTracingLibraryHandle {}
	};
	std::shared_ptr<opentracing::Tracer>  tracer;
	struct otc_tracer                    *retptr = nullptr;

	if ((retptr = ot_tracer_new()) == nullptr) {
		/* Do nothing. */;
	}
	else if (ot_tracer_load(library, errbuf, errbufsiz, *handle) == -1) {
		retptr->destroy(&retptr);
	}
	else {
		ot_dynlib = std::move(handle);
	}

	return retptr;
}


/***
 * NAME
 *   otc_tracer_start -
 *
 * ARGUMENTS
 *   cfgfile   -
 *   cfgbuf    -
 *   errbuf    -
 *   errbufsiz -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int otc_tracer_start(const char *cfgfile, const char *cfgbuf, char *errbuf, int errbufsiz)
{
	std::shared_ptr<opentracing::Tracer>  tracer;
	char                                 *config = OT_CAST_CONST(char *, cfgbuf);
	int                                   retval = -1;

	if (cfgfile != nullptr) {
		config = otc_file_read(cfgfile, "#", errbuf, errbufsiz);
		if (config == nullptr)
			return retval;
	}

	if (ot_tracer_start(config, errbuf, errbufsiz, tracer) == -1) {
		/* Do nothing. */;
	} else {
		ot_tracer = std::move(tracer);

		(void)opentracing::Tracer::InitGlobal(ot_tracer);

		retval = 0;
	}

	if (config != cfgbuf)
		OT_FREE(config);

	return retval;
}


/***
 * NAME
 *   otc_tracer_init -
 *
 * ARGUMENTS
 *   library   -
 *   cfgfile   -
 *   cfgbuf    -
 *   errbuf    -
 *   errbufsiz -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_tracer *otc_tracer_init(const char *library, const char *cfgfile, const char *cfgbuf, char *errbuf, int errbufsiz)
{
	struct otc_tracer *retptr = nullptr;

	if ((retptr = otc_tracer_load(library, errbuf, errbufsiz)) != nullptr)
		if (otc_tracer_start(cfgfile, cfgbuf, errbuf, errbufsiz) == -1)
			retptr->destroy(&retptr);

	return retptr;
}


/***
 * NAME
 *   otc_tracer_global -
 *
 * ARGUMENTS
 *   tracer - NOT USED
 *
 * DESCRIPTION
 *   - NOT IMPLEMENTED
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_tracer_global(struct otc_tracer *tracer)
{
	if (tracer == nullptr)
		return;
}


/***
 * NAME
 *   otc_tracer_init_global -
 *
 * ARGUMENTS
 *   tracer - NOT USED
 *
 * DESCRIPTION
 *   - NOT IMPLEMENTED
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_tracer_init_global(struct otc_tracer *tracer)
{
	if (tracer == nullptr)
		return;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
