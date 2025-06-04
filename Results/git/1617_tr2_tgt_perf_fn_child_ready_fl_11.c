static void fn_child_ready_fl(const char *file, int line,
			      uint64_t us_elapsed_absolute, int cid, int pid,
			      const char *ready, uint64_t us_elapsed_child)
{
	const char *event_name = "child_ready";
	struct strbuf buf_payload = STRBUF_INIT;

	strbuf_addf(&buf_payload, "[ch%d] pid:%d ready:%s", cid, pid, ready);

	perf_io_write_fl(file, line, event_name, NULL, &us_elapsed_absolute,
			 &us_elapsed_child, NULL, &buf_payload);
	strbuf_release(&buf_payload);
}


// Source: tr2_tgt_perf.c
// Lines 363-375
