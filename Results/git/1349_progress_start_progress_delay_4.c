static struct progress *start_progress_delay(const char *title, uint64_t total,
					     unsigned delay, unsigned sparse)
{
	struct progress *progress = xmalloc(sizeof(*progress));
	progress->title = title;
	progress->total = total;
	progress->last_value = -1;
	progress->last_percent = -1;
	progress->delay = delay;
	progress->sparse = sparse;
	progress->throughput = NULL;
	progress->start_ns = getnanotime();
	strbuf_init(&progress->counters_sb, 0);
	progress->title_len = utf8_strwidth(title);
	progress->split = 0;
	set_progress_signal();
	trace2_region_enter("progress", title, the_repository);
	return progress;
}


// Source: progress.c
// Lines 252-270
