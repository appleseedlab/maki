// Based off a bug found while analyzing gcc-12.1.0/gcc/wide-int.h
// Currently crashes


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

#define GTY(x)  /* nothing - marker for gengtype */

// Note: In the original code, in the file hwint.h, the definition of
// HOST_WIDE_INT is nested inside a static conditional.
// I've removed that conditional because it seems that the conditions
// it is based on are hardcoded to always be true
#   define HOST_WIDE_INT long

template <bool SE, bool HDP = true>
class wide_int_ref_storage;


typedef generic_wide_int <wide_int_ref_storage <false> > wide_int_ref;


enum signop {
  SIGNED,
  UNSIGNED
};

template <int N>
class GTY(()) fixed_wide_int_storage
{
private:
  HOST_WIDE_INT val[(N + HOST_BITS_PER_WIDE_INT + 1) / HOST_BITS_PER_WIDE_INT];
  unsigned int len;

public:
  fixed_wide_int_storage ();
  template <typename T>
  fixed_wide_int_storage (const T &);

  /* The standard generic_wide_int storage methods.  */
  unsigned int get_precision () const;
  const HOST_WIDE_INT *get_val () const;
  unsigned int get_len () const;
  HOST_WIDE_INT *write_val ();
  void set_len (unsigned int, bool = false);

  static FIXED_WIDE_INT (N) from (const wide_int_ref &, signop);
  static FIXED_WIDE_INT (N) from_array (const HOST_WIDE_INT *, unsigned int,
					bool = true);
};

int main(void) { return 0; }