static void fn_child_exit_fl(const char *file, int line,
			     uint64_t us_elapsed_absolute, int cid, int pid,
			     int code, uint64_t us_elapsed_child)
{
	const char *event_name = "child_exit";
	struct strbuf buf_payload = STRBUF_INIT;

	strbuf_addf(&buf_payload, "[ch%d] pid:%d code:%d", cid, pid, code);

	perf_io_write_fl(file, line, event_name, NULL, &us_elapsed_absolute,
			 &us_elapsed_child, NULL, &buf_payload);
	strbuf_release(&buf_payload);
}


// Source: tr2_tgt_perf.c
// Lines 349-361
