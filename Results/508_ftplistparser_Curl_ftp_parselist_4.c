size_t Curl_ftp_parselist(char *buffer, size_t size, size_t nmemb,
                          void *connptr)
{
  size_t bufflen = size*nmemb;
  struct Curl_easy *data = (struct Curl_easy *)connptr;
  struct ftp_wc *ftpwc = data->wildcard.protdata;
  struct ftp_parselist_data *parser = ftpwc->parser;
  struct fileinfo *infop;
  struct curl_fileinfo *finfo;
  unsigned long i = 0;
  CURLcode result;
  size_t retsize = bufflen;

  if(parser->error) { /* error in previous call */
    /* scenario:
     * 1. call => OK..
     * 2. call => OUT_OF_MEMORY (or other error)
     * 3. (last) call => is skipped RIGHT HERE and the error is hadled later
     *    in wc_statemach()
     */
    goto fail;
  }

  if(parser->os_type == OS_TYPE_UNKNOWN && bufflen > 0) {
    /* considering info about FILE response format */
    parser->os_type = (buffer[0] >= '0' && buffer[0] <= '9') ?
                       OS_TYPE_WIN_NT : OS_TYPE_UNIX;
  }

  while(i < bufflen) { /* FSM */

    char c = buffer[i];
    if(!parser->file_data) { /* tmp file data is not allocated yet */
      parser->file_data = Curl_fileinfo_alloc();
      if(!parser->file_data) {
        parser->error = CURLE_OUT_OF_MEMORY;
        goto fail;
      }
      parser->file_data->info.b_data = malloc(FTP_BUFFER_ALLOCSIZE);
      if(!parser->file_data->info.b_data) {
        parser->error = CURLE_OUT_OF_MEMORY;
        goto fail;
      }
      parser->file_data->info.b_size = FTP_BUFFER_ALLOCSIZE;
      parser->item_offset = 0;
      parser->item_length = 0;
    }

    infop = parser->file_data;
    finfo = &infop->info;
    finfo->b_data[finfo->b_used++] = c;

    if(finfo->b_used >= finfo->b_size - 1) {
      /* if it is important, extend buffer space for file data */
      char *tmp = realloc(finfo->b_data,
                          finfo->b_size + FTP_BUFFER_ALLOCSIZE);
      if(tmp) {
        finfo->b_size += FTP_BUFFER_ALLOCSIZE;
        finfo->b_data = tmp;
      }
      else {
        Curl_fileinfo_cleanup(parser->file_data);
        parser->file_data = NULL;
        parser->error = CURLE_OUT_OF_MEMORY;
        goto fail;
      }
    }


// Source: ftplistparser.c
// Lines 326-392
