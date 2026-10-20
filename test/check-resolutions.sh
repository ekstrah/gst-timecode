#!/bin/bash
# Run overlay -> x264 -> parse at several sizes; every frame must read back a plausible latency.
# Needs GST_PLUGIN_PATH to include builddir (run inside gst-env.py).
cd "$(dirname "$0")/.." || exit 1
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
rc=0
for size in 854x480 1280x720 1920x1080; do
 for run in 1 2 3; do
  w=${size%x*}; h=${size#*x}
  gst-launch-1.0 -q filesrc location=../video/train_30.mp4 ! qtdemux ! h264parse ! avdec_h264 \
    ! identity eos-after=300 ! videoscale ! "video/x-raw,format=I420,width=$w,height=$h" \
    ! timecodeoverlay location="$tmp/o.log" \
    ! x264enc pass=cbr speed-preset=ultrafast tune=zerolatency key-int-max=30 bitrate=2000 \
    ! h264parse ! avdec_h264 ! timecodeparse location="$tmp/p.log" ! fakesink sync=false || rc=1
  # parse log columns: ts, frame_nr, latency (us); first line is the header
  read -r n m < <(tail -n +2 "$tmp/p.log" | awk -F'\t' '{n++} $3>=0 && $3<=1000000 {m++} END {print n+0, m+0}')
  echo "${w}x${h} run $run: parsed $n, plausible $m"
  [ "$n" = 314 ] && [ "$m" = 314 ] || rc=1
 done
done
exit $rc
