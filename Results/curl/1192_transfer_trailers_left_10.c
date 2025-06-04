static size_t trailers_left(void *raw)
{
  struct Curl_easy *data = (struct Curl_easy *)raw;
  struct dynbuf *trailers_buf = &data->state.trailers_buf;
  return Curl_dyn_len(trailers_buf) - data->state.trailers_bytes_sent;
}


// Source: transfer.c
// Lines 147-152
