// Source: curl/lib/mime.c
// Lines 726-728
static void mime_file_free(void *ptr)
{
  curl_mimepart *part = (curl_mimepart *) ptr;

  if(part->fp) {
    fclose(part->fp);
    part->fp = NULL;
  }
  Curl_safefree(part->data);
  part->data = NULL;
}
