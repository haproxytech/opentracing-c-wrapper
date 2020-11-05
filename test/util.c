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


/***
 * NAME
 *   nsleep -
 *
 * ARGUMENTS
 *   sec  -
 *   nsec -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
void nsleep (uint sec, uint nsec)
{
	struct timespec ts1 = { sec, nsec }, ts2, *req = &ts1, *rem = &ts2;

	while ((nanosleep (req, rem) == -1) && (errno == EINTR))
		SWAP(req, rem);
}


/***
 * NAME
 *   str_hex -
 *
 * ARGUMENTS
 *   data -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
const char *str_hex(const void *data, size_t size)
{
	static __thread char  retbuf[BUFSIZ];
	const uint8_t        *ptr = data;
	size_t                i;

	if (_NULL(data))
		return "(null)";
	else if (size == 0)
		return "()";

	for (i = 0, size <<= 1; (i < (sizeof(retbuf) - 2)) && (i < size); ptr++) {
		retbuf[i++] = NIBBLE_TO_HEX(*ptr >> 4);
		retbuf[i++] = NIBBLE_TO_HEX(*ptr & 0x0f);
	}

	retbuf[i] = '\0';

	return retbuf;
}


/***
 * NAME
 *   str_ctrl -
 *
 * ARGUMENTS
 *   data -
 *   size -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
const char *str_ctrl(const void *data, size_t size)
{
	static __thread char  retbuf[BUFSIZ];
	const uint8_t        *ptr = data;
	size_t                i, n = 0;

	if (_NULL(data))
		return "(null)";
	else if (size == 0)
		return "()";

	for (i = 0; (n < (sizeof(retbuf) - 1)) && (i < size); i++)
		retbuf[n++] = IN_RANGE(ptr[i], 0x20, 0x7e) ? ptr[i] : '.';

	retbuf[n] = '\0';

	return retbuf;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
