# Copyright (C) 2020 Daniel Maksymow
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

interactive_tests = [
  'cartesian',
  'polynomals_test',
  'scale_test',
]

interactive_test_dependencies = [
  common_dependencies,
  dependency('gtk+-3.0'),
  dependency('libxml-2.0'),
  dependency('libsoup-2.4'),
  compiler.find_library('m', required: true),
]

if get_option('gstreamer')
  interactive_test_dependencies += gstreamer_dependency + gstreamer_audio_dependency + gstreamer_video_dependency + gstreamer_app_dependency + gstreamer_pbutils_dependency
endif

foreach interactive_test : interactive_tests
  source = ['@0@.c'.format(interactive_test)]
  executable(
    interactive_test,
    source,
    c_args: compiler_flags,
    include_directories: [includes],
    dependencies: [interactive_test_dependencies],
    link_with: [
      libwidget,
    ]
  )
endforeach

#TODO: move this to environment variables
srcdir_def = '-DSRCDIR="@0@"'.format(meson.source_root())

common_test_dependencies = [
  common_dependencies,
  cunit_dependency,
]

subdir('X')
subdir('audio')
subdir('file')
subdir('lib')
subdir('object')
subdir('plugin')
subdir('server')
subdir('thread')
