static void wc_data_dtor(void *ptr)
{
  struct ftp_wc *ftpwc = ptr;
  if(ftpwc && ftpwc->parser)
    Curl_ftp_parselist_data_free(&ftpwc->parser);
  free(ftpwc);
}


// Source: ftp.c
// Lines 3759-3765
