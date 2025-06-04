static int can_optimize(unsigned long paddr)
{
	unsigned long addr, size = 0, offset = 0;
	struct insn insn;
	kprobe_opcode_t buf[MAX_INSN_SIZE];

	/* Lookup symbol including addr */
	if (!kallsyms_lookup_size_offset(paddr, &size, &offset))
		return 0;

	/*
	 * Do not optimize in the entry code due to the unstable
	 * stack handling and registers setup.
	 */
	if (((paddr >= (unsigned long)__entry_text_start) &&
	     (paddr <  (unsigned long)__entry_text_end)) ||
	    ((paddr >= (unsigned long)__irqentry_text_start) &&
	     (paddr <  (unsigned long)__irqentry_text_end)))
		return 0;

	/* Check there is enough space for a relative jump. */
	if (size - offset < RELATIVEJUMP_SIZE)
		return 0;

	/* Decode instructions */
	addr = paddr - offset;
	while (addr < paddr - offset + size) { /* Decode until function end */
		unsigned long recovered_insn;
		if (search_exception_tables(addr))
			/*
			 * Since some fixup code will jumps into this function,
			 * we can't optimize kprobe in this function.
			 */
			return 0;
		recovered_insn = recover_probed_instruction(buf, addr);
		if (!recovered_insn)
			return 0;
		kernel_insn_init(&insn, (void *)recovered_insn, MAX_INSN_SIZE);
		insn_get_length(&insn);
		/* Another subsystem puts a breakpoint */
		if (insn.opcode.bytes[0] == BREAKPOINT_INSTRUCTION)
			return 0;
		/* Recover address */
		insn.kaddr = (void *)addr;
		insn.next_byte = (void *)(addr + insn.length);
		/* Check any instructions don't jump into target */
		if (insn_is_indirect_jump(&insn) ||
		    insn_jump_into_range(&insn, paddr + INT3_SIZE,
					 RELATIVE_ADDR_SIZE))
			return 0;
		addr += insn.length;
	}

	return 1;
}


// Source: opt.c
// Lines 253-307
