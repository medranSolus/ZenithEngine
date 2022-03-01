// Maybe instead of doing if() test just take the flag value out (bit shift) and then multiply the value of equation by it (avoiding warp/wavefront divergence in work group)
static const uint FLAG_USE_SPECULAR_POWER_ALPHA = 1;
static const uint FLAG_USE_TEXTURE = 2;
static const uint FLAG_USE_NORMAL = 4;
static const uint FLAG_USE_SPECULAR = 8;
static const uint FLAG_USE_PARALLAX = 16;