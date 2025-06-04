static int __init copy_notes_elf32(const Elf32_Ehdr *ehdr_ptr, char *notes_buf)
{
	int i, rc=0;
	Elf32_Phdr *phdr_ptr;

	phdr_ptr = (Elf32_Phdr*)(ehdr_ptr + 1);

	for (i = 0; i < ehdr_ptr->e_phnum; i++, phdr_ptr++) {
		u64 offset;
		if (phdr_ptr->p_type != PT_NOTE)
			continue;
		offset = phdr_ptr->p_offset;
		rc = elfcorehdr_read_notes(notes_buf, phdr_ptr->p_memsz,
					   &offset);
		if (rc < 0)
			return rc;
		notes_buf += phdr_ptr->p_memsz;
	}

	return 0;
}


// Source: vmcore.c
// Lines 992-1012
