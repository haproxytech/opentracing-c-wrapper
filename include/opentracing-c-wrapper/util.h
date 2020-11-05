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
#ifndef OPENTRACING_C_WRAPPER_UTIL_H
#define OPENTRACING_C_WRAPPER_UTIL_H

__CPLUSPLUS_DECL_BEGIN

typedef enum {
	OTC_TEXT_MAP_DUP_KEY    = 0x01, /* Duplicate the key data. */
	OTC_TEXT_MAP_DUP_VALUE  = 0x02, /* Duplicate the value data. */
	OTC_TEXT_MAP_FREE_KEY   = 0x04, /* Release the key data. */
	OTC_TEXT_MAP_FREE_VALUE = 0x08, /* Release the value data. */
} otc_text_map_flags_t;

struct otc_text_map {
	char   **key;
	char   **value;
	size_t   count;
	size_t   size;
	bool     is_dynamic;
};

struct otc_binary_data {
	void   *data;
	size_t  size;
	bool    is_dynamic;
};

/*
 * Jaeger Trace/Span Identity
 *   uber-trace-id:               {trace-id}:{span-id}:{parent-span-id}:{flags}
 *   uberctx-{baggage-key}:       {value}
 *
 * Zipkin
 *   x-b3-traceid:                {trace-id}
 *   x-b3-spanid:                 {span-id}
 *   x-b3-parentspanid:           {parent-span-id}
 *   x-b3-flags:                  {flags}
 *   x-b3-sampled:                {value}
 *   ot-baggage-{baggage-key}:    {value}
 *
 * DataDog
 *   x-datadog-trace-id:          {trace-id}
 *   x-datadog-parent-id:         {parent-id}
 *   x-datadog-sampling-priority: {value}
 *   ot-baggage-{baggage-key}:    {value}
 */
struct otc_jaeger_trace_context {
	uint64_t trace_id[2];    /* 128-bit random number, value of 0 is not valid. */
	uint64_t span_id;        /* 64-bit random number, value of 0 is not valid. */
	uint64_t parent_span_id; /* 64-bit parent span id, 0 value is valid and means 'root span' */
	uint8_t  flags;          /* 8-bit bitmap, only bits 0 and 1 are used. */
	uint8_t  baggage[0];
} __attribute__((packed));

struct otc_zipkin_trace_context {
	uint8_t data[0];
} __attribute__((packed));

struct otc_dd_trace_context {
	uint8_t data[0];
} __attribute__((packed));


#ifdef OTC_DBG_MEM
typedef void *(*otc_ext_malloc_t)(const char *, int, size_t);
typedef void  (*otc_ext_free_t)(const char *, int, void *);
#else
typedef void *(*otc_ext_malloc_t)(size_t);
typedef void  (*otc_ext_free_t)(void *);
#endif


void                    otc_ext_init(otc_ext_malloc_t func_malloc, otc_ext_free_t func_free);

struct otc_text_map    *otc_text_map_new(struct otc_text_map *text_map, size_t size);
int                     otc_text_map_add(struct otc_text_map *text_map, const char *key, size_t key_len, const char *value, size_t value_len, otc_text_map_flags_t flags);
void                    otc_text_map_destroy(struct otc_text_map **text_map, otc_text_map_flags_t flags);

struct otc_binary_data *otc_binary_data_new(struct otc_binary_data *binary_data, const void *data, size_t size);
void                    otc_binary_data_destroy(struct otc_binary_data **binary_data);

char                   *otc_file_read(const char *filename, const char *comment, char *errbuf, int errbufsiz);

void                    otc_statistics(char *buffer, size_t bufsiz);

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_UTIL_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
