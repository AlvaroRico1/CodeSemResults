inline Symbol FileDescriptorTables::FindNestedSymbol(
    const void* parent, StringPiece name) const {
  Symbol::QueryKey query;
  query.name = name;
  query.parent = parent;
  auto it = symbols_by_parent_.find(query);
  return it == symbols_by_parent_.end() ? Symbol() : *it;
}


// Source: descriptor.cc
// Lines 1511-1518
