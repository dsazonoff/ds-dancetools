include_guard(GLOBAL)

load_targets(parser db export validator)

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl fmt)

target_link_libraries(${target_name} PRIVATE parser db export validator)

add_version_define(${target_name} DS_VERSION)

end_target()
