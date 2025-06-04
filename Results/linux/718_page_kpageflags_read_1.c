static ssize_t kpageflags_read(struct file *file, char __user *buf,
			     size_t count, loff_t *ppos)
{
	u64 __user *out = (u64 __user *)buf;
	struct page *ppage;
	unsigned long src = *ppos;
	unsigned long pfn;
	ssize_t ret = 0;

	pfn = src / KPMSIZE;
	count = min_t(unsigned long, count, (max_pfn * KPMSIZE) - src);
	if (src & KPMMASK || count & KPMMASK)
		return -EINVAL;

	while (count > 0) {
		if (pfn_valid(pfn))
			ppage = pfn_to_page(pfn);
		else
			ppage = NULL;

		if (put_user(stable_page_flags(ppage), out)) {
			ret = -EFAULT;
			break;
		}

		pfn++;
		out++;
		count -= KPMSIZE;

		cond_resched();
	}

	*ppos += (char __user *)out - buf;
	if (!ret)
		ret = (char __user *)out - buf;
	return ret;
}


// Source: page.c
// Lines 204-240
