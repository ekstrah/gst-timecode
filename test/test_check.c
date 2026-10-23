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

  /* status column: unreadable leaves the check alone, rejected is a failed check */
  tc_check_init (&c);
  assert (tc_check_status (&c, FALSE, 0, 0) == TC_STATUS_UNREADABLE);
  assert (!c.have);                          /* an unreadable frame sets no sec_offset */
  assert (tc_check_status (&c, TRUE, 100, 7) == TC_STATUS_OK);
  assert (tc_check_status (&c, FALSE, 100, 8) == TC_STATUS_UNREADABLE);
  assert (c.last_frame_nr == 7);
  assert (tc_check_status (&c, TRUE, 100, 7) == TC_STATUS_REJECTED);  /* repeat */
  assert (tc_check_status (&c, TRUE, 999, 9) == TC_STATUS_REJECTED);  /* wrong sec_offset */
  assert (tc_check_status (&c, TRUE, 100, 9) == TC_STATUS_OK);
  assert (g_str_equal (tc_status_name (TC_STATUS_OK), "ok"));
  assert (g_str_equal (tc_status_name (TC_STATUS_UNREADABLE), "unreadable"));
  assert (g_str_equal (tc_status_name (TC_STATUS_REJECTED), "rejected"));
  return 0;
}
