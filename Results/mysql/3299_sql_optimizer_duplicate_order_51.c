static bool duplicate_order(const ORDER *first_order,
                            const ORDER *possible_dup) {
  const ORDER *order;
  for (order = first_order; order; order = order->next) {
    if (order == possible_dup) {
      // all expressions preceding possible_dup have been checked.
      return false;
    } else {
      const Item *it1 = order->item[0]->real_item();
      const Item *it2 = possible_dup->item[0]->real_item();

      if (it1->eq(it2, false)) return true;
    }
  }
  return false;
}


// Source: sql_optimizer.cc
// Lines 9688-9703
