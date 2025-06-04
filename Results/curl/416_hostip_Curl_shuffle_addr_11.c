UNITTEST CURLcode Curl_shuffle_addr(struct Curl_easy *data,
                                    struct Curl_addrinfo **addr)
{
  CURLcode result = CURLE_OK;
  const int num_addrs = Curl_num_addresses(*addr);

  if(num_addrs > 1) {
    struct Curl_addrinfo **nodes;
    infof(data, "Shuffling %i addresses", num_addrs);

    nodes = malloc(num_addrs*sizeof(*nodes));
    if(nodes) {
      int i;
      unsigned int *rnd;
      const size_t rnd_size = num_addrs * sizeof(*rnd);

      /* build a plain array of Curl_addrinfo pointers */
      nodes[0] = *addr;
      for(i = 1; i < num_addrs; i++) {
        nodes[i] = nodes[i-1]->ai_next;
      }

      rnd = malloc(rnd_size);
      if(rnd) {
        /* Fisher-Yates shuffle */
        if(Curl_rand(data, (unsigned char *)rnd, rnd_size) == CURLE_OK) {
          struct Curl_addrinfo *swap_tmp;
          for(i = num_addrs - 1; i > 0; i--) {
            swap_tmp = nodes[rnd[i] % (i + 1)];
            nodes[rnd[i] % (i + 1)] = nodes[i];
            nodes[i] = swap_tmp;
          }

          /* relink list in the new order */
          for(i = 1; i < num_addrs; i++) {
            nodes[i-1]->ai_next = nodes[i];
          }

          nodes[num_addrs-1]->ai_next = NULL;
          *addr = nodes[0];
        }


// Source: hostip.c
// Lines 351-391
