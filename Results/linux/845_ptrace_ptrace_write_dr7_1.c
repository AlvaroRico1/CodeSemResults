static int ptrace_write_dr7(struct task_struct *tsk, unsigned long data)
{
	struct thread_struct *thread = &tsk->thread;
	unsigned long old_dr7;
	bool second_pass = false;
	int i, rc, ret = 0;

	data &= ~DR_CONTROL_RESERVED;
	old_dr7 = ptrace_get_dr7(thread->ptrace_bps);

restore:
	rc = 0;
	for (i = 0; i < HBP_NUM; i++) {
		unsigned len, type;
		bool disabled = !decode_dr7(data, i, &len, &type);
		struct perf_event *bp = thread->ptrace_bps[i];

		if (!bp) {
			if (disabled)
				continue;

			bp = ptrace_register_breakpoint(tsk,
					len, type, 0, disabled);
			if (IS_ERR(bp)) {
				rc = PTR_ERR(bp);
				break;
			}

			thread->ptrace_bps[i] = bp;
			continue;
		}

		rc = ptrace_modify_breakpoint(bp, len, type, disabled);
		if (rc)
			break;
	}


// Source: ptrace.c
// Lines 564-599
