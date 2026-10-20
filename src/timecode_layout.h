#ifndef TIMECODE_LAYOUT_H
#define TIMECODE_LAYOUT_H

#include <glib.h>

/* Stripe layout shared by timecodeoverlay and timecodeparse.
 * Line L covers rows top + L*px .. top + (L+1)*px - 1,
 * bit b covers columns left + b*px .. left + (b+1)*px - 1.
 * Lines 5, 6, 7 are used. 1920x1080 gives {16, 512, 56}. */
typedef struct { guint px, left, top; } tc_layout_t;

static inline gboolean
tc_layout (guint width, guint height, tc_layout_t * out)
{
  out->px = (width / 120) & ~1u;
  out->left = (width * 4 / 15) & ~1u;
  out->top = (56 * height / 1080) & ~1u;
  return out->px >= 4 &&
      out->left + 64 * out->px <= width &&
      out->top + 8 * out->px <= height;
}

#endif
