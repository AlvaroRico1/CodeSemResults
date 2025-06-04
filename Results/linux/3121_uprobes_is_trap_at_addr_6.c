static int is_trap_at_addr(struct mm_struct *mm, unsigned long vaddr)
{
	struct page *page;
	uprobe_opcode_t opcode;
	int result;

	pagefault_disable();
	result = __get_user(opcode, (uprobe_opcode_t __user *)vaddr);
	pagefault_enable();

	if (likely(result == 0))
		goto out;

	/*
	 * The NULL 'tsk' here ensures that any faults that occur here
	 * will not be accounted to the task.  'mm' *is* current->mm,
	 * but we treat this as a 'remote' access since it is
	 * essentially a kernel access to the memory.
	 */
	result = get_user_pages_remote(NULL, mm, vaddr, 1, FOLL_FORCE, &page,
			NULL, NULL);
	if (result < 0)
		return result;

	copy_from_page(page, vaddr, &opcode, UPROBE_SWBP_INSN_SIZE);
	put_page(page);
 out:
	/* This needs to return true for any variant of the trap insn */
	return is_trap_insn(&opcode);
}


// Source: uprobes.c
// Lines 1959-1988
