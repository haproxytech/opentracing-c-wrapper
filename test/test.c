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


#define DEFAULT_DEBUG_LEVEL     0
#define DEFAULT_THREADS_COUNT   1000


typedef unsigned char bool_t;

enum FLAG_OPT_enum {
	FLAG_OPT_HELP    = 0x01,
	FLAG_OPT_VERSION = 0x02,
};

static struct {
	uint8_t            debug_level;
	uint8_t            opt_flags;
	int                runcount;
	int                runtime_ms;
	int                threads;
	const char        *ot_config;
	const char        *ot_plugin;
	struct otc_tracer *ot_tracer;
} cfg = {
	.debug_level = DEFAULT_DEBUG_LEVEL,
	.runtime_ms  = -1,
	.threads     = DEFAULT_THREADS_COUNT,
};

enum OT_SPAN_enum {
	OT_SPAN_ROOT = 0,
	OP_SPAN_BAGGAGE,
	OT_SPAN_PROP_TM,
	OT_SPAN_PROP_HH,
	OT_SPAN_PROP_BD,
	OT_SPAN_MAX,
};

struct worker {
	pthread_t        thread;
	int              id;
	struct otc_span *ot_span[OT_SPAN_MAX];
	int              ot_state;
	uint64_t         count;
};

static struct {
	const char      *name;
	struct timeval   start_time;
	struct worker    worker[8192];
	volatile bool_t  flag_run;
} prg;


uint8_t *cfg_debug_level = &(cfg.debug_level);


