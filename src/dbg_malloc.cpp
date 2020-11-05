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


static struct otc_dbg_mem *dbg_mem = nullptr;


/***
 * NAME
 *   otc_dbg_set_metadata -
 *
 * ARGUMENTS
 *   ptr  - the real address of the allocated data
 *   data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void otc_dbg_set_metadata(void *ptr, struct otc_dbg_mem_data *data)
{
	struct otc_dbg_mem_metadata *metadata;

	if (ptr == nullptr)
		return;

	metadata        = OT_CAST_TYPEOF(metadata, ptr);
	metadata->data  = (data == nullptr) ? OT_CAST_TYPEOF(data, metadata) : data;
	metadata->magic = DBG_MEM_MAGIC;
}


/***
 * NAME
 *   otc_dbg_mem_add -
 *
 * ARGUMENTS
 *   func   -
 *   line   -
 *   ptr    - the real address of the allocated data
 *   size   -
 *   data   -
 *   op_idx -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void otc_dbg_mem_add(const char *func, int line, void *ptr, size_t size, struct otc_dbg_mem_data *data, int op_idx)
{
	(void)snprintf(data->func, sizeof(data->func), "%s:%d", func, line);

	data->ptr  = ptr;
	data->size = size;
	data->used = 1;

	dbg_mem->size += size;
	dbg_mem->op_cnt[op_idx]++;

	otc_dbg_set_metadata(ptr, data);
}


/***
 * NAME
 *   otc_dbg_mem_alloc -
 *
 * ARGUMENTS
 *   func    -
 *   line    -
 *   old_ptr - the address of the data returned to the program
 *   ptr     - the real address of the allocated data
 *   size    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void otc_dbg_mem_alloc(const char *func, int line, void *old_ptr, void *ptr, size_t size)
{
	size_t i = 0;
	int    rc;

	otc_dbg_set_metadata(ptr, nullptr);

	if (dbg_mem == nullptr)
		return;

	if ((rc = pthread_mutex_lock(&(dbg_mem->mutex))) != 0) {
		DBG_MEM_ERR("cannot lock mutex: %s", otc_strerror(rc));

		return;
	}

	if (old_ptr != nullptr) {
		/* Reallocating memory. */
		struct otc_dbg_mem_metadata *metadata = DBG_MEM_DATA(old_ptr);

		if (metadata == nullptr) {
			DBG_MEM_ERR("no metadata: MEM_REALLOC %s:%d(%p -> %p %zu)", func, line, old_ptr, DBG_MEM_PTR(ptr), size);
		}
		else if (metadata->data == nullptr) {
			DBG_MEM_ERR("invalid metadata: MEM_REALLOC %s:%d(%p -> %p %zu)", func, line, old_ptr, DBG_MEM_PTR(ptr), size);
		}
		else if (metadata->data == OT_CAST_TYPEOF(metadata->data, metadata)) {
			DBG_MEM_ERR("unset metadata: MEM_REALLOC %s:%d(%p -> %p %zu)", func, line, old_ptr, DBG_MEM_PTR(ptr), size);
		}
		else if (metadata->magic != DBG_MEM_MAGIC) {
			DBG_MEM_ERR("invalid magic: MEM_REALLOC %s:%d(%p -> %p %zu) 0x%016" PRIu64, func, line, old_ptr, DBG_MEM_PTR(ptr), size, metadata->magic);
		}
		else if (metadata->data->used && (metadata->data->ptr == metadata)) {
			DBG_MEM_INFO(1, "MEM_REALLOC: %s:%d(%p %zu -> %p %zu)", func, line, old_ptr, metadata->data->size, DBG_MEM_PTR(ptr), size);

			dbg_mem->size -= metadata->data->size;
			otc_dbg_mem_add(func, line, ptr, size, metadata->data, 1);
		}
	} else {
		/*
		 * The first attempt is to find a location that has not been
		 * used at all so far.  If such is not found, an attempt is
		 * made to find the first available location.
		 */
		if (dbg_mem->unused < dbg_mem->count) {
			i = dbg_mem->unused++;
		} else {
			do {
				if (dbg_mem->reused >= dbg_mem->count)
					dbg_mem->reused = 0;

				if (!dbg_mem->data[dbg_mem->reused].used) {
					i = dbg_mem->reused++;

					break;
				}

				dbg_mem->reused++;
			} while (++i <= dbg_mem->count);
		}

		if (i < dbg_mem->count) {
			DBG_MEM_INFO(1, "MEM_ALLOC: %s:%d(%p %zu %zu)", func, line, DBG_MEM_PTR(ptr), size, i);

			otc_dbg_mem_add(func, line, ptr, size, dbg_mem->data + i, 0);
		}
	}

	if ((rc = pthread_mutex_unlock(&(dbg_mem->mutex))) != 0) {
		DBG_MEM_ERR("cannot unlock mutex: %s", otc_strerror(rc));

		return;
	}

	if (i >= dbg_mem->count)
		DBG_MEM_ERR("alloc overflow: %s:%d(%p -> %p %zu)", func, line, old_ptr, DBG_MEM_PTR(ptr), size);
}


