static void __blk_add_trace(struct blk_trace *bt, sector_t sector, int bytes,
		     int op, int op_flags, u32 what, int error, int pdu_len,
		     void *pdu_data, union kernfs_node_id *cgid)
{
	struct task_struct *tsk = current;
	struct ring_buffer_event *event = NULL;
	struct ring_buffer *buffer = NULL;
	struct blk_io_trace *t;
	unsigned long flags = 0;
	unsigned long *sequence;
	pid_t pid;
	int cpu, pc = 0;
	bool blk_tracer = blk_tracer_enabled;
	ssize_t cgid_len = cgid ? sizeof(*cgid) : 0;

	if (unlikely(bt->trace_state != Blktrace_running && !blk_tracer))
		return;

	what |= ddir_act[op_is_write(op) ? WRITE : READ];
	what |= MASK_TC_BIT(op_flags, SYNC);
	what |= MASK_TC_BIT(op_flags, RAHEAD);
	what |= MASK_TC_BIT(op_flags, META);
	what |= MASK_TC_BIT(op_flags, PREFLUSH);
	what |= MASK_TC_BIT(op_flags, FUA);
	if (op == REQ_OP_DISCARD || op == REQ_OP_SECURE_ERASE)
		what |= BLK_TC_ACT(BLK_TC_DISCARD);
	if (op == REQ_OP_FLUSH)
		what |= BLK_TC_ACT(BLK_TC_FLUSH);
	if (cgid)
		what |= __BLK_TA_CGROUP;

	pid = tsk->pid;
	if (act_log_check(bt, what, sector, pid))
		return;
	cpu = raw_smp_processor_id();

	if (blk_tracer) {
		tracing_record_cmdline(current);

		buffer = blk_tr->trace_buffer.buffer;
		pc = preempt_count();
		event = trace_buffer_lock_reserve(buffer, TRACE_BLK,
						  sizeof(*t) + pdu_len + cgid_len,
						  0, pc);
		if (!event)
			return;
		t = ring_buffer_event_data(event);
		goto record_it;
	}

	if (unlikely(tsk->btrace_seq != blktrace_seq))
		trace_note_tsk(tsk);

	/*
	 * A word about the locking here - we disable interrupts to reserve
	 * some space in the relay per-cpu buffer, to prevent an irq
	 * from coming in and stepping on our toes.
	 */
	local_irq_save(flags);
	t = relay_reserve(bt->rchan, sizeof(*t) + pdu_len + cgid_len);
	if (t) {
		sequence = per_cpu_ptr(bt->sequence, cpu);

		t->magic = BLK_IO_TRACE_MAGIC | BLK_IO_TRACE_VERSION;
		t->sequence = ++(*sequence);
		t->time = ktime_to_ns(ktime_get());
record_it:
		/*
		 * These two are not needed in ftrace as they are in the
		 * generic trace_entry, filled by tracing_generic_entry_update,
		 * but for the trace_event->bin() synthesizer benefit we do it
		 * here too.
		 */
		t->cpu = cpu;
		t->pid = pid;

		t->sector = sector;
		t->bytes = bytes;
		t->action = what;
		t->device = bt->dev;
		t->error = error;
		t->pdu_len = pdu_len + cgid_len;

		if (cgid_len)
			memcpy((void *)t + sizeof(*t), cgid, cgid_len);
		if (pdu_len)
			memcpy((void *)t + sizeof(*t) + cgid_len, pdu_data, pdu_len);

		if (blk_tracer) {
			trace_buffer_unlock_commit(blk_tr, buffer, event, 0, pc);
			return;
		}
	}

	local_irq_restore(flags);
}


// Source: blktrace.c
// Lines 213-308