/***
 * NAME
 *   thread_id -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
int thread_id(void)
{
	pthread_t id;
	int       i;

	id = pthread_self();

	for (i = 0; i < MIN(cfg.threads, TABLESIZE(prg.worker)); i++)
		if (pthread_equal(prg.worker[i].thread, id))
			return i + 1;

	return 0;
}


/***
 * NAME
 *   worker_finish_all_spans -
 *
 * ARGUMENTS
 *   worker -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static void worker_finish_all_spans(struct worker *worker)
{
	struct timespec ts;
	int             i;

	OT_FUNC("%p", worker);

	(void)clock_gettime(CLOCK_MONOTONIC, &ts);

	for (i = 0; i < TABLESIZE(worker->ot_span); i++)
		if (_nNULL(worker->ot_span[i]))
			ot_span_finish(worker->ot_span + i, &ts);
}


/***
 * NAME
 *   worker_thread -
 *
 * ARGUMENTS
 *   data -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
__attribute__((noreturn)) static void *worker_thread(void *data)
{
	struct otc_text_map_writer        tm_wr;
	struct otc_http_headers_writer    hh_wr;
	struct otc_custom_carrier_writer  cc_wr;
	struct otc_span_context          *context;
	struct timeval                    now;
	char                              name[16];
	struct worker                    *worker = data;
#ifdef DEBUG
	int                               n = 0;
#endif

	OT_FUNC("%p", data);

#ifdef __linux__
	OT_DBG(WORKER, "Worker started, thread id: %" PRI_PTHREADT, syscall(SYS_gettid));
#else
	OT_DBG(WORKER, "Worker started, thread id: %" PRI_PTHREADT, worker->thread);
#endif

	(void)gettimeofday(&now, NULL);
	(void)srandom(now.tv_usec);

	(void)snprintf(name, sizeof(name), "test/wrk: %d", worker->id);
	(void)pthread_setname_np(worker->thread, name);

	while (!prg.flag_run) {
		nsleep(0, 10000000);

#ifdef DEBUG
		n++;
#endif
	}

	OT_DBG(DEBUG, "waiting loop count: %d", n);

	for ( ; 1; worker->ot_state++) {
		if (worker->ot_state != 0)
			/* Do nothing. */;
		else if ((cfg.runtime_ms > 0) && (TIMEVAL_DIFF_MS(&now, &(prg.start_time)) >= (uint)cfg.runtime_ms))
			break;
		else if ((cfg.runcount > 0) && ((uint)cfg.runcount <= worker->count))
			break;

		if (worker->ot_state == 0) {
			worker->ot_span[OT_SPAN_ROOT] = ot_span_init(cfg.ot_tracer, "root span", -1, -1, NULL);
		}
		else if (worker->ot_state == 1) {
			worker->ot_span[OP_SPAN_BAGGAGE] = ot_span_init(cfg.ot_tracer, "span #1", otc_span_reference_child_of, -1, worker->ot_span[OT_SPAN_ROOT]);
		}
		else if (worker->ot_state == 2) {
			(void)ot_span_set_baggage(worker->ot_span[OP_SPAN_BAGGAGE], "baggage_1", "value_1", "baggage_2", "value_2", NULL);
		}
		else if (worker->ot_state == 3) {
			if (_nNULL(context = ot_inject_text_map(cfg.ot_tracer, worker->ot_span[OP_SPAN_BAGGAGE], &tm_wr)))
				context->destroy(&context);
		}
		else if (worker->ot_state == 4) {
			struct otc_text_map_reader  tm_rd;
			struct otc_text_map        *text_map = &(tm_wr.text_map);

			if (_nNULL(context = ot_extract_text_map(cfg.ot_tracer, &tm_rd, text_map))) {
				worker->ot_span[OT_SPAN_PROP_TM] = ot_span_init(cfg.ot_tracer, "text map propagation", otc_span_reference_child_of, context->idx, NULL);
				context->destroy(&context);
			}
			otc_text_map_destroy(&text_map, OTC_TEXT_MAP_FREE_KEY | OTC_TEXT_MAP_FREE_VALUE);
		}
		else if (worker->ot_state == 5) {
			struct otc_text_map *baggage = ot_span_baggage(worker->ot_span[OP_SPAN_BAGGAGE], "baggage_1", "baggage_2", NULL);

			otc_text_map_destroy(&baggage, OTC_TEXT_MAP_FREE_KEY | OTC_TEXT_MAP_FREE_VALUE);
		}
		else if (worker->ot_state == 6) {
			if (_nNULL(context = ot_inject_http_headers(cfg.ot_tracer, worker->ot_span[OP_SPAN_BAGGAGE], &hh_wr)))
				context->destroy(&context);
		}
		else if (worker->ot_state == 7) {
			struct otc_http_headers_reader  hh_rd;
			struct otc_text_map            *text_map = &(hh_wr.text_map);

			if (_nNULL(context = ot_extract_http_headers(cfg.ot_tracer, &hh_rd, text_map))) {
				worker->ot_span[OT_SPAN_PROP_HH] = ot_span_init(cfg.ot_tracer, "http headers propagation", otc_span_reference_child_of, context->idx, NULL);
				context->destroy(&context);
			}
			otc_text_map_destroy(&text_map, OTC_TEXT_MAP_FREE_KEY | OTC_TEXT_MAP_FREE_VALUE);
		}
		else if (worker->ot_state == 8) {
			if (_nNULL(context = ot_inject_binary(cfg.ot_tracer, worker->ot_span[OP_SPAN_BAGGAGE], &cc_wr)))
				context->destroy(&context);
		}
		else if (worker->ot_state == 9) {
			struct otc_custom_carrier_reader  cc_rd;
			struct otc_binary_data           *binary_data = &(cc_wr.binary_data);

			if (_nNULL(context = ot_extract_binary(cfg.ot_tracer, &cc_rd, binary_data))) {
				worker->ot_span[OT_SPAN_PROP_BD] = ot_span_init(cfg.ot_tracer, "binary data propagation", otc_span_reference_child_of, context->idx, NULL);
				context->destroy(&context);
			}
			otc_binary_data_destroy(&binary_data);
		}
		else if (worker->ot_state == 10) {
			(void)ot_span_tag(worker->ot_span[OT_SPAN_ROOT], "tag_1", OT_VARGS(string, "value_1"), "tag_2", OT_VARGS(string, "value_2"), NULL);
		}
		else if (worker->ot_state == 11) {
			(void)ot_span_log_kv(worker->ot_span[OT_SPAN_PROP_TM], "log_1", "content_1", "log_2", "content_2", NULL);
		}
		else {
			worker_finish_all_spans(worker);

			worker->ot_state = -1;
			worker->count++;
		}

		nsleep(0, ((random() % 100) + 1) * 10000);
		(void)gettimeofday(&now, NULL);
	}

	pthread_exit(NULL);
}


