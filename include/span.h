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
#ifndef _OPENTRACING_C_WRAPPER_SPAN_H_
#define _OPENTRACING_C_WRAPPER_SPAN_H_

#define OT_LF(a)   { fields[a].key, str_value[a] }


class otc_hash {
	public:
	size_t operator() (int64_t key) const { return key; }
};


class otc_equal_to {
	public:
	bool operator() (int64_t a, int64_t b) const { return a == b; }
};


#  ifdef OT_THREADS_NO_LOCKING
template<typename T>
	using Handle = std::unordered_map<
		int64_t,
		std::unique_ptr<T>,
		otc_hash,
		otc_equal_to
	>;
struct HandleData {
	int64_t key;
	int64_t alloc_fail_cnt;
	int64_t erase_cnt;
	int64_t destroy_cnt;
};

#     define OT_LOCK_GUARD(a,...)
#     define OT_LOCK(a,b)

extern thread_local Handle<opentracing::Span>        ot_span_handle;
extern thread_local Handle<opentracing::SpanContext> ot_span_context_handle;
extern struct HandleData                             ot_span;
extern struct HandleData                             ot_span_context;
#  else
template<typename T> struct Handle {
	std::unordered_map<
		int64_t,
		std::unique_ptr<T>,
		otc_hash,
		otc_equal_to
	>          handle;
	int64_t    key;
	int64_t    alloc_fail_cnt;
	int64_t    erase_cnt;
	int64_t    destroy_cnt;
	std::mutex mutex;
};

#     define ot_span_handle           ot_span.handle
#     define ot_span_context_handle   ot_span_context.handle
#     define OT_LOCK_GUARD(a,...)     const std::lock_guard<std::mutex> guard_##a(ot_##a.mutex, ##__VA_ARGS__)
#     define OT_LOCK(a,b)             std::lock(ot_##a.mutex, ot_##b.mutex); OT_LOCK_GUARD(a, std::adopt_lock); OT_LOCK_GUARD(b, std::adopt_lock);

extern struct Handle<opentracing::Span>        ot_span;
extern struct Handle<opentracing::SpanContext> ot_span_context;
#  endif /* OT_THREADS_NO_LOCKING */


struct otc_span         *ot_span_new(void);
void                             ot_nolock_span_destroy(struct otc_span **span);
struct otc_span_context *ot_span_context_new(const struct otc_span *span);

#endif /* _OPENTRACING_C_WRAPPER_SPAN_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
