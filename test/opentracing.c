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


static void ot_text_map_show(const struct otc_text_map *text_map)
{
	OT_FUNC("%p", text_map);

	if (_NULL(text_map))
		return;

	OT_DBG(OT, "%p:{ %p %p %zu/%zu %hhu }", text_map, text_map->key, text_map->value, text_map->count, text_map->size, text_map->is_dynamic);

	if (_nNULL(text_map->key) && _nNULL(text_map->value) && (text_map->count > 0)) {
		size_t i;

		for (i = 0; i < text_map->count; i++)
			OT_DBG(OT, "  \"%s\" -> \"%s\"", text_map->key[i], text_map->value[i]);
	}
}


/***
 * NAME
 *   ot_span_init -
 *
 * ARGUMENTS
 *   operation_name -
 *   ref_type       -
 *   ref_ctx_idx    -
 *   ref_span       -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span *ot_span_init(struct otc_tracer *tracer, const char *operation_name, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span)
{
	struct otc_start_span_options  options;
	struct otc_span_context        context = { .idx = ref_ctx_idx, .span = ref_span };
	struct otc_span_reference      references = { ref_type, &context };
	struct otc_span               *retptr = NULL;

	OT_FUNC("%p, \"%s\", %d, %d, %p", tracer, operation_name, ref_type, ref_ctx_idx, ref_span);

	(void)memset(&options, 0, sizeof(options));

	if (IN_RANGE(ref_type, otc_span_reference_child_of, otc_span_reference_follows_from)) {
		options.references     = &references;
		options.num_references = 1;
	}

	retptr = tracer->start_span_with_options(tracer, operation_name, &options);
	if (_NULL(retptr))
		OT_DBG(OT, "cannot init new span");
	else
		OT_DBG(OT, "span %p:%zu initialized", retptr, retptr->idx);

	return retptr;
}


/***
 * NAME
 *   ot_span_tag -
 *
 * ARGUMENTS
 *   span  -
 *   key   -
 *   type  -
 *   value -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_tag(struct otc_span *span, const char *key, int type, ...)
{
	va_list          ap;
	struct otc_value ot_value;
	int              retval = -1;

	OT_FUNC("%p, \"%s\", %d, ...", span, key, type);

	if (_NULL(span))
		return retval;

	va_start(ap, type);
	for (retval = 0; _nNULL(key) && IN_RANGE(type, otc_value_bool, otc_value_null); retval++) {
		ot_value.type = type;
		if (type == otc_value_bool)
			ot_value.value.bool_value = va_arg(ap, typeof(ot_value.value.bool_value));
		else if (type == otc_value_double)
			ot_value.value.double_value = va_arg(ap, typeof(ot_value.value.double_value));
		else if (type == otc_value_int64)
			ot_value.value.int64_value = va_arg(ap, typeof(ot_value.value.int64_value));
		else if (type == otc_value_uint64)
			ot_value.value.uint64_value = va_arg(ap, typeof(ot_value.value.uint64_value));
		else if (type == otc_value_string)
			ot_value.value.string_value = va_arg(ap, typeof(ot_value.value.string_value));
		else if (type == otc_value_null)
			ot_value.value.string_value = va_arg(ap, typeof(ot_value.value.string_value));
		span->set_tag(span, key, &ot_value);

		if (_nNULL(key = va_arg(ap, typeof(key))))
			type = va_arg(ap, typeof(type));
	}
	va_end(ap);

	return retval;
}


/***
 * NAME
 *   ot_span_set_baggage -
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
 *   -
 */
int ot_span_set_baggage(struct otc_span *span, const char *key, const char *value, ...)
{
	va_list ap;
	int     retval = -1;

	OT_FUNC("%p, \"%s\", \"%s\", ...", span, key, value);

	if (_NULL(span))
		return retval;

	va_start(ap, value);
	for (retval = 0; _nNULL(key); retval++) {
		OT_DBG(OT, "set baggage: \"%s\" \"%s\"", key, value);

		span->set_baggage_item(span, key, value);

		if (_nNULL(key = va_arg(ap, typeof(key))))
			value = va_arg(ap, typeof(value));
	}
	va_end(ap);

	return retval;
}


