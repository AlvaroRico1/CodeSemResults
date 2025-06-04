rb_simple_write(struct file *filp, const char __user *ubuf,
		size_t cnt, loff_t *ppos)
{
	struct trace_array *tr = filp->private_data;
	struct ring_buffer *buffer = tr->trace_buffer.buffer;
	unsigned long val;
	int ret;

	ret = kstrtoul_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	if (buffer) {
		mutex_lock(&trace_types_lock);
		if (!!val == tracer_tracing_is_on(tr)) {
			val = 0; /* do nothing */
		} else if (val) {
			tracer_tracing_on(tr);
			if (tr->current_trace->start)
				tr->current_trace->start(tr);
		} else {
			tracer_tracing_off(tr);
			if (tr->current_trace->stop)
				tr->current_trace->stop(tr);
		}
		mutex_unlock(&trace_types_lock);
	}

	(*ppos)++;

	return cnt;
}


// Source: trace.c
// Lines 8124-8155
