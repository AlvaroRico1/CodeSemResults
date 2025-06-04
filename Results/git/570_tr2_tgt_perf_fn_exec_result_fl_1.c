static void fn_exec_result_fl(const char *file, int line,
			      uint64_t us_elapsed_absolute, int exec_id,
			      int code)
{
	const char *event_name = "exec_result";
	struct strbuf buf_payload = STRBUF_INIT;

	strbuf_addf(&buf_payload, "id:%d code:%d", exec_id, code);
	if (code > 0)
		strbuf_addf(&buf_payload, " err:%s", strerror(code));

	perf_io_write_fl(file, line, event_name, NULL, &us_elapsed_absolute,
			 NULL, NULL, &buf_payload);
	strbuf_release(&buf_payload);
}

static void fn_param_fl(const char *file, int line, const char *param,
			const char *value)
{
	const char *event_name = "def_param";
	struct strbuf buf_payload = STRBUF_INIT;

	strbuf_addf(&buf_payload, "%s:%s", param, value);

	perf_io_write_fl(file, line, event_name, NULL, NULL, NULL, NULL,
			 &buf_payload);
	strbuf_release(&buf_payload);
}


// Source: tr2_tgt_perf.c
// Lines 421-448
