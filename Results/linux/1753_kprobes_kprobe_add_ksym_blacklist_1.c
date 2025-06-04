int kprobe_add_ksym_blacklist(unsigned long entry)
{
	struct kprobe_blacklist_entry *ent;
	unsigned long offset = 0, size = 0;

	if (!kernel_text_address(entry) ||
	    !kallsyms_lookup_size_offset(entry, &size, &offset))
		return -EINVAL;

	ent = kmalloc(sizeof(*ent), GFP_KERNEL);
	if (!ent)
		return -ENOMEM;
	ent->start_addr = entry;
	ent->end_addr = entry + size;
	INIT_LIST_HEAD(&ent->list);
	list_add_tail(&ent->list, &kprobe_blacklist);

	return (int)size;
}


// Source: kprobes.c
// Lines 2101-2119
