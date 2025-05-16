static size_t trailers_read(char *buffer, size_t size, size_t nitems,
                            void *raw)
{
  struct Curl_easy *data = (struct Curl_easy *)raw;
  struct dynbuf *trailers_buf = &data->state.trailers_buf;
  size_t bytes_left = Curl_dyn_len(trailers_buf) -
    data->state.trailers_bytes_sent;
  size_t to_copy = (size*nitems < bytes_left) ? size*nitems : bytes_left;
  if(to_copy) {
    memcpy(buffer,
           Curl_dyn_ptr(trailers_buf) + data->state.trailers_bytes_sent,
           to_copy);
    data->state.trailers_bytes_sent += to_copy;
  }
  return to_copy;
}


// Source: transfer.c
// Lines 130-145
