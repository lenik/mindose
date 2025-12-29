resource_lib = static_library('resource',
  'resource.c',
  include_directories : inc_dirs,
  dependencies : math_dep
)

resource_standalone_lib = static_library('resource_standalone',
  'resource.c',
  include_directories : inc_dirs,
  c_args : ['-DSTANDALONE_APP'],
  dependencies : math_dep
)
