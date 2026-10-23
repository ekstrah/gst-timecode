#undef NDEBUG /* the tests are asserts: keep them live in any build type */
#include <assert.h>
#include "../src/timecode_check.h"

int
main (void)
{
  tc_check_t c;
  tc_check_init (&c);

  assert (tc_check_accept (&c, 100, 0));    /* first frame */
  assert (tc_check_accept (&c, 100, 1));
  assert (tc_check_accept (&c, 100, 2));
  assert (tc_check_accept (&c, 100, 3));
  assert (!tc_check_accept (&c, 100, 3));   /* repeat */
  assert (!tc_check_accept (&c, 100, 2));   /* older */
  assert (tc_check_accept (&c, 100, 4));    /* rejections did not poison state */
  assert (!tc_check_accept (&c, 999, 5));   /* wrong sec_offset */
  assert (tc_check_accept (&c, 100, 5));
  return 0;
}
