void get_parallel_checkout_configs(int *num_workers, int *threshold)
{
	char *env_workers = getenv("GIT_TEST_CHECKOUT_WORKERS");

	if (env_workers && *env_workers) {
		if (strtol_i(env_workers, 10, num_workers)) {
			die("invalid value for GIT_TEST_CHECKOUT_WORKERS: '%s'",
			    env_workers);
		}
		if (*num_workers < 1)
			*num_workers = online_cpus();

		*threshold = 0;
		return;
	}

	if (git_config_get_int("checkout.workers", num_workers))
		*num_workers = DEFAULT_NUM_WORKERS;
	else if (*num_workers < 1)
		*num_workers = online_cpus();

	if (git_config_get_int("checkout.thresholdForParallelism", threshold))
		*threshold = DEFAULT_THRESHOLD_FOR_PARALLELISM;
}


// Source: parallel-checkout.c
// Lines 36-59
