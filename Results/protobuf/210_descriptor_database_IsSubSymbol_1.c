bool IsSubSymbol(StringPiece sub_symbol, StringPiece super_symbol) {
  return sub_symbol == super_symbol ||
         (HasPrefixString(super_symbol, sub_symbol) &&
          super_symbol[sub_symbol.size()] == '.');
}

}  // namespace


// Source: descriptor_database.cc
// Lines 188-194
