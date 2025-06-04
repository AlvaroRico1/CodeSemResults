void truncate_partition_filename(MEM_ROOT *root, const char **path) {
  if (*path) {
    const char *last_slash = strrchr(*path, FN_LIBCHAR);

#ifdef _WIN32
    if (!last_slash) last_slash = strrchr(*path, FN_LIBCHAR2);
#endif

    if (last_slash) {
      /* Look for a partition-type filename */
      for (const char *pound = strchr(last_slash, '#'); pound;
           pound = strchr(pound + 1, '#')) {
        if ((pound[1] == 'P' || pound[1] == 'p') && pound[2] == '#') {
          if (root == nullptr) {
            char *p = const_cast<char *>(last_slash);
            *p = '\0';
          } else {
            *path = strmake_root(root, *path, last_slash - *path);
          }


// Source: sql_partition.cc
// Lines 1793-1811
