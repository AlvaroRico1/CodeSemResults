format_log_debug_cb(const char *key, const char *value, void *arg)
{
	const char	*prefix = arg;

	log_debug("%s: %s=%s", prefix, key, value);
}


// Source: format.c
// Lines 3100-3105
