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
#ifndef TEST_DEBUG_H
#define TEST_DEBUG_H

#define OT_LOG(f,...)       (void)printf("[%4d] " f "\n", thread_id(), ##__VA_ARGS__)

#ifdef DEBUG
enum DBG_LEVEL_enum {
	DBG_LEVEL_FUNC = 0, /* Function debug level. */
	DBG_LEVEL_INFO,     /* Generic info level. */
	DBG_LEVEL_DEBUG,    /* Generic debug level. */
	DBG_LEVEL_OT,       /* OpenTracing debug level. */
	DBG_LEVEL_WORKER,   /* Worker debug level. */
	DBG_LEVEL_ENABLED,  /* This have to be the last entry. */
};

#  define OT_FUNC(f,...)    \
	do {                                                           \
		if (_nNULL(cfg_debug_level) && (*cfg_debug_level > 0)) \
			OT_LOG("%s(" f ")", __func__, ##__VA_ARGS__);  \
	} while (0)
#  define OT_DBG(l,f,...)   \
	do {                                                                              \
		if (_nNULL(cfg_debug_level) && (*cfg_debug_level & (1 << DBG_LEVEL_##l))) \
			OT_LOG("  " f, ##__VA_ARGS__);                                    \
	} while (0)


extern uint8_t *cfg_debug_level;
#else
#  define OT_FUNC(...)      do { } while (0)
#  define OT_DBG(...)       do { } while (0)
#endif /* DEBUG */

#endif /* TEST_DEBUG_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
