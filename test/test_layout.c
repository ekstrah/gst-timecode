#include <assert.h>
#include "../src/timecode_layout.h"

int
main (void)
{
  tc_layout_t l;

  assert (tc_layout (1920, 1080, &l));
  assert (l.px == 16 && l.left == 512 && l.top == 56);

  guint sizes[][2] = { {854, 480}, {1280, 720} };
  for (int i = 0; i < 2; i++) {
    assert (tc_layout (sizes[i][0], sizes[i][1], &l));
    assert (l.px % 2 == 0 && l.px >= 4);
    assert (l.left + 64 * l.px <= sizes[i][0]);
    assert (l.top + 8 * l.px <= sizes[i][1]);
  }

  assert (tc_layout (480, 270, &l) && l.px == 4);
  assert (!tc_layout (478, 270, &l));
  assert (!tc_layout (320, 240, &l));
  return 0;
}
