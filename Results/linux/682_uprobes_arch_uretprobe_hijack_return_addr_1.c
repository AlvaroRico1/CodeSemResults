arch_uretprobe_hijack_return_addr(unsigned long trampoline_vaddr, struct pt_regs *regs)
{
	int rasize = sizeof_long(regs), nleft;
	unsigned long orig_ret_vaddr = 0; /* clear high bits for 32-bit apps */

	if (copy_from_user(&orig_ret_vaddr, (void __user *)regs->sp, rasize))
		return -1;

	/* check whether address has been already hijacked */
	if (orig_ret_vaddr == trampoline_vaddr)
		return orig_ret_vaddr;

	nleft = copy_to_user((void __user *)regs->sp, &trampoline_vaddr, rasize);
	if (likely(!nleft))
		return orig_ret_vaddr;

	if (nleft != rasize) {
		pr_err("return address clobbered: pid=%d, %%sp=%#lx, %%ip=%#lx\n",
		       current->pid, regs->sp, regs->ip);

		force_sig(SIGSEGV);
	}

	return -1;
}


// Source: uprobes.c
// Lines 1060-1084
