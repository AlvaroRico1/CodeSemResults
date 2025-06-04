store_trace_args(void *data, struct trace_probe *tp, struct pt_regs *regs,
		 int header_size, int maxlen)
{
	struct probe_arg *arg;
	void *base = data - header_size;
	void *dyndata = data + tp->size;
	u32 *dl;	/* Data location */
	int ret, i;

	for (i = 0; i < tp->nr_args; i++) {
		arg = tp->args + i;
		dl = data + arg->offset;
		/* Point the dynamic data area if needed */
		if (unlikely(arg->dynamic))
			*dl = make_data_loc(maxlen, dyndata - base);
		ret = process_fetch_insn(arg->code, regs, dl, base);
		if (unlikely(ret < 0 && arg->dynamic)) {
			*dl = make_data_loc(0, dyndata - base);
		} else {
			dyndata += ret;
			maxlen -= ret;
		}
	}
}


// Source: trace_probe_tmpl.h
// Lines 191-214