/***
 * NAME
 *   otc_dbg_mem_release -
 *
 * ARGUMENTS
 *   func   -
 *   line   -
 *   ptr    - the address of the data returned to the program
 *   op_idx -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void otc_dbg_mem_release(const char *func, int line, void *ptr, int op_idx)
{
	struct otc_dbg_mem_metadata *metadata;
	bool                         flag_invalid = 0;
	size_t                       i;
	int                          rc;

	if (dbg_mem == nullptr) {
		return;
	}
	else if (ptr == nullptr) {
		DBG_MEM_ERR("invalid memory address: %p", ptr);

		return;
	}

	if ((rc = pthread_mutex_lock(&(dbg_mem->mutex))) != 0) {
		DBG_MEM_ERR("cannot lock mutex: %s", otc_strerror(rc));

		return;
	}

	metadata = DBG_MEM_DATA(ptr);
	if (metadata == nullptr) {
		DBG_MEM_ERR("no metadata: MEM_%s %s:%d(%p)", (op_idx == 2) ? "FREE" : "RELEASE", func, line, ptr);
	}
	else if (metadata->data == nullptr) {
		DBG_MEM_ERR("invalid metadata: MEM_%s %s:%d(%p)", (op_idx == 2) ? "FREE" : "RELEASE", func, line, ptr);
	}
	else if (metadata->data == OT_CAST_TYPEOF(metadata->data, metadata)) {
		DBG_MEM_ERR("unset metadata: MEM_%s %s:%d(%p)", (op_idx == 2) ? "FREE" : "RELEASE", func, line, ptr);
	}
	else if (metadata->magic != DBG_MEM_MAGIC) {
		DBG_MEM_ERR("invalid magic: MEM_%s %s:%d(%p) 0x%016" PRIu64, (op_idx == 2) ? "FREE" : "RELEASE", func, line, ptr, metadata->magic);
	}
	else if (metadata->data->used && (metadata->data->ptr == metadata)) {
		DBG_MEM_INFO(1, "MEM_%s: %s:%d(%p %zu)", (op_idx == 2) ? "FREE" : "RELEASE", func, line, ptr, metadata->data->size);

		metadata->data->used = 0;

		dbg_mem->size -= metadata->data->size;
		dbg_mem->op_cnt[op_idx]++;
	}
	else {
		flag_invalid = 1;
	}

	if ((rc = pthread_mutex_unlock(&(dbg_mem->mutex))) != 0) {
		DBG_MEM_ERR("cannot unlock mutex: %s", otc_strerror(rc));

		return;
	}

	if (flag_invalid) {
		DBG_MEM_ERR("invalid ptr: %s:%d(%p)", func, line, ptr);

		if (metadata != nullptr)
			for (i = 0; i < dbg_mem->count; i++)
				if (dbg_mem->data[i].ptr == metadata)
					DBG_MEM_ERR("possible previous use: %s %hhu", dbg_mem->data[i].func, dbg_mem->data[i].used);
	}
}


/***
 * NAME
 *   otc_dbg_malloc -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
void *otc_dbg_malloc(const char *func, int line, size_t size)
{
	void *retptr;

	retptr = malloc(DBG_MEM_SIZE(size));

	otc_dbg_mem_alloc(func, line, nullptr, retptr, size);

	return DBG_MEM_RETURN(retptr);
}


/***
 * NAME
 *   otc_dbg_calloc -
 *
 * ARGUMENTS
 *   func   -
 *   line   -
 *   nelem  -
 *   elsize -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
void *otc_dbg_calloc(const char *func, int line, size_t nelem, size_t elsize)
{
	void *retptr;

	retptr = malloc(DBG_MEM_SIZE(nelem * elsize));
	if (retptr != nullptr)
		(void)memset(retptr, 0, DBG_MEM_SIZE(nelem * elsize));

	otc_dbg_mem_alloc(func, line, nullptr, retptr, nelem * elsize);

	return DBG_MEM_RETURN(retptr);
}


/***
 * NAME
 *   otc_dbg_realloc -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   ptr  -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
void *otc_dbg_realloc(const char *func, int line, void *ptr, size_t size)
{
	void *retptr;

	retptr = realloc(ptr, DBG_MEM_SIZE(size));

	otc_dbg_mem_alloc(func, line, ptr, retptr, size);

	return DBG_MEM_RETURN(retptr);
}


/***
 * NAME
 *   otc_dbg_free -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   ptr  -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_dbg_free(const char *func, int line, void *ptr)
{
	struct otc_dbg_mem_metadata *metadata;

	otc_dbg_mem_release(func, line, ptr, 2);

	metadata = DBG_MEM_DATA(ptr);
	if ((metadata == nullptr) || (metadata->data == nullptr) || (metadata->magic != DBG_MEM_MAGIC))
		free(ptr);
	else
		free(DBG_MEM_DATA(ptr));
}


/***
 * NAME
 *   otc_dbg_strdup -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   s    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
char *otc_dbg_strdup(const char *func, int line, const char *s)
{
	size_t  len = 0;
	char   *retptr = nullptr;

	if (s != nullptr) {
		len    = strlen(s) + 1;
		retptr = OT_CAST_TYPEOF(retptr, malloc(DBG_MEM_SIZE(len)));
		if (retptr != nullptr)
			(void)memcpy(DBG_MEM_PTR(retptr), s, len);
	}

	otc_dbg_mem_alloc(func, line, nullptr, retptr, len);

	return OT_CAST_TYPEOF(retptr, DBG_MEM_RETURN(retptr));
}


/***
 * NAME
 *   otc_dbg_strndup -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   s    -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
char *otc_dbg_strndup(const char *func, int line, const char *s, size_t size)
{
	size_t  len = 0;
	char   *retptr = nullptr;

	if (s != nullptr) {
		len    = strlen(s);
		len    = (len < size) ? len : size;
		retptr = OT_CAST_TYPEOF(retptr, malloc(DBG_MEM_SIZE(len + 1)));
		if (retptr != nullptr) {
			(void)memcpy(DBG_MEM_PTR(retptr), s, len);
			DBG_MEM_PTR(retptr)[len] = '\0';
		}
	}

	otc_dbg_mem_alloc(func, line, nullptr, retptr, len + 1);

	return OT_CAST_TYPEOF(retptr, DBG_MEM_RETURN(retptr));
}


/***
 * NAME
 *   otc_dbg_mem_init -
 *
 * ARGUMENTS
 *   mem   -
 *   data  -
 *   count -
 *   level -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int otc_dbg_mem_init(struct otc_dbg_mem *mem, struct otc_dbg_mem_data *data, size_t count, uint8_t level)
{
	pthread_mutexattr_t attr;
	int                 rc, retval = -1;

	if ((mem == nullptr) || (data == nullptr) || (count == 0))
		return retval;

	(void)memset(mem, 0, sizeof(*mem));
	(void)memset(data, 0, sizeof(*data) * count);

	dbg_mem        = mem;
	dbg_mem->data  = data;
	dbg_mem->count = count;
	dbg_mem->level = level;

	if ((rc = pthread_mutexattr_init(&attr)) == 0)
		if ((rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) == 0)
			if ((rc = pthread_mutex_init(&(dbg_mem->mutex), &attr)) == 0)
				retval = 0;

	return retval;
}


/***
 * NAME
 *   otc_dbg_mem_disable -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_dbg_mem_disable(void)
{
	if (dbg_mem == nullptr)
		return;

	(void)pthread_mutex_destroy(&(dbg_mem->mutex));

	dbg_mem = nullptr;
}


/***
 * NAME
 *   otc_dbg_mem_info -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_dbg_mem_info(void)
{
	struct mallinfo mi;
	size_t          i, n = 0;
	uint64_t        size = 0;

	if (dbg_mem == nullptr)
		return;

	DBG_MEM_INFO(0, "--- Memory info -------------------------------------");
	DBG_MEM_INFO(0, "  alloc/realloc: %" PRIu64 "/%" PRIu64 ", free/release: %" PRIu64 "/%" PRIu64, dbg_mem->op_cnt[0], dbg_mem->op_cnt[1], dbg_mem->op_cnt[2], dbg_mem->op_cnt[3]);
	DBG_MEM_INFO(0, "  unused: %zu, reused: %zu, count: %zu", dbg_mem->unused, dbg_mem->reused, dbg_mem->count);
	for (i = 0; i < dbg_mem->count; i++)
		if (dbg_mem->data[i].used) {
			DBG_MEM_INFO(0, "  %zu %s(%p %zu)", n, dbg_mem->data[i].func, dbg_mem->data[i].ptr, dbg_mem->data[i].size);

			size += dbg_mem->data[i].size;
			n++;
		}

	if (n > 0)
		DBG_MEM_INFO(0, "  allocated %zu byte(s) in %zu chunk(s)", size, n);

	if (dbg_mem->size != size)
		DBG_MEM_INFO(0, "  size does not match: %zu != %zu", dbg_mem->size, size);

	mi = mallinfo();
	DBG_MEM_INFO(0, "--- Memory space usage ------------------------------");
	DBG_MEM_INFO(0, "  Total non-mmapped bytes:     %" PRI_MI, mi.arena);
	DBG_MEM_INFO(0, "  # of free chunks:            %" PRI_MI, mi.ordblks);
	DBG_MEM_INFO(0, "  # of free fastbin blocks:    %" PRI_MI, mi.smblks);
	DBG_MEM_INFO(0, "  Bytes in mapped regions:     %" PRI_MI, mi.hblkhd);
	DBG_MEM_INFO(0, "  # of mapped regions:         %" PRI_MI, mi.hblks);
	DBG_MEM_INFO(0, "  Max. total allocated space:  %" PRI_MI, mi.usmblks);
	DBG_MEM_INFO(0, "  Free bytes held in fastbins: %" PRI_MI, mi.fsmblks);
	DBG_MEM_INFO(0, "  Total allocated space:       %" PRI_MI, mi.uordblks);
	DBG_MEM_INFO(0, "  Total free space:            %" PRI_MI, mi.fordblks);
	DBG_MEM_INFO(0, "  Topmost releasable block:    %" PRI_MI, mi.keepcost);
}


/***
 * NAME
 *   otc_dbg_memdup -
 *
 * ARGUMENTS
 *   func -
 *   line -
 *   s    -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
void *otc_dbg_memdup(const char *func, int line, const void *s, size_t size)
{
	void *retptr = nullptr;

	if (s != nullptr) {
		retptr = malloc(DBG_MEM_SIZE(size + 1));
		if (retptr != nullptr) {
			(void)memcpy(DBG_MEM_PTR(retptr), s, size);
			DBG_MEM_PTR(retptr)[size] = '\0';
		}
	}

	otc_dbg_mem_alloc(func, line, nullptr, retptr, size);

	return DBG_MEM_RETURN(retptr);
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
