executable('gtk-v4l', 'gtk-v4l.c', link_with:gtkv4l_lib, install: true, dependencies: gtkv4l_deps, include_directories:library_inc)

po_dir = join_paths (meson.source_root (), 'po')

desktop = i18n.merge_file ('desktop',
			   input:'gtk-v4l.desktop.in',
			   output:'gtk-v4l.desktop',
			   install:true,
			   install_dir:get_option('datadir') / 'applications',
			   po_dir:po_dir,
			   type:'desktop')
