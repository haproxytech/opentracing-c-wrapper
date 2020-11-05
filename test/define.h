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
#ifndef TEST_DEFINE_H
#define TEST_DEFINE_H

#ifdef DEBUG
#  define OTC_DBG_MEM
#endif

#ifndef PACKAGE_BUILD
#  define PACKAGE_BUILD         0
#endif

#define OT_USE_INJECT_CB
#define OT_USE_EXTRACT_CB

#ifdef __linux__
#  define PRI_PIDT              "d"
#  define PRI_PTHREADT          "lu"
#else
#  define PRI_PIDT              "ld"
#  define PRI_PTHREADT          "u"
#endif

#ifndef MIN
#  define MIN(a,b)              (((a) < (b)) ? (a) : (b))
#endif

#define _NULL(a)                ((a) == NULL)
#define _nNULL(a)               ((a) != NULL)
#define TABLESIZE(a)            ((int)(sizeof(a) / sizeof((a)[0])))
#define IN_RANGE(v,a,b)         (((v) >= (a)) && ((v) <= (b)))
#define TIMEVAL_DIFF_MS(a,b)    (((a)->tv_sec - (b)->tv_sec) * 1000ULL + ((a)->tv_usec - (b)->tv_usec + 500) / 1000)
#define TIMEVAL_DIFF_US(a,b)    (((a)->tv_sec - (b)->tv_sec) * 1000000ULL + (a)->tv_usec - (b)->tv_usec)
#define NIBBLE_TO_HEX(a)        ((a) + (((a) < 10) ? '0' : ('a' - 10)))
#define SWAP(a,b)               do { typeof(a) _a = (a); (a) = (b); (b) = _a; } while (0)
#define OT_VARGS(t,v)           otc_value_##t, (v)

#endif /* TEST_DEFINE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