/***
 * NAME
 *   ot_span_baggage -
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
struct otc_text_map *ot_span_baggage(const struct otc_span *span, const char *key, ...)
{
	va_list              ap;
	struct otc_text_map *retptr = NULL;
	int                  i, n;

	OT_FUNC("%p, \"%s\", ...", span, key);

	if (_NULL(span) || _NULL(key))
		return retptr;

	va_start(ap, key);
	for (n = 1; _nNULL(va_arg(ap, typeof(key))); n++);
	va_end(ap);

	if (_NULL(retptr = otc_text_map_new(NULL, n)))
		return retptr;

	va_start(ap, key);
	for (i = 0; (i < n) && _nNULL(key); i++) {
		char *value;

		if (_nNULL(value = (char *)span->baggage_item(span, key))) {
			(void)otc_text_map_add(retptr, key, 0, value, 0, OTC_TEXT_MAP_DUP_KEY);

			OT_DBG(OT, "get baggage[%d]: \"%s\" -> \"%s\"", i, retptr->key[i], retptr->value[i]);
		} else {
			OT_DBG(OT, "get baggage[%d]: \"%s\" -> invalid key", i, key);
		}

		key = va_arg(ap, typeof(key));
	}
	va_end(ap);

	return retptr;
}


/***
 * NAME
 *   ot_span_log_kv -
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
 *   -
 */
int ot_span_log_kv(struct otc_span *span, const char *key, const char *value, ...)
{
	va_list              ap;
	struct otc_log_field log_data[OTC_MAXLOGFIELDS];
	int                  retval = -1;

	OT_FUNC("%p, \"%s\", \"%s\", ...", span, key, value);

	if (_NULL(span) || _NULL(key) || _NULL(value))
		return retval;

	va_start(ap, value);
	for (retval = 0; (retval < TABLESIZE(log_data)) && _nNULL(key); retval++) {
		log_data[retval].key                      = key;
		log_data[retval].value.type               = otc_value_string;
		log_data[retval].value.value.string_value = value;

		if (_nNULL(key = va_arg(ap, typeof(key))))
			value = va_arg(ap, typeof(value));
	}
	va_end(ap);

	span->log_fields(span, log_data, retval);

	return retval;
}


/***
 * NAME
 *   ot_span_log -
 *
 * ARGUMENTS
 *   span   -
 *   key    -
 *   format -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int ot_span_log(struct otc_span *span, const char *key, const char *format, ...)
{
	va_list ap;
	char    value[BUFSIZ];

	OT_FUNC("%p, \"%s\", \"%s\", ...", span, key, format);

	if (_NULL(span) || _NULL(key) || _NULL(format))
		return -1;

	va_start(ap, format);
	(void)vsnprintf(value, sizeof(value), format, ap);
	va_end(ap);

	return ot_span_log_kv(span, key, value, NULL);
}


#ifdef OT_USE_INJECT_CB

/***
 * NAME
 *   ot_text_map_writer_set_cb -
 *
 * ARGUMENTS
 *   writer -
 *   key    -
 *   value  -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_text_map_writer_set_cb(struct otc_text_map_writer *writer, const char *key, const char *value)
{
	otc_propagation_error_code_t retval = otc_propagation_error_code_success;

	OT_FUNC("%p, \"%s\", \"%s\"", writer, key, value);

	if (otc_text_map_add(&(writer->text_map), key, 0, value, 0, OTC_TEXT_MAP_DUP_KEY | OTC_TEXT_MAP_DUP_VALUE) == -1)
		retval = otc_propagation_error_code_unknown;

	return retval;
}

#endif /* OT_USE_INJECT_CB */


#ifdef OT_USE_EXTRACT_CB

/***
 * NAME
 *   ot_text_map_reader_foreach_key_cb -
 *
 * ARGUMENTS
 *   reader  -
 *   handler -
 *   arg     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_text_map_reader_foreach_key_cb(struct otc_text_map_reader *reader, otc_propagation_error_code_t (*handler)(void *arg, const char *key, const char *value), void *arg)
{
	size_t                       i;
	otc_propagation_error_code_t retval = otc_propagation_error_code_success;

	OT_FUNC("%p, %p, %p", reader, handler, arg);

	for (i = 0; (retval == otc_propagation_error_code_success) && (i < reader->text_map.count); i++) {
		OT_DBG(OT, "\"%s\" -> \"%s\"", reader->text_map.key[i], reader->text_map.value[i]);

		retval = handler(arg, reader->text_map.key[i], reader->text_map.value[i]);
	}

	return retval;
}

#endif /* OT_USE_EXTRACT_CB */


