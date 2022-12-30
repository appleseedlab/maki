// Based off a bug found while analyzing gcc-12.1.0/gcc/wide-int.h

#define BITS_PER_UNIT (8)
#define MAX_BITSIZE_MODE_ANY_INT (64*BITS_PER_UNIT)
#define HOST_BITS_PER_WIDE_INT 64


#define WIDE_INT_MAX_ELTS \
  ((MAX_BITSIZE_MODE_ANY_INT + HOST_BITS_PER_WIDE_INT) / HOST_BITS_PER_WIDE_INT)

#define WIDE_INT_MAX_PRECISION (WIDE_INT_MAX_ELTS * HOST_BITS_PER_WIDE_INT)

template <typename T> class generic_wide_int;
template <int N> class fixed_wide_int_storage;

#define FIXED_WIDE_INT(N) \
  generic_wide_int < fixed_wide_int_storage <N> >

typedef FIXED_WIDE_INT (WIDE_INT_MAX_PRECISION) widest_int;

int main(void) { return 0; }