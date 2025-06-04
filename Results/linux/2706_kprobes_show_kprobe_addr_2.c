static int show_kprobe_addr(struct seq_file *pi, void *v)
{
	struct hlist_head *head;
	struct kprobe *p, *kp;
	const char *sym = NULL;
	unsigned int i = *(loff_t *) v;
	unsigned long offset = 0;
	char *modname, namebuf[KSYM_NAME_LEN];

	head = &kprobe_table[i];
	preempt_disable();
	hlist_for_each_entry_rcu(p, head, hlist) {
		sym = kallsyms_lookup((unsigned long)p->addr, NULL,
					&offset, &modname, namebuf);
		if (kprobe_aggrprobe(p)) {
			list_for_each_entry_rcu(kp, &p->list, list)
				report_probe(pi, kp, sym, offset, modname, p);
		} else
			report_probe(pi, p, sym, offset, modname, NULL);
	}
	preempt_enable();
	return 0;
}


// Source: kprobes.c
// Lines 2332-2354
