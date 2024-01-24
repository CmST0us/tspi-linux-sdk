project(
    'libdrm-cursor',
    'c',
    version : '1.0.0',
    meson_version : '>=0.47.0',
    default_options: ['buildtype=release', 'warning_level=3'],
)

pkgconfig = import('pkgconfig')

libdrm_dep = dependency('libdrm', version : '>= 2.4.0')
libthreads_dep = dependency('threads')
libgbm_dep = dependency('gbm')
libegl_dep = dependency('egl')
libgles_dep = dependency('glesv2')

libdrm_cursor_deps = [
    libdrm_dep,
    libthreads_dep,
    libgbm_dep,
    libegl_dep,
    libgles_dep,
]

libdrm_cursor_srcs = [
    'drm_cursor.c',
    'drm_egl.c',
]

add_project_arguments(['-D_GNU_SOURCE'], language: 'c')

if get_option('prefer-afbc')
    message('Prefer ARM AFBC modifier')
    add_project_arguments(['-DPREFER_AFBC_MODIFIER'], language: 'c')
endif

libdrm_cursor = shared_library(
    'drm-cursor',
    libdrm_cursor_srcs,
    dependencies : libdrm_cursor_deps,
    version : meson.project_version(),
    install : true,
)

pkgconfig.generate(
    libraries : 'libdrm-cursor',
    filebase : 'libdrm-cursor',
    name : 'libdrm-cursor',
    version : meson.project_version(),
    description : 'A hook of drm cursor APIs to fake cursor plane.',
)

configure_file(
    input : 'drm-cursor.conf.sample',
    output : 'drm-cursor.conf',
    install_dir : get_option('sysconfdir'),
    copy : true,
)

executable(
    'cursor-test',
    [ libdrm_cursor_srcs, 'test.c' ],
    dependencies : libdrm_cursor_deps,
    install : get_option('install-test'),
)
