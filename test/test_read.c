#undef NDEBUG /* the tests are asserts: keep them live in any build type */
#include <assert.h>
#include <string.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include "../src/timecode_layout.h"
#include "../src/timecode_read.h"

/* Draw `value` on `line`; a bit listed in grey_bit (0..63; -1 none, -2 all) gets luma 128. */
static void
draw (GstVideoFrame * f, const tc_layout_t * l, int line, guint64 value, int grey_bit, guchar grey)
{
  guchar *y = f->data[0];
  guchar *u = y + f->info.offset[1];
  guchar *v = y + f->info.offset[2];
  for (guint r = 0; r < l->px; r++) {
    guint row = l->top + line * l->px + r;
    for (int bit = 0; bit < 64; bit++) {
      guchar c = (bit == grey_bit || grey_bit == -2) ? grey : ((value >> (63 - bit)) & 1) * 255;
      memset (y + row * f->info.stride[0] + l->left + bit * l->px, c, l->px);
    }
    memset (u + (row / 2) * f->info.stride[1] + l->left / 2, 128, l->px / 2 * 64);
    memset (v + (row / 2) * f->info.stride[2] + l->left / 2, 128, l->px / 2 * 64);
  }
}

static void
check_size (guint w, guint h)
{
  GstVideoInfo info;
  gst_video_info_set_format (&info, GST_VIDEO_FORMAT_I420, w, h);
  GstBuffer *buf = gst_buffer_new_allocate (NULL, info.size, NULL);
  GstVideoFrame f;
  assert (gst_video_frame_map (&f, &info, buf, GST_MAP_READWRITE));
  tc_layout_t l;
  assert (tc_layout (w, h, &l));
  guint64 out = 77;

  draw (&f, &l, 5, 0, -1, 0);
  assert (tc_read_line (5, &l, &f, &out) && out == 0);

  out = 77;
  draw (&f, &l, 6, 0x0102030405060708ULL, -1, 0);
  assert (tc_read_line (6, &l, &f, &out) && out == 0x0102030405060708ULL);

  out = 77;
  draw (&f, &l, 7, 0, -2, 128);   /* whole line grey */
  assert (!tc_read_line (7, &l, &f, &out) && out == 77);

  draw (&f, &l, 7, 0x0102030405060708ULL, 13, 128);
  assert (!tc_read_line (7, &l, &f, &out) && out == 77);

  /* valid luma, chroma 200 (outside 100..156) */
  draw (&f, &l, 7, 0x0102030405060708ULL, -1, 0);
  {
    guchar *u = f.data[0] + f.info.offset[1];
    for (guint r = 0; r < l.px; r++) {
      guint row = l.top + 7 * l.px + r;
      memset (u + (row / 2) * f.info.stride[1] + l.left / 2, 200, l.px / 2 * 64);
    }
  }
  out = 77;
  assert (!tc_read_line (7, &l, &f, &out) && out == 77);

  gst_video_frame_unmap (&f);
  gst_buffer_unref (buf);
}

int
main (int argc, char **argv)
{
  gst_init (&argc, &argv);
  check_size (854, 480);
  check_size (1920, 1080);
  return 0;
}
