project('mindose', 'c',
  version : '1.0.0',
  default_options : [
    'warning_level=3',
    'c_std=c99',
    'buildtype=debug'
  ]
)

# Math library dependency
math_dep = meson.get_compiler('c').find_library('m', required : true)

# Include directories
inc_dirs = include_directories('.', 'kernel', 'fs', 'gui', 'process', 'apps', 'resource')

# Build subdirectories
subdir('kernel')
subdir('fs')
subdir('gui')
subdir('process')
subdir('resource')

# Main Mindose OS executable
mindose_exe = executable('mindose',
  'main.c',
  include_directories : inc_dirs,
  link_with : [kernel_lib, fs_lib, gui_lib, process_lib, resource_lib],
  dependencies : math_dep,
  install : true
)

# Build applications
subdir('apps')

# Custom targets for convenience
run_target('run',
  command : [mindose_exe, '--mem', '512M']
)

run_target('run-demo',
  command : [mindose_exe, '--mem', '256M', '--arch', 'x86']
)

# Summary
summary({
  'OS Executable' : mindose_exe.full_path(),
  'Applications' : app_executables.keys(),
  'Math Library' : math_dep.found() ? 'Found' : 'Not Found',
  'Build Type' : get_option('buildtype'),
  'C Standard' : get_option('c_std')
}, section : 'Mindose OS Build Configuration')
