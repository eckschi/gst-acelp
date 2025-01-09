lib_args = common_args + []

# Define custom file extension (e.g., `.tab`) as header files
tab_files = files(
  'inv_sqrt.tab',
  'log2.tab',
  'pow2.tab',
  'window.tab',
  'lag_wind.tab',
  'grid.tab',
  'ener_qua.tab',
  'clsp_334.tab',
)

# sources used to compile this plug-in
plugin_sources = [
  'gstacelpenc.c',
  'gstacelpenc.h',
  'fbas_tet.c',
  'fexp_tet.c',
  'fmat_tet.c',
  'scod_tet.c',
  'sub_dsp.c',
  'sub_sc_d.c',
  'tetra_op.c',
  'structs.c',
  'c_source.h',
  'c_acelp.h',
  'c_structs.h'
#  'sdec_tet.c',

]

shlib = shared_library('gstacelpenc',
  plugin_sources,
  c_args : lib_args,
  include_directories: [configinc],
  dependencies : plugin_deps,
  gnu_symbol_visibility : 'hidden',
  install : true,
  install_dir : plugins_install_dir,
)

# Make this library usable as a Meson subproject.
gst_acelpenc_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with : shlib)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'gst-acelpenc',
  filebase : 'gst-acelpenc',
  description : 'GStreamer TETRA ACELP Plugin',
  subdirs : 'src',
  libraries : shlib,
  version : '"@0@"'.format(meson.project_version()),
)
