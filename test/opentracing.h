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
#ifndef TEST_OPENTRACING_H
#define TEST_OPENTRACING_H

struct otc_span         *ot_span_init(struct otc_tracer *tracer, const char *operation_name, int ref_type, int ref_ctx_idx, const struct otc_span *ref_span);
int                      ot_span_tag(struct otc_span *span, const char *key, int type, ...);
int                      ot_span_set_baggage(struct otc_span *span, const char *key, const char *value, ...);
struct otc_text_map     *ot_span_baggage(const struct otc_span *span, const char *key, ...);
int                      ot_span_log_kv(struct otc_span *span, const char *key, const char *value, ...);
int                      ot_span_log(struct otc_span *span, const char *key, const char *format, ...);
struct otc_span_context *ot_inject_text_map(struct otc_tracer *tracer, const struct otc_span *span, struct otc_text_map_writer *carrier);
struct otc_span_context *ot_extract_text_map(struct otc_tracer *tracer, struct otc_text_map_reader *carrier, const struct otc_text_map *text_map);
struct otc_span_context *ot_inject_http_headers(struct otc_tracer *tracer, const struct otc_span *span, struct otc_http_headers_writer *carrier);
struct otc_span_context *ot_extract_http_headers(struct otc_tracer *tracer, struct otc_http_headers_reader *carrier, const struct otc_text_map *text_map);
struct otc_span_context *ot_inject_binary(struct otc_tracer *tracer, const struct otc_span *span, struct otc_custom_carrier_writer *carrier);
struct otc_span_context *ot_extract_binary(struct otc_tracer *tracer, struct otc_custom_carrier_reader *carrier, const struct otc_binary_data *binary_data);
void                     ot_span_finish(struct otc_span **span, const struct timespec *ts_finish);

#endif /* TEST_OPENTRACING_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
