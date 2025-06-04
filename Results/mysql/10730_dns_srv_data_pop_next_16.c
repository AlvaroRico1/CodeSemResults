  bool pop_next(std::string &host, unsigned &port) {
    if (data_.empty()) return true;

    dns_entry_list_t &list = data_.begin()->second;
    assert(!list.empty());

    unsigned long sum = 0;
    for (Dns_entry &elt : list) elt.add_weight_sum(sum);

    unsigned long draw = (std::rand() * 1UL * sum) / RAND_MAX;

    dns_entry_list_t::const_iterator iter = list.cbegin();
    while (iter->weight_sum() < draw) iter++;
    assert(iter != list.end());

    host = iter->host();
    port = iter->port();

    list.erase(iter);
    if (list.empty()) data_.erase(data_.begin());
    return false;
  }
};


// Source: dns_srv_data.h
// Lines 94-116
