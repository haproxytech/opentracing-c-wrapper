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


otc_ext_malloc_t otc_ext_malloc = OT_IFDEF_DBG(otc_dbg_malloc, malloc);
otc_ext_free_t   otc_ext_free   = OT_IFDEF_DBG(otc_dbg_free,   free);


/***
 * NAME
 *   timespec_to_duration -
 *
 * ARGUMENTS
 *   ts -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
std::chrono::nanoseconds timespec_to_duration(const struct timespec *ts)
{
	auto duration = std::chrono::seconds{ts->tv_sec} + std::chrono::nanoseconds{ts->tv_nsec};

	return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
}


/***
 * NAME
 *   otc_ext_init -
 *
 * ARGUMENTS
 *   func_malloc -
 *   func_free   -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_ext_init(otc_ext_malloc_t func_malloc, otc_ext_free_t func_free)
{
	otc_ext_malloc = (func_malloc != nullptr) ? func_malloc : OT_IFDEF_DBG(otc_dbg_malloc, malloc);
	otc_ext_free   = (func_free   != nullptr) ? func_free   : OT_IFDEF_DBG(otc_dbg_free,   free);
}


/***
 * NAME
 *   otc_text_map_new -
 *
 * ARGUMENTS
 *   text_map -
 *   size     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_text_map *otc_text_map_new(struct otc_text_map *text_map, size_t size)
{
	struct otc_text_map *retptr = text_map;

	if (retptr == nullptr)
		retptr = OT_CAST_TYPEOF(retptr, OTC_DBG_CALLOC(1, sizeof(*retptr)));

	if (retptr != nullptr) {
		retptr->count      = 0;
		retptr->size       = size;
		retptr->is_dynamic = text_map == nullptr;

		if (size == 0)
			/* Do nothing. */;
		else if ((retptr->key = OT_CAST_TYPEOF(retptr->key, OTC_DBG_CALLOC(size, sizeof(*(retptr->key))))) == nullptr)
			otc_text_map_destroy(&retptr, OT_CAST_STAT(otc_text_map_flags_t, 0));
		else if ((retptr->value = OT_CAST_TYPEOF(retptr->value, OTC_DBG_CALLOC(size, sizeof(*(retptr->value))))) == nullptr)
			otc_text_map_destroy(&retptr, OT_CAST_STAT(otc_text_map_flags_t, 0));
	}

	return retptr;
}


/***
 * NAME
 *   otc_text_map_add -
 *
 * ARGUMENTS
 *   text_map  -
 *   key       -
 *   key_len   -
 *   value     -
 *   value_len -
 *   flags     -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int otc_text_map_add(struct otc_text_map *text_map, const char *key, size_t key_len, const char *value, size_t value_len, otc_text_map_flags_t flags)
{
	int retval = -1;

	if ((text_map == nullptr) || (key == nullptr) || (value == nullptr))
		return retval;

	/*
	 * Check if it is necessary to increase the number of key/value pairs.
	 * The number of pairs is increased by half the current number of pairs
	 * (for example: 8 -> 12 -> 18 -> 27 -> 40 -> 60 ...).
	 */
	if (text_map->count >= text_map->size) {
		typeof(text_map->key)   ptr_key;
		typeof(text_map->value) ptr_value;
		size_t                  size_add = (text_map->size > 1) ? (text_map->size / 2) : 1;

		if ((ptr_key = OT_CAST_TYPEOF(ptr_key, OTC_DBG_REALLOC(text_map->key, OT_TEXT_MAP_SIZE(key, size_add)))) == nullptr)
			return retval;

		text_map->key = ptr_key;
		(void)memset(text_map->key + OT_TEXT_MAP_SIZE(key, 0), 0, sizeof(*(text_map->key)) * size_add);

		if ((ptr_value = OT_CAST_TYPEOF(ptr_value, OTC_DBG_REALLOC(text_map->value, OT_TEXT_MAP_SIZE(value, size_add)))) == nullptr)
			return retval;

		text_map->value = ptr_value;
		(void)memset(text_map->value + OT_TEXT_MAP_SIZE(value, 0), 0, sizeof(*(text_map->value)) * size_add);

		text_map->size += size_add;
	}

	text_map->key[text_map->count]   = (flags & OTC_TEXT_MAP_DUP_KEY) ? ((key_len > 0) ? OTC_DBG_STRNDUP(key, key_len) : OTC_DBG_STRDUP(key)) : OT_CAST_CONST(char *, key);
	text_map->value[text_map->count] = (flags & OTC_TEXT_MAP_DUP_VALUE) ? ((value_len > 0) ? OTC_DBG_STRNDUP(value, value_len) : OTC_DBG_STRDUP(value)) : OT_CAST_CONST(char *, value);

	if ((text_map->key[text_map->count] != nullptr) && (text_map->value[text_map->count] != nullptr))
		retval = text_map->count;

	text_map->count++;

	return retval;
}


