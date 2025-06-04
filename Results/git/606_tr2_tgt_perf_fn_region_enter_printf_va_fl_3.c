static void fn_region_enter_printf_va_fl(const char *file, int line,
					 uint64_t us_elapsed_absolute,
					 const char *category,
					 const char *label,
					 const struct repository *repo,
					 const char *fmt, va_list ap)
{
	const char *event_name = "region_enter";
	struct strbuf buf_payload = STRBUF_INIT;

	if (label)
		strbuf_addf(&buf_payload, "label:%s", label);
	if (fmt && *fmt) {
		strbuf_addch(&buf_payload, ' ');
		maybe_append_string_va(&buf_payload, fmt, ap);
	}


// Source: tr2_tgt_perf.c
// Lines 464-479
