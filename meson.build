project('gst-timecode', 'c', version : '1.0.0', license : 'LGPL')

# Link with the system ld: a linuxbrew ld earlier in PATH can't resolve system glib's deps (same rule as the CMake build).
if import('fs').exists('/usr/bin/ld')
  add_project_link_arguments('-B/usr/bin/', language : 'c')
endif

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')

gst_version = meson.project_version()

api_version = '1.0'

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])
gstvideo_dep = dependency('gstreamer-video-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-timecode')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer Timecode')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')
configure_file(output : 'config.h', configuration : cdata)

gsttimecodeoverlay_sources = [
  'src/gsttimecodeoverlay.c',
]

gsttimecodeoverlay = library('gsttimecodeoverlay',
  gsttimecodeoverlay_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstbase_dep, gstvideo_dep],
  install : true,
  install_dir : plugins_install_dir,
)

gsttimecodeparse_sources = [
  'src/gsttimecodeparse.c',
]

gsttimecodeparse = library('gsttimecodeparse',
  gsttimecodeparse_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstbase_dep, gstvideo_dep],
  install : true,
  install_dir : plugins_install_dir,
)

test_layout = executable('test_layout', 'test/test_layout.c',
  dependencies : [gst_dep])
test('layout', test_layout)

test_check = executable('test_check', 'test/test_check.c',
  dependencies : [gst_dep])
test('check', test_check)

test_read = executable('test_read', 'test/test_read.c',
  dependencies : [gst_dep, gstvideo_dep])
test('read', test_read)

# One log row per frame (no timestamp, too small, control), through the real elements of this build.
test_rows = executable('test_rows', 'test/test_rows.c',
  c_args : ['-DTC_BUILDDIR="' + meson.current_build_dir() + '"'],
  dependencies : [gst_dep, gstvideo_dep])
test('rows', test_rows, depends : [gsttimecodeparse, gsttimecodeoverlay])