/***
 * NAME
 *   worker_run -
 *
 * ARGUMENTS
 *   This function takes no arguments.
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   -
 */
static int worker_run(void)
{
	struct timeval now;
	char           ot_infbuf[BUFSIZ];
	uint64_t       total_count = 0;
	int            i, num_threads = 0, retval = EX_OK;

	OT_FUNC("");

	(void)pthread_setname_np(pthread_self(), "test/wrk: main");

	for (i = 0; i < cfg.threads; i++) {
		prg.worker[i].id = i + 1;

		if (pthread_create(&(prg.worker[i].thread), NULL, worker_thread, prg.worker + i) != 0)
			(void)fprintf(stderr, "ERROR: Failed to start thread for worker %d: %m\n", prg.worker[i].id);
		else
			num_threads++;
	}

	prg.flag_run = 1;

	(void)gettimeofday(&now, NULL);
	OT_DBG(WORKER, "%d threads started in %llu ms", num_threads, TIMEVAL_DIFF_MS(&now, &(prg.start_time)));

	for (i = 0; i < cfg.threads; i++) {
		if (pthread_join(prg.worker[i].thread, NULL) != 0)
			(void)fprintf(stderr, "ERROR: Failed to join worker thread %d: %m\n", prg.worker[i].id);
		else
			OT_LOG("worker %d count: %" PRIu64, i, prg.worker[i].count);

		total_count += prg.worker[i].count;
	}

	OT_LOG("%d worker(s) total count: %" PRIu64, cfg.threads, total_count);

	cfg.ot_tracer->close(cfg.ot_tracer);

	otc_statistics(ot_infbuf, sizeof(ot_infbuf));
	OT_LOG("OpenTracing statistics: %s", ot_infbuf);

	return retval;
}


/***
 * NAME
 *   usage -
 *
 * ARGUMENTS
 *   program_name -
 *   flag_verbose -
 *
 * DESCRIPTION
 *   -
 *
 * RETURN VALUE
 *   This function does not return a value.
 */
static void usage(const char *program_name, bool_t flag_verbose)
{
	(void)printf("\nUsage: %s { -h --help }\n", program_name);
	(void)printf("       %s { -V --version }\n", program_name);
	(void)printf("       %s { [ -R --runcount=VALUE ] | [ -r --runtime=TIME ] } [OPTION]...\n\n", program_name);

	if (flag_verbose) {
		(void)printf("Options are:\n");
		(void)printf("  -c, --config=FILE     Specify the configuration for the used tracer.\n");
#ifdef DEBUG
		(void)printf("  -d, --debug=LEVEL     Enable and specify the debug mode level (default: %d).\n", DEFAULT_DEBUG_LEVEL);
#endif
		(void)printf("  -h, --help            Show this text.\n");
		(void)printf("  -p, --plugin=FILE     Specify the OpenTracing compatible plugin library.\n");
		(void)printf("  -R, --runcount=VALUE  Execute this program a certain number of passes (0 = unlimited).\n");
		(void)printf("  -r, --runtime=TIME    Run this program for a certain amount of time (ms, 0 = unlimited).\n");
		(void)printf("  -t, --threads=VALUE   Specify the number of threads (default: %d).\n", DEFAULT_THREADS_COUNT);
		(void)printf("  -V, --version         Show program version.\n\n");
		(void)printf("Copyright 2020 HAProxy Technologies\n");
		(void)printf("SPDX-License-Identifier: Apache-2.0\n\n");
	} else {
		(void)printf("For help type: %s -h\n\n", program_name);
	}
}


