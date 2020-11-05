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
#ifndef OPENTRACING_C_WRAPPER_DBG_MALLOC_H
#define OPENTRACING_C_WRAPPER_DBG_MALLOC_H

__CPLUSPLUS_DECL_BEGIN

#ifdef OTC_DBG_MEM

#define OTC_DBG_MALLOC(s)      otc_dbg_malloc(__func__, __LINE__, (s))
#define OTC_DBG_CALLOC(n,e)    otc_dbg_calloc(__func__, __LINE__, (n), (e))
#define OTC_DBG_REALLOC(p,s)   otc_dbg_realloc(__func__, __LINE__, (p), (s))
#define OTC_DBG_FREE(p)        otc_dbg_free(__func__, __LINE__, (p))
#define OTC_DBG_STRDUP(s)      otc_dbg_strdup(__func__, __LINE__, (s))
#define OTC_DBG_STRNDUP(s,n)   otc_dbg_strndup(__func__, __LINE__, (s), (n))
#define OTC_DBG_MEMDUP(s,n)    otc_dbg_memdup(__func__, __LINE__, (s), (n))
#define OTC_DBG_MEMINFO()      otc_dbg_mem_info()


struct otc_dbg_mem_data {
	const void *ptr;
	size_t      size;
	char        func[63];
	bool        used;
} __attribute__((packed));

struct otc_dbg_mem {
	struct otc_dbg_mem_data *data;
	size_t                   count;
	size_t                   unused;
	size_t                   reused;
	uint64_t                 size;
	uint64_t                 op_cnt[4];
	uint8_t                  level;
	pthread_mutex_t          mutex;
};


void *otc_dbg_malloc(const char *func, int line, size_t size);
void *otc_dbg_calloc(const char *func, int line, size_t nelem, size_t elsize);
void *otc_dbg_realloc(const char *func, int line, void *ptr, size_t size);
void  otc_dbg_free(const char *func, int line, void *ptr);
char *otc_dbg_strdup(const char *func, int line, const char *s);
char *otc_dbg_strndup(const char *func, int line, const char *s, size_t size);
void *otc_dbg_memdup(const char *func, int line, const void *s, size_t size);
int   otc_dbg_mem_init(struct otc_dbg_mem *mem, struct otc_dbg_mem_data *data, size_t count, uint8_t level);
void  otc_dbg_mem_disable(void);
void  otc_dbg_mem_info(void);

#else

#define OTC_DBG_MALLOC(s)      malloc(s)
#define OTC_DBG_CALLOC(n,e)    calloc((n), (e))
#define OTC_DBG_REALLOC(p,s)   realloc((p), (s))
#define OTC_DBG_FREE(p)        free(p)
#define OTC_DBG_STRDUP(s)      strdup(s)
#define OTC_DBG_STRNDUP(s,n)   strndup((s), (n))
#define OTC_DBG_MEMDUP(s,n)    mem_dup((s), (n))
#define OTC_DBG_MEMINFO()      while (0)

#endif /* OTC_DBG_MEM */

__CPLUSPLUS_DECL_END
#endif /* OPENTRACING_C_WRAPPER_DBG_MALLOC_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
