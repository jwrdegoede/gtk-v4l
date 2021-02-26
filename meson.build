project('gtk-v4l', 'c', version:'0.5.0', meson_version:'>=0.50')

i18n = import ('i18n')

v4l2_dep = dependency('libv4l2', version:'>=0.6')
gtk_dep = dependency ('gtk+-3.0', version:'>=3.12')
gudev_dep = dependency('gudev-1.0', version:'>=151')

gtkv4l_deps = [v4l2_dep, gtk_dep, gudev_dep]

subdir('po')
subdir('lib')
subdir('src')