int main(int argc, char **argv)
{
	static const struct option longopts[] = {
		{ "config",   required_argument, NULL, 'c' },
#ifdef DEBUG
		{ "debug",    required_argument, NULL, 'd' },
#endif
		{ "help",     no_argument,       NULL, 'h' },
		{ "plugin",   required_argument, NULL, 'p' },
		{ "runcount", required_argument, NULL, 'R' },
		{ "runtime",  required_argument, NULL, 'r' },
		{ "threads",  required_argument, NULL, 't' },
		{ "version",  no_argument,       NULL, 'V' },
		{ NULL,       0,                 NULL, 0   }
	};
#ifdef OTC_DBG_MEM
	static struct otc_dbg_mem_data  dbg_mem_data[1000000];
	struct otc_dbg_mem              dbg_mem;
#endif
	const char                     *shortopts = "c:d:hp:R:r:t:V";
	struct timeval                  now;
	int                             c, longopts_idx = -1, retval = EX_OK;
	bool_t                          flag_error = 0;
	char                            ot_errbuf[BUFSIZ];

	(void)gettimeofday(&(prg.start_time), NULL);

	prg.name = basename(argv[0]);

#ifdef OTC_DBG_MEM
	retval = otc_dbg_mem_init(&dbg_mem, dbg_mem_data, TABLESIZE(dbg_mem_data), 0xff);
	if (retval == -1) {
		(void)fprintf(stderr, "ERROR: cannot initialize memory debugger\n");

		return retval;
	}
#endif

	while ((c = getopt_long(argc, argv, shortopts, longopts, &longopts_idx)) != EOF) {
		if (c == 'c')
			cfg.ot_config = optarg;
#ifdef DEBUG
		else if (c == 'd')
			cfg.debug_level = atoi(optarg) & UINT8_C(0xff);
#endif
		else if (c == 'h')
			cfg.opt_flags |= FLAG_OPT_HELP;
		else if (c == 'p')
			cfg.ot_plugin = optarg;
		else if (c == 'R')
			cfg.runcount = atoi(optarg);
		else if (c == 'r')
			cfg.runtime_ms = atoi(optarg);
		else if (c == 't')
			cfg.threads = atoi(optarg);
		else if (c == 'V')
			cfg.opt_flags |= FLAG_OPT_VERSION;
		else
			retval = EX_USAGE;
	}

	if (cfg.opt_flags & FLAG_OPT_HELP) {
		usage(prg.name, 1);
	}
	else if (cfg.opt_flags & FLAG_OPT_VERSION) {
		(void)printf("\n%s v%s [build %d] by %s, %s\n\n", prg.name, PACKAGE_VERSION, PACKAGE_BUILD, PACKAGE_AUTHOR, __DATE__);
	}
	else {
		if ((cfg.runcount < 0) && (cfg.runtime_ms < 0)) {
			(void)fprintf(stderr, "ERROR: run count/time value not set\n");
			flag_error = 1;
		}

		if (!IN_RANGE(cfg.threads, 1, TABLESIZE(prg.worker))) {
			(void)fprintf(stderr, "ERROR: invalid number of threads '%d'\n", cfg.threads);
			flag_error = 1;
		}

		if (_NULL(cfg.ot_plugin) || _NULL(cfg.ot_config)) {
			(void)fprintf(stderr, "ERROR: the OpenTracing configuration not set\n");
			flag_error = 1;
		}

		if (flag_error)
			usage(prg.name, 0);
	}

	cfg_debug_level = &(cfg.debug_level);

	OT_FUNC("%d, %p", argc, argv);

	if (flag_error || (cfg.opt_flags & (FLAG_OPT_HELP | FLAG_OPT_VERSION)))
		return flag_error ? EX_USAGE : EX_OK;

	if (_NULL(cfg.ot_tracer = otc_tracer_load(cfg.ot_plugin, ot_errbuf, sizeof(ot_errbuf)))) {
		(void)fprintf(stderr, "ERROR: %s\n", (*ot_errbuf == '\0') ? "Unable to load tracing library" : ot_errbuf);

		retval = EX_SOFTWARE;
	}
	else if (otc_tracer_start(cfg.ot_config, NULL, ot_errbuf, sizeof(ot_errbuf)) == -1) {
		(void)fprintf(stderr, "ERROR: %s\n", (*ot_errbuf == '\0') ? "Unable to start tracing" : ot_errbuf);

		retval = EX_SOFTWARE;
	}
	else {
		retval = worker_run();
	}

	(void)gettimeofday(&now, NULL);
	OT_DBG(INFO, "Program runtime: %llu ms", TIMEVAL_DIFF_MS(&now, &(prg.start_time)));

	OTC_DBG_MEMINFO();

	return retval;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
