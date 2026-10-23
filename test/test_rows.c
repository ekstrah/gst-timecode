#undef NDEBUG /* the tests are asserts: keep them live in any build type */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <glib/gstdio.h>

/* timecodeparse writes exactly one log row per frame, whatever the frame:
 * the i-th row is the i-th frame out of the element (rig/ssim.py relies on it).
 * Pushes n flat gray I420 frames through `<pre> timecodeparse` and checks the
 * log has n rows, each with the expected status (7th column). */
static void
run (const char *dir, const char *name, const char *pre, int w, int h, int n,
    gboolean timestamps, const char *want)
{
  gchar *log = g_strdup_printf ("%s/%s.log", dir, name);
  gchar *desc = g_strdup_printf ("appsrc name=src format=time "
      "caps=video/x-raw,format=I420,width=%d,height=%d,framerate=30/1 ! %s "
      "timecodeparse location=\"%s\" ! fakesink sync=false", w, h, pre, log);
  GError *err = NULL;
  GstElement *pipe = gst_parse_launch (desc, &err);
  if (!pipe || err) {
    fprintf (stderr, "%s: %s\n", name, err ? err->message : "no pipeline");
    assert (0);
  }
  GstElement *src = gst_bin_get_by_name (GST_BIN (pipe), "src");
  assert (gst_element_set_state (pipe, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE);

  GstVideoInfo info;
  assert (gst_video_info_set_format (&info, GST_VIDEO_FORMAT_I420, w, h));
  for (int i = 0; i < n; i++) {
    GstBuffer *buf = gst_buffer_new_allocate (NULL, info.size, NULL);
    gst_buffer_memset (buf, 0, 128, info.size);       /* flat gray: no stripes */
    if (timestamps) {
      GST_BUFFER_PTS (buf) = i * GST_SECOND / 30;
      GST_BUFFER_DURATION (buf) = GST_SECOND / 30;
    }                                                 /* else PTS stays GST_CLOCK_TIME_NONE */
    GstFlowReturn ret;
    g_signal_emit_by_name (src, "push-buffer", buf, &ret);
    gst_buffer_unref (buf);
    assert (ret == GST_FLOW_OK);
  }
  GstFlowReturn ret;
  g_signal_emit_by_name (src, "end-of-stream", &ret);
  GstBus *bus = gst_element_get_bus (pipe);
  GstMessage *msg = gst_bus_timed_pop_filtered (bus, 10 * GST_SECOND,
      GST_MESSAGE_EOS | GST_MESSAGE_ERROR);
  assert (msg && GST_MESSAGE_TYPE (msg) == GST_MESSAGE_EOS);
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_object_unref (src);
  gst_element_set_state (pipe, GST_STATE_NULL);
  gst_object_unref (pipe);                            /* closes the log */

  gchar *text = NULL;
  assert (g_file_get_contents (log, &text, NULL, NULL));
  gchar **lines = g_strsplit (text, "\n", -1);
  int rows = 0;
  assert (g_str_has_suffix (lines[0], "\tstatus"));
  for (int i = 1; lines[i] && lines[i][0]; i++) {
    gchar **col = g_strsplit (lines[i], "\t", -1);
    assert (g_strv_length (col) == 7);
    if (strcmp (col[6], want) != 0) {
      fprintf (stderr, "%s: row %d status %s, want %s\n", name, i, col[6], want);
      assert (0);
    }
    g_strfreev (col);
    rows++;
  }
  if (rows != n) {
    fprintf (stderr, "%s: %d rows for %d frames\n", name, rows, n);
    assert (0);
  }
  printf ("%s: %d frames -> %d rows, all %s\n", name, n, rows, want);
  g_strfreev (lines);
  g_free (text);
  g_unlink (log);
  g_free (desc);
  g_free (log);
}

int
main (int argc, char **argv)
{
  gst_init (&argc, &argv);
  /* this build of the plugins, whatever else GST_PLUGIN_PATH holds */
  gst_registry_scan_path (gst_registry_get (), TC_BUILDDIR);
  gchar *dir = g_dir_make_tmp ("tc-rows-XXXXXX", NULL);
  assert (dir);
  gchar *olog = g_strdup_printf ("timecodeoverlay location=\"%s/overlay.log\" !", dir);

  run (dir, "no-timestamp", "", 640, 480, 5, FALSE, "unreadable");  /* big enough to read */
  run (dir, "too-small", "", 320, 240, 5, TRUE, "unreadable");     /* tc_layout: px 2 < 4 */
  run (dir, "too-small-no-timestamp", "", 64, 48, 3, FALSE, "unreadable");
  run (dir, "overlay-control", olog, 640, 480, 5, TRUE, "ok");

  gchar *ofile = g_strdup_printf ("%s/overlay.log", dir);
  g_unlink (ofile);
  g_free (ofile);
  g_rmdir (dir);
  g_free (olog);
  g_free (dir);
  return 0;
}
