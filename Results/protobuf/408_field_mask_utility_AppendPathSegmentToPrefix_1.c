std::string AppendPathSegmentToPrefix(StringPiece prefix,
                                      StringPiece segment) {
  if (prefix.empty()) {
    return std::string(segment);
  }
  if (segment.empty()) {
    return std::string(prefix);
  }
  // If the segment is a map key, appends it to the prefix without the ".".
  if (HasPrefixString(segment, "[\"")) {
    return StrCat(prefix, segment);
  }
  return StrCat(prefix, ".", segment);
}

}  // namespace


// Source: field_mask_utility.cc
// Lines 49-64
