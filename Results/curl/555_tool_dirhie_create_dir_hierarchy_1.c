CURLcode create_dir_hierarchy(const char *outfile, FILE *errors)
{
  char *tempdir;
  char *tempdir2;
  char *outdup;
  char *dirbuildup;
  CURLcode result = CURLE_OK;
  size_t outlen;

  outlen = strlen(outfile);
  outdup = strdup(outfile);
  if(!outdup)
    return CURLE_OUT_OF_MEMORY;

  dirbuildup = malloc(outlen + 1);
  if(!dirbuildup) {
    Curl_safefree(outdup);
    return CURLE_OUT_OF_MEMORY;
  }
  dirbuildup[0] = '\0';

  /* Allow strtok() here since this isn't used threaded */
  /* !checksrc! disable BANNEDFUNC 2 */
  tempdir = strtok(outdup, PATH_DELIMITERS);

  while(tempdir != NULL) {
    bool skip = false;
    tempdir2 = strtok(NULL, PATH_DELIMITERS);
    /* since strtok returns a token for the last word even
       if not ending with DIR_CHAR, we need to prune it */
    if(tempdir2 != NULL) {
      size_t dlen = strlen(dirbuildup);
      if(dlen)
        msnprintf(&dirbuildup[dlen], outlen - dlen, "%s%s", DIR_CHAR, tempdir);
      else {
        if(outdup == tempdir) {
#if defined(MSDOS) || defined(WIN32)
          /* Skip creating a drive's current directory.
             It may seem as though that would harmlessly fail but it could be
             a corner case if X: did not exist, since we would be creating it
             erroneously.
             eg if outfile is X:\foo\bar\filename then don't mkdir X:
             This logic takes into account unsupported drives !:, 1:, etc. */
          char *p = strchr(tempdir, ':');
          if(p && !p[1])
            skip = true;
#endif
          /* the output string doesn't start with a separator */
          strcpy(dirbuildup, tempdir);
        }
        else
          msnprintf(dirbuildup, outlen, "%s%s", DIR_CHAR, tempdir);
      }
      /* Create directory. Ignore access denied error to allow traversal. */
      if(!skip && (-1 == mkdir(dirbuildup, (mode_t)0000750)) &&
         (errno != EACCES) && (errno != EEXIST)) {
        show_dir_errno(errors, dirbuildup);
        result = CURLE_WRITE_ERROR;
        break; /* get out of loop */
      }
    }
    tempdir = tempdir2;
  }

  Curl_safefree(dirbuildup);
  Curl_safefree(outdup);

  return result;
}


// Source: tool_dirhie.c
// Lines 102-170