/***
 * NAME
 *   ot_inject_text_map -
 *
 * ARGUMENTS
 *   tracer  -
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_text_map(struct otc_tracer *tracer, const struct otc_span *span, struct otc_text_map_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, span, carrier);

	if (_NULL(span))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));
#ifdef OT_USE_INJECT_CB
	carrier->set = ot_text_map_writer_set_cb;
#endif

	rc = tracer->inject_text_map(tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: inject_text_map() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	} else {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	OT_DBG(OT, "carrier %p: { { %p %p %zu/%zu %hhu } %p }", carrier, carrier->text_map.key, carrier->text_map.value, carrier->text_map.count, carrier->text_map.size, carrier->text_map.is_dynamic, carrier->set);
	ot_text_map_show(&(carrier->text_map));

	return retptr;
}


/***
 * NAME
 *   ot_extract_text_map -
 *
 * ARGUMENTS
 *   tracer   -
 *   carrier  -
 *   text_map -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_text_map(struct otc_tracer *tracer, struct otc_text_map_reader *carrier, const struct otc_text_map *text_map)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, carrier, text_map);

	(void)memset(carrier, 0, sizeof(*carrier));
#ifdef OT_USE_EXTRACT_CB
	carrier->foreach_key = ot_text_map_reader_foreach_key_cb;
#endif

	if (_nNULL(text_map))
		(void)memcpy(&(carrier->text_map), text_map, sizeof(carrier->text_map));

	OT_DBG(OT, "carrier %p: { { %p %p %zu/%zu %hhu } }", carrier, carrier->text_map.key, carrier->text_map.value, carrier->text_map.count, carrier->text_map.size, carrier->text_map.is_dynamic);

	rc = tracer->extract_text_map(tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: extract_text_map() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	}
	else if (_nNULL(retptr)) {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	return retptr;
}


#ifdef OT_USE_INJECT_CB

/***
 * NAME
 *   ot_http_headers_writer_set_cb -
 *
 * ARGUMENTS
 *   writer -
 *   key    -
 *   value  -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_http_headers_writer_set_cb(struct otc_http_headers_writer *writer, const char *key, const char *value)
{
	otc_propagation_error_code_t retval = otc_propagation_error_code_success;

	OT_FUNC("%p, \"%s\", \"%s\"", writer, key, value);

	if (otc_text_map_add(&(writer->text_map), key, 0, value, 0, OTC_TEXT_MAP_DUP_KEY | OTC_TEXT_MAP_DUP_VALUE) == -1)
		retval = otc_propagation_error_code_unknown;

	return retval;
}

#endif /* OT_USE_INJECT_CB */


#ifdef OT_USE_EXTRACT_CB

/***
 * NAME
 *   ot_http_headers_reader_foreach_key_cb -
 *
 * ARGUMENTS
 *   reader  -
 *   handler -
 *   arg     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static otc_propagation_error_code_t ot_http_headers_reader_foreach_key_cb(struct otc_http_headers_reader *reader, otc_propagation_error_code_t (*handler)(void *arg, const char *key, const char *value), void *arg)
{
	size_t                       i;
	otc_propagation_error_code_t retval = otc_propagation_error_code_success;

	OT_FUNC("%p, %p, %p", reader, handler, arg);

	for (i = 0; (retval == otc_propagation_error_code_success) && (i < reader->text_map.count); i++) {
		OT_DBG(OT, "\"%s\" -> \"%s\"", reader->text_map.key[i], reader->text_map.value[i]);

		retval = handler(arg, reader->text_map.key[i], reader->text_map.value[i]);
	}

	return retval;
}

#endif /* OT_USE_EXTRACT_CB */


