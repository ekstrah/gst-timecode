#ifndef TIMECODE_CHECK_H
#define TIMECODE_CHECK_H

#include <glib.h>

/* Integrity check for decoded timestamps: decoder concealment can copy an older
 * frame's stripes. A frame is plausible if sec_offset is the stream's and
 * frame_nr is strictly greater than the last accepted one. */
typedef struct { gboolean have; guint64 sec_offset; guint64 last_frame_nr; } tc_check_t;

static inline void
tc_check_init (tc_check_t * c)
{
  c->have = FALSE;
  c->sec_offset = 0;
  c->last_frame_nr = 0;
}

static inline gboolean
tc_check_accept (tc_check_t * c, guint64 sec_offset, guint64 frame_nr)
{
  if (!c->have) {
    c->have = TRUE;
    c->sec_offset = sec_offset;
  } else if (sec_offset != c->sec_offset || frame_nr <= c->last_frame_nr) {
    return FALSE;
  }
  c->last_frame_nr = frame_nr;
  return TRUE;
}

#endif
