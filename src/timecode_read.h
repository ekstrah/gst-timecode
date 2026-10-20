#ifndef TIMECODE_READ_H
#define TIMECODE_READ_H

#include <gst/video/video.h>
#include "timecode_layout.h"

/* Read stripe line `line` (see tc_layout_t). Returns TRUE and sets *out only if
 * every bit is clearly black (<= 50) or white (>= 200) and the chroma is near
 * neutral; otherwise returns FALSE and leaves *out untouched. */
static inline gboolean
tc_read_line (int line, const tc_layout_t * l, const GstVideoFrame * frame, guint64 * out)
{
  guint64 timestamp = 0;

  const guchar *y = frame->data[0];
  const guchar *u = y + frame->info.offset[1];
  const guchar *v = y + frame->info.offset[2];
  guint px = l->px;

  /* Decide each bit from the mean of the block interior (1-pixel border dropped),
   * which survives codec ringing at block edges better than a single sample. */
  guint row0 = l->top + line * px;
  guint cb = (px / 2 >= 3) ? 1 : 0;   /* chroma border */
  guint u_sum = 0;
  guint v_sum = 0;
  for (int bit = 0; bit < 64; bit++) {
    guint col0 = l->left + bit * px;
    guint y_sum = 0, y_n = 0, c_sum_u = 0, c_sum_v = 0, c_n = 0;
    for (guint r = row0 + 1; r < row0 + px - 1; r++)
      for (guint c = col0 + 1; c < col0 + px - 1; c++) {
        y_sum += y[r * frame->info.stride[0] + c];
        y_n++;
      }
    for (guint r = row0 / 2 + cb; r < row0 / 2 + px / 2 - cb; r++)
      for (guint c = col0 / 2 + cb; c < col0 / 2 + px / 2 - cb; c++) {
        c_sum_u += u[r * frame->info.stride[1] + c];
        c_sum_v += v[r * frame->info.stride[2] + c];
        c_n++;
      }
    guint y_value = y_sum / y_n;
    u_sum += c_sum_u / c_n;
    v_sum += c_sum_v / c_n;
    if (y_value > 50 && y_value < 200)
      return FALSE;

    timestamp |= (y_value >= 200) ? (guint64) 1 << (63 - bit) : 0;
  }

  if ((u_sum / 64 < 100) || (u_sum / 64 > 156) || (v_sum / 64 < 100) || (v_sum / 64 > 156))
    return FALSE;

  *out = timestamp;
  return TRUE;
}

#endif
