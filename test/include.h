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
#ifndef TEST_INCLUDE_H
#define TEST_INCLUDE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <pthread.h>
#include <sysexits.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux__
#  include <sys/syscall.h>
#endif

#include "config.h"
#include "define.h"

#include "opentracing-c-wrapper/define.h"
#include "opentracing-c-wrapper/dbg_malloc.h"
#include "opentracing-c-wrapper/common.h"
#include "opentracing-c-wrapper/util.h"
#include "opentracing-c-wrapper/value.h"
#include "opentracing-c-wrapper/span.h"
#include "opentracing-c-wrapper/propagation.h"
#include "opentracing-c-wrapper/tracer.h"

#include "version.h"
#include "debug.h"
#include "opentracing.h"
#include "util.h"

#endif /* TEST_INCLUDE_H */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
