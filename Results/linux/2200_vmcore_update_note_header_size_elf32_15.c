static int __init update_note_header_size_elf32(const Elf32_Ehdr *ehdr_ptr)
{
	int i, rc=0;
	Elf32_Phdr *phdr_ptr;
	Elf32_Nhdr *nhdr_ptr;

	phdr_ptr = (Elf32_Phdr *)(ehdr_ptr + 1);
	for (i = 0; i < ehdr_ptr->e_phnum; i++, phdr_ptr++) {
		void *notes_section;
		u64 offset, max_sz, sz, real_sz = 0;
		if (phdr_ptr->p_type != PT_NOTE)
			continue;
		max_sz = phdr_ptr->p_memsz;
		offset = phdr_ptr->p_offset;
		notes_section = kmalloc(max_sz, GFP_KERNEL);
		if (!notes_section)
			return -ENOMEM;
		rc = elfcorehdr_read_notes(notes_section, max_sz, &offset);
		if (rc < 0) {
			kfree(notes_section);
			return rc;
		}
		nhdr_ptr = notes_section;
		while (nhdr_ptr->n_namesz != 0) {
			sz = sizeof(Elf32_Nhdr) +
				(((u64)nhdr_ptr->n_namesz + 3) & ~3) +
				(((u64)nhdr_ptr->n_descsz + 3) & ~3);
			if ((real_sz + sz) > max_sz) {
				pr_warn("Warning: Exceeded p_memsz, dropping PT_NOTE entry n_namesz=0x%x, n_descsz=0x%x\n",
					nhdr_ptr->n_namesz, nhdr_ptr->n_descsz);
				break;
			}
			real_sz += sz;
			nhdr_ptr = (Elf32_Nhdr*)((char*)nhdr_ptr + sz);
		}
		kfree(notes_section);
		phdr_ptr->p_memsz = real_sz;
		if (real_sz == 0) {
			pr_warn("Warning: Zero PT_NOTE entries found\n");
		}
	}

	return 0;
}


// Source: vmcore.c
// Lines 894-937
