make_cpp_dir (cpp_reader *pfile, const char *dir_name, int sysp)
{
  struct cpp_file_hash_entry *entry, **hash_slot;
  cpp_dir *dir;

  hash_slot = (struct cpp_file_hash_entry **)
    htab_find_slot_with_hash (pfile->dir_hash, dir_name,
			      htab_hash_string (dir_name),
			      INSERT);

  /* Have we already hashed this directory?  */
  for (entry = *hash_slot; entry; entry = entry->next)
    if (entry->start_dir == NULL)
      return entry->u.dir;

  dir = XCNEW (cpp_dir);
  dir->next = pfile->quote_include;
  dir->name = (char *) dir_name;
  dir->len = strlen (dir_name);
  dir->sysp = sysp;
  dir->construct = 0;

  /* Store this new result in the hash table.  */
  entry = new_file_hash_entry (pfile);
  entry->next = *hash_slot;
  entry->start_dir = NULL;
  entry->location = pfile->line_table->highest_location;
  entry->u.dir = dir;
  *hash_slot = entry;

  return dir;
}


// Source: files.c
// Lines 1157-1188
