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

/* What timecodeparse's log says about a frame (its `status` column):
 *   ok          read and accepted by the integrity check;
 *   unreadable  a stripe line failed to read (check state untouched);
 *   rejected    read, but failed the integrity check (wrong sec_offset or frame_nr not increasing).
 * The latency column is -1 for unreadable and rejected frames, and also for an ok frame
 * whose latency is implausible (< 0 s or > 30 s, e.g. a file decoded later). */
typedef enum { TC_STATUS_OK, TC_STATUS_UNREADABLE, TC_STATUS_REJECTED } tc_status_t;

static inline tc_status_t
tc_check_status (tc_check_t * c, gboolean read_ok, guint64 sec_offset, guint64 frame_nr)
{
  if (!read_ok)
    return TC_STATUS_UNREADABLE;
  return tc_check_accept (c, sec_offset, frame_nr) ? TC_STATUS_OK : TC_STATUS_REJECTED;
}

static inline const char *
tc_status_name (tc_status_t s)
{
  switch (s) {
    case TC_STATUS_OK: return "ok";
    case TC_STATUS_UNREADABLE: return "unreadable";
    case TC_STATUS_REJECTED: return "rejected";
  }
  return "?";
}

#endif
