void do_machine_check(struct pt_regs *regs, long error_code)
{
	DECLARE_BITMAP(valid_banks, MAX_NR_BANKS);
	DECLARE_BITMAP(toclear, MAX_NR_BANKS);
	struct mca_config *cfg = &mca_cfg;
	int cpu = smp_processor_id();
	char *msg = "Unknown";
	struct mce m, *final;
	int worst = 0;

	/*
	 * Establish sequential order between the CPUs entering the machine
	 * check handler.
	 */
	int order = -1;

	/*
	 * If no_way_out gets set, there is no safe way to recover from this
	 * MCE.  If mca_cfg.tolerant is cranked up, we'll try anyway.
	 */
	int no_way_out = 0;

	/*
	 * If kill_it gets set, there might be a way to recover from this
	 * error.
	 */
	int kill_it = 0;

	/*
	 * MCEs are always local on AMD. Same is determined by MCG_STATUS_LMCES
	 * on Intel.
	 */
	int lmce = 1;

	if (__mc_check_crashing_cpu(cpu))
		return;

	ist_enter(regs);

	this_cpu_inc(mce_exception_count);

	mce_gather_info(&m, regs);
	m.tsc = rdtsc();

	final = this_cpu_ptr(&mces_seen);
	*final = m;

	memset(valid_banks, 0, sizeof(valid_banks));
	no_way_out = mce_no_way_out(&m, &msg, valid_banks, regs);

	barrier();

	/*
	 * When no restart IP might need to kill or panic.
	 * Assume the worst for now, but if we find the
	 * severity is MCE_AR_SEVERITY we have other options.
	 */
	if (!(m.mcgstatus & MCG_STATUS_RIPV))
		kill_it = 1;

	/*
	 * Check if this MCE is signaled to only this logical processor,
	 * on Intel only.
	 */
	if (m.cpuvendor == X86_VENDOR_INTEL)
		lmce = m.mcgstatus & MCG_STATUS_LMCES;

	/*
	 * Local machine check may already know that we have to panic.
	 * Broadcast machine check begins rendezvous in mce_start()
	 * Go through all banks in exclusion of the other CPUs. This way we
	 * don't report duplicated events on shared banks because the first one
	 * to see it will clear it.
	 */
	if (lmce) {
		if (no_way_out)
			mce_panic("Fatal local machine check", &m, msg);
	} else {
		order = mce_start(&no_way_out);
	}

	__mc_scan_banks(&m, final, toclear, valid_banks, no_way_out, &worst);

	if (!no_way_out)
		mce_clear_state(toclear);

	/*
	 * Do most of the synchronization with other CPUs.
	 * When there's any problem use only local no_way_out state.
	 */
	if (!lmce) {
		if (mce_end(order) < 0)
			no_way_out = worst >= MCE_PANIC_SEVERITY;
	} else {
		/*
		 * If there was a fatal machine check we should have
		 * already called mce_panic earlier in this function.
		 * Since we re-read the banks, we might have found
		 * something new. Check again to see if we found a
		 * fatal error. We call "mce_severity()" again to
		 * make sure we have the right "msg".
		 */
		if (worst >= MCE_PANIC_SEVERITY && mca_cfg.tolerant < 3) {
			mce_severity(&m, cfg->tolerant, &msg, true);
			mce_panic("Local fatal machine check!", &m, msg);
		}
	}

	/*
	 * If tolerant is at an insane level we drop requests to kill
	 * processes and continue even when there is no way out.
	 */
	if (cfg->tolerant == 3)
		kill_it = 0;
	else if (no_way_out)
		mce_panic("Fatal machine check on current CPU", &m, msg);

	if (worst > 0)
		irq_work_queue(&mce_irq_work);

	mce_wrmsrl(MSR_IA32_MCG_STATUS, 0);

	sync_core();

	if (worst != MCE_AR_SEVERITY && !kill_it)
		goto out_ist;

	/* Fault was in user mode and we need to take some action */
	if ((m.cs & 3) == 3) {
		ist_begin_non_atomic(regs);
		local_irq_enable();

		if (kill_it || do_memory_failure(&m))
			force_sig(SIGBUS);
		local_irq_disable();
		ist_end_non_atomic();
	} else {
		if (!fixup_exception(regs, X86_TRAP_MC, error_code, 0))
			mce_panic("Failed kernel mode recovery", &m, NULL);
	}

out_ist:
	ist_exit(regs);
}


// Source: core.c
// Lines 1218-1361
