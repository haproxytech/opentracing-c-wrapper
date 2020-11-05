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
#ifndef OPENTRACING_C_WRAPPER_TRACER_H
#define OPENTRACING_C_WRAPPER_TRACER_H

__CPLUSPLUS_DECL_BEGIN

struct otc_tag {
	const char       *key;
	struct otc_value  value;
};

struct otc_start_span_options {
	struct otc_duration              start_time_steady;
	struct otc_timestamp             start_time_system;
	const struct otc_span_reference *references;
	int                              num_references;
	const struct otc_tag            *tags;
	int                              num_tags;
};

/***
 * tracer interface
 */
struct otc_tracer {
	void (*close)(struct otc_tracer *tracer)
		OTC_NONNULL_ALL;

	struct otc_span *(*start_span)(struct otc_tracer *tracer, const char *operation_name)
		OTC_NONNULL_ALL;

	struct otc_span *(*start_span_with_options)(struct otc_tracer *tracer, const char *operation_name, const struct otc_start_span_options *options)
		OTC_NONNULL(1, 2);

	otc_propagation_error_code_t (*inject_text_map)(struct otc_tracer *tracer, struct otc_text_map_writer *carrier, const struct otc_span_context *span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*inject_http_headers)(struct otc_tracer *tracer, struct otc_http_headers_writer *carrier, const struct otc_span_context *span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*inject_binary)(struct otc_tracer *tracer, struct otc_custom_carrier_writer *carrier, const struct otc_span_context *span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*inject_custom)(struct otc_tracer *tracer, struct otc_custom_carrier_writer *carrier, const struct otc_span_context *span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*extract_text_map)(struct otc_tracer *tracer, const struct otc_text_map_reader *carrier, struct otc_span_context **span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*extract_http_headers)(struct otc_tracer *tracer, const struct otc_http_headers_reader *carrier, struct otc_span_context **span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*extract_binary)(struct otc_tracer *tracer, const struct otc_custom_carrier_reader *carrier, struct otc_span_context **span_context)
		OTC_NONNULL_ALL;

	otc_propagation_error_code_t (*extract_custom)(struct otc_tracer *tracer, const struct otc_custom_carrier_reader *carrier, struct otc_span_context **span_context)
		OTC_NONNULL_ALL;

	void (*destroy)(struct otc_tracer **tracer)
		OTC_NONNULL_ALL;
};


struct otc_tracer *otc_tracer_init(const char *library, const char *cfgfile, const char *cfgbuf, char *errbuf, int errbufsiz);
void               otc_tracer_global(struct otc_tracer *tracer);
void               otc_tracer_init_global(struct otc_tracer *tracer);

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_TRACER_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