/***
 * NAME
 *   ot_inject_http_headers -
 *
 * ARGUMENTS
 *   tracer  -
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_http_headers(struct otc_tracer *tracer, const struct otc_span *span, struct otc_http_headers_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, span, carrier);

	if (_NULL(span))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));
#ifdef OT_USE_INJECT_CB
	carrier->set = ot_http_headers_writer_set_cb;
#endif

	rc = tracer->inject_http_headers(tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: inject_http_headers() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	} else {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	OT_DBG(OT, "carrier %p: { { %p %p %zu/%zu %hhu } %p }", carrier, carrier->text_map.key, carrier->text_map.value, carrier->text_map.count, carrier->text_map.size, carrier->text_map.is_dynamic, carrier->set);
	ot_text_map_show(&(carrier->text_map));

	return retptr;
}


/***
 * NAME
 *   ot_extract_http_headers -
 *
 * ARGUMENTS
 *   tracer  -
 *   carrier  -
 *   text_map -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_http_headers(struct otc_tracer *tracer, struct otc_http_headers_reader *carrier, const struct otc_text_map *text_map)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, carrier, text_map);

	(void)memset(carrier, 0, sizeof(*carrier));
#ifdef OT_USE_EXTRACT_CB
	carrier->foreach_key = ot_http_headers_reader_foreach_key_cb;
#endif

	if (_nNULL(text_map))
		(void)memcpy(&(carrier->text_map), text_map, sizeof(carrier->text_map));

	OT_DBG(OT, "carrier %p: { { %p %p %zu/%zu %hhu } }", carrier, carrier->text_map.key, carrier->text_map.value, carrier->text_map.count, carrier->text_map.size, carrier->text_map.is_dynamic);

	rc = tracer->extract_http_headers(tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: extract_http_headers() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	}
	else if (_nNULL(retptr)) {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	return retptr;
}


/***
 * NAME
 *   ot_inject_binary -
 *
 * ARGUMENTS
 *   tracer  -
 *   span    -
 *   carrier -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_inject_binary(struct otc_tracer *tracer, const struct otc_span *span, struct otc_custom_carrier_writer *carrier)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, span, carrier);

	if (_NULL(span))
		return retptr;

	if (_NULL(retptr = span->span_context((struct otc_span *)span)))
		return retptr;

	(void)memset(carrier, 0, sizeof(*carrier));

	rc = tracer->inject_binary(tracer, carrier, retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: inject_binary() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	} else {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	OT_DBG(OT, "carrier %p: { { %p %zu %hhu } %p }", carrier, carrier->binary_data.data, carrier->binary_data.size, carrier->binary_data.is_dynamic, carrier->inject);

#ifdef DEBUG
	if (carrier->binary_data.data != NULL) {
		struct otc_jaeger_trace_context *ctx_jaeger = carrier->binary_data.data;
		struct otc_dd_trace_context     *ctx_dd = carrier->binary_data.data;

		OT_DBG(OT, "Jaeger trace context: %016" PRIx64 "%016" PRIx64 ":%016" PRIx64 ":%016" PRIx64 ":%02hhx <%s> <%s>",
		       ctx_jaeger->trace_id[0], ctx_jaeger->trace_id[1], ctx_jaeger->span_id, ctx_jaeger->parent_span_id, ctx_jaeger->flags,
		       str_hex(ctx_jaeger->baggage, carrier->binary_data.size - sizeof(*ctx_jaeger)),
		       str_ctrl(ctx_jaeger->baggage, carrier->binary_data.size - sizeof(*ctx_jaeger)));
		OT_DBG(OT, "DataDog trace context: <%s> <%s>",
		       str_hex(ctx_dd->data, carrier->binary_data.size),
		       str_ctrl(ctx_dd->data, carrier->binary_data.size));
	}
#endif /* DEBUG */

	return retptr;
}


/***
 * NAME
 *   ot_extract_binary -
 *
 * ARGUMENTS
 *   tracer      -
 *   carrier     -
 *   binary_data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_span_context *ot_extract_binary(struct otc_tracer *tracer, struct otc_custom_carrier_reader *carrier, const struct otc_binary_data *binary_data)
{
	struct otc_span_context *retptr = NULL;
	int                      rc;

	OT_FUNC("%p, %p, %p", tracer, carrier, binary_data);

	(void)memset(carrier, 0, sizeof(*carrier));

	if (_nNULL(binary_data) && _nNULL(binary_data->data) && (binary_data->size > 0))
		(void)memcpy(&(carrier->binary_data), binary_data, sizeof(carrier->binary_data));

	OT_DBG(OT, "carrier %p: { { %p %zu %hhu } %p }", carrier, carrier->binary_data.data, carrier->binary_data.size, carrier->binary_data.is_dynamic, carrier->extract);

	rc = tracer->extract_binary(tracer, carrier, &retptr);
	if (rc != otc_propagation_error_code_success) {
		OT_LOG("  ERROR: extract_binary() failed: %d", rc);

		OTC_DBG_FREE(retptr);
	}
	else if (_nNULL(retptr)) {
		OT_DBG(OT, "context %p: { %" PRId64 " %p %p }", retptr, retptr->idx, retptr->span, retptr->destroy);
	}

	return retptr;
}


/***
 * NAME
 *   ot_span_finish -
 *
 * ARGUMENTS
 *   span      -
 *   ts_finish -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void ot_span_finish(struct otc_span **span, const struct timespec *ts_finish)
{
	struct otc_finish_span_options options;

	OT_FUNC("%p, %p", span, ts_finish);

	if (_NULL(span) || _NULL(*span))
		return;

	(void)memset(&options, 0, sizeof(options));

	if (_nNULL(ts_finish))
		(void)memcpy(&(options.finish_time.value), ts_finish, sizeof(options.finish_time.value));

	OT_DBG(OT, "span %p:%zu finished", *span, (*span)->idx);

	(*span)->finish_with_options(*span, &options);

	*span = NULL;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
