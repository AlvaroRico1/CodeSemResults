static int create_filter(struct trace_array *tr,
			 struct trace_event_call *call,
			 char *filter_string, bool set_str,
			 struct event_filter **filterp)
{
	struct filter_parse_error *pe = NULL;
	int err;

	/* filterp must point to NULL */
	if (WARN_ON(*filterp))
		*filterp = NULL;

	err = create_filter_start(filter_string, set_str, &pe, filterp);
	if (err)
		return err;

	err = process_preds(call, filter_string, *filterp, pe);
	if (err && set_str)
		append_filter_err(tr, pe, *filterp);
	create_filter_finish(pe);

	return err;
}


// Source: trace_events_filter.c
// Lines 1730-1752
