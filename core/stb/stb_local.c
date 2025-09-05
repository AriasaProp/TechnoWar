int stbi_enabled_flags = 0;

void stbi_enable_flags(int flags) {
  stbi_enabled_flags |= flags;
}
void stbi_disable_flags(int flags) {
  stbi_enabled_flags &= ~flags;
}