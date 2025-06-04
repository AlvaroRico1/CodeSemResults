static int init_common(git_runtime_init_fn init_fns[], size_t cnt)
{
	size_t i;
	int ret;

	/* Initialize subsystems that have global state */
	for (i = 0; i < cnt; i++) {
		if ((ret = init_fns[i]()) != 0)
			break;
	}

	GIT_MEMORY_BARRIER;

	return ret;
}


// Source: runtime.c
// Lines 16-30
