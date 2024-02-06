include_guard(GLOBAL)

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl fmt boost)

add_version_define(${target_name} DS_VERSION)

end_target()
