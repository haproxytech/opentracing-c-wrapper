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
#ifndef _OPENTRACING_C_WRAPPER_DBG_MALLOC_H_
#define _OPENTRACING_C_WRAPPER_DBG_MALLOC_H_

#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#endif


#define DBG_MEM(l,s,f, ...)                                        \
	do {                                                       \
		if (dbg_mem == nullptr)                            \
			/* Do nothing. */;                         \
		else if (!(l) || (dbg_mem->level & (1 << (l))))    \
			(void)fprintf((s), f "\n", ##__VA_ARGS__); \
	} while (0)
#define DBG_MEM_ERR(f, ...)      DBG_MEM(0, stderr, "MEM_ERROR: " f, ##__VA_ARGS__)
#define DBG_MEM_INFO(l,f, ...)   DBG_MEM((l), stdout, f, ##__VA_ARGS__)

/* 8 bytes - dBgM (dBgM ^ 0xffffffff) */
#define DBG_MEM_MAGIC            UINT64_C(0x6442674d9bbd98b2)
#define DBG_MEM_SIZE(n)          ((n) + sizeof(struct otc_dbg_mem_metadata))
#define DBG_MEM_PTR(p)           DBG_MEM_SIZE(OT_CAST_TYPEOF(uint8_t *, (p)))
#define DBG_MEM_DATA(p)          OT_CAST_TYPEOF(struct otc_dbg_mem_metadata *, OT_CAST_TYPEOF(uint8_t *, (p)) - DBG_MEM_SIZE(0))
#define DBG_MEM_RETURN(p)        (((p) == nullptr) ? nullptr : DBG_MEM_PTR(p))

struct otc_dbg_mem_metadata {
	struct otc_dbg_mem_data *data;
	uint64_t                 magic;
};

#ifdef __sun
#define PRI_MI                   "lu"
#else
#define PRI_MI                   "d"
#endif

#endif /* _OPENTRACING_C_WRAPPER_DBG_MALLOC_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
