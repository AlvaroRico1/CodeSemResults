bool test_if_subpart(ORDER *a, ORDER *b) {
  ORDER *first = a;
  ORDER *second = b;
  for (; first && second; first = first->next, second = second->next) {
    if ((*first->item)->eq(*second->item, true))
      continue;
    else
      return false;
  }
  // If the second argument is not subpart of the first return false
  if (second) return false;
  // Else assign the direction of the second argument to the first
  else {
    for (; a && b; a = a->next, b = b->next) a->direction = b->direction;
    return true;
  }
}


// Source: sql_select.cc
// Lines 3861-3877
