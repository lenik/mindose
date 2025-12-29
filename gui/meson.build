gui_lib = static_library('gui',
  'gui.c',
  include_directories : inc_dirs,
  dependencies : math_dep
)

gui_standalone_lib = static_library('gui_standalone',
  'gui.c',
  include_directories : inc_dirs,
  c_args : ['-DSTANDALONE_APP'],
  dependencies : math_dep
)