/***
 * NAME
 *   otc_text_map_destroy -
 *
 * ARGUMENTS
 *   text_map -
 *   flags    -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_text_map_destroy(struct otc_text_map **text_map, otc_text_map_flags_t flags)
{
	if ((text_map == nullptr) || (*text_map == nullptr))
		return;

	if ((*text_map)->key != nullptr) {
		if (flags & OTC_TEXT_MAP_FREE_KEY)
			for (size_t i = 0; i < (*text_map)->count; i++)
				OT_FREE((*text_map)->key[i]);

		OT_FREE_CLEAR((*text_map)->key);
	}

	if ((*text_map)->value != nullptr) {
		if (flags & OTC_TEXT_MAP_FREE_VALUE)
			for (size_t i = 0; i < (*text_map)->count; i++)
				OT_FREE((*text_map)->value[i]);

		OT_FREE_CLEAR((*text_map)->value);
	}

	if ((*text_map)->is_dynamic) {
		OT_FREE_CLEAR(*text_map);
	} else {
		(*text_map)->count = 0;
		(*text_map)->size  = 0;
	}
}


/***
 * NAME
 *   otc_binary_data_new -
 *
 * ARGUMENTS
 *   binary_data -
 *   data        -
 *   size        -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
struct otc_binary_data *otc_binary_data_new(struct otc_binary_data *binary_data, const void *data, size_t size)
{
	struct otc_binary_data *retptr = binary_data;

	if (retptr == nullptr)
		retptr = OT_CAST_TYPEOF(retptr, OTC_DBG_CALLOC(1, sizeof(*retptr)));

	if (retptr != nullptr) {
		retptr->size       = size;
		retptr->is_dynamic = binary_data == nullptr;

		if ((data == nullptr) || (size == 0))
			/* Do nothing. */;
		else if ((retptr->data = OTC_DBG_MALLOC(size)) != nullptr)
			(void)memcpy(retptr->data, data, size);
		else
			otc_binary_data_destroy(&retptr);
	}

	return retptr;
}


/***
 * NAME
 *   otc_binary_data_destroy -
 *
 * ARGUMENTS
 *   binary_data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_binary_data_destroy(struct otc_binary_data **binary_data)
{
	if ((binary_data == nullptr) || (*binary_data == nullptr))
		return;

	OT_FREE_CLEAR((*binary_data)->data);

	if ((*binary_data)->is_dynamic)
		OT_FREE_CLEAR(*binary_data);
	else
		(*binary_data)->size = 0;
}


/***
 * NAME
 *   otc_strerror -
 *
 * ARGUMENTS
 *   errnum -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
const char *otc_strerror(int errnum)
{
	static __THR char  retbuf[1024];
	const char        *retptr = retbuf;

	errno = 0;
	(void)strerror_r(errnum, retbuf, sizeof(retbuf));
	if (errno != 0)
		retptr = "Unknown error";

	return retptr;
}


/***
 * NAME
 *   otc_file_read -
 *
 * ARGUMENTS
 *   filename  -
 *   comment   -
 *   errbuf    -
 *   errbufsiz -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
char *otc_file_read(const char *filename, const char *comment, char *errbuf, int errbufsiz)
{
	struct stat  statbuf;
	char        *retptr = nullptr;
	int          fd, rc;

	if (filename == nullptr)
		return retptr;

	if ((fd = open(filename, O_RDONLY)) == -1) {
		(void)snprintf(errbuf, errbufsiz, "'%s': %s", filename, otc_strerror(errno));
	}
	else if ((rc = fstat(fd, &statbuf)) == -1) {
		(void)snprintf(errbuf, errbufsiz, "'%s': %s", filename, otc_strerror(errno));
	}
	else if ((retptr = OT_CAST_TYPEOF(retptr, OTC_DBG_MALLOC(statbuf.st_size + 1))) == nullptr) {
		(void)snprintf(errbuf, errbufsiz, "cannot allocate memory: %s", otc_strerror(errno));
	}
	else {
		char  *buf = retptr;
		off_t  size = statbuf.st_size;

		while (size > 0) {
			ssize_t  n;

			if ((n = read(fd, buf, size)) > 0) {
				size -= n;
				buf  += n;
			}
			else if (n == -1) {
				if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
					break;
				}
				else if (errno != EINTR)
				{
					(void)snprintf(errbuf, errbufsiz, "'%s': %s", filename, otc_strerror(errno));
					OT_FREE_CLEAR(retptr);

					break;
				}
			}
			else {
				break;
			}
		}

		if (comment != nullptr) {
			off_t i = 0, c = -1, n = statbuf.st_size;

			for (i = 0; i < n; i++)
				if (c < 0) {
					/* Remember the starting position of the comment line. */
					if ((strchr(comment, retptr[i]) != nullptr) && ((i == 0) || (retptr[i - 1] == '\n')))
						c = i;
				}
				else if ((retptr[i] == '\n') && ((i + 1) < n)) {
					/* Delete the entire comment line. */
					(void)memmove(retptr + c, retptr + i + 1, n - i - 1);

					n -= i + 1 - c;
					i  = c - 1;
					c  = -1;
				}

			/* If a comment remains in the last line, delete it. */
			if (c >= 0) {
				n -= i - c;
				i  = c;
			}

			retptr[i] = '\0';
		}
		else if (size != 0)
			OT_FREE_CLEAR(retptr);
	}

	(void)close(fd);

	return retptr;
}


/***
 * NAME
 *   otc_statistics -
 *
 * ARGUMENTS
 *   buffer -
 *   bufsiz -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void otc_statistics(char *buffer, size_t bufsiz)
{
	if ((buffer == nullptr) || (bufsiz < 24))
		return;

	(void)snprintf(buffer, bufsiz, "span: %" PRId64 "/%" PRId64 "+%" PRId64 "(%" PRId64 ")/%" PRId64 ", context: %" PRId64 "/%" PRId64 "+%" PRId64 "(%"  PRId64 ")/%" PRId64,
	               ot_span.key, ot_span_handle.size(), ot_span.erase_cnt, ot_span.destroy_cnt, ot_span.alloc_fail_cnt,
	               ot_span_context.key, ot_span_context_handle.size(), ot_span_context.erase_cnt, ot_span_context.destroy_cnt, ot_span_context.alloc_fail_cnt);
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
