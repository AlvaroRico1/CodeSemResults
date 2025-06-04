static void fn_region_leave_printf_va_fl(
	const char *file, int line, uint64_t us_elapsed_absolute,
	uint64_t us_elapsed_region, const char *category, const char *label,
	const struct repository *repo, const char *fmt, va_list ap)
{
	const char *event_name = "region_leave";
	struct strbuf buf_payload = STRBUF_INIT;

	if (label)
		strbuf_addf(&buf_payload, "label:%s", label);
	if (fmt && *fmt) {
		strbuf_addch(&buf_payload, ' ' );
		maybe_append_string_va(&buf_payload, fmt, ap);
	}

	perf_io_write_fl(file, line, event_name, repo, &us_elapsed_absolute,
			 &us_elapsed_region, category, &buf_payload);
	strbuf_release(&buf_payload);
}


// Source: tr2_tgt_perf.c
// Lines 486-504
