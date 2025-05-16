bool tool_create_output_file(struct OutStruct *outs,
                             struct OperationConfig *config)
{
  struct GlobalConfig *global;
  FILE *file = NULL;
  DEBUGASSERT(outs);
  DEBUGASSERT(config);
  global = config->global;
  if(!outs->filename || !*outs->filename) {
    warnf(global, "Remote filename has no length!\n");
    return FALSE;
  }

  if(outs->is_cd_filename) {
    /* don't overwrite existing files */
    int fd;
    char *name = outs->filename;
    char *aname = NULL;
    if(config->output_dir) {
      aname = aprintf("%s/%s", config->output_dir, name);
      if(!aname) {
        errorf(global, "out of memory\n");
        return FALSE;
      }
      name = aname;
    }
    fd = open(name, O_CREAT | O_WRONLY | O_EXCL | O_BINARY, OPENMODE);
    if(fd != -1) {
      file = fdopen(fd, "wb");
      if(!file)
        close(fd);
    }
    free(aname);
  }
  else
    /* open file for writing */
    file = fopen(outs->filename, "wb");

  if(!file) {
    warnf(global, "Failed to create the file %s: %s\n", outs->filename,
          strerror(errno));
    return FALSE;
  }
  outs->s_isreg = TRUE;
  outs->fopened = TRUE;
  outs->stream = file;
  outs->bytes = 0;
  outs->init = 0;
  return TRUE;
}


// Source: tool_cb_wrt.c
// Lines 52-101
