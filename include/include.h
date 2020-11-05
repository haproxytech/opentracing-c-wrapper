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
#ifndef _OPENTRACING_C_WRAPPER_INCLUDE_H_
#define _OPENTRACING_C_WRAPPER_INCLUDE_H_

#include <cstdio>
#include <cinttypes>
#include <stdbool.h>
#include <sstream>
#include <mutex>

#include <opentracing/dynamic_load.h>
#include <opentracing/version.h>

#include "config.h"
#include "define.h"
#ifdef DEBUG
#  include "dbg_malloc.h"
#endif

#include "opentracing-c-wrapper/define.h"
#include "opentracing-c-wrapper/dbg_malloc.h"
#include "opentracing-c-wrapper/common.h"
#include "opentracing-c-wrapper/util.h"
#include "opentracing-c-wrapper/value.h"
#include "opentracing-c-wrapper/span.h"
#include "opentracing-c-wrapper/propagation.h"
#include "opentracing-c-wrapper/tracer.h"

#include "span.h"
#include "tracer.h"
#include "util.h"

#endif /* _OPENTRACING_C_WRAPPER_INCLUDE_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
