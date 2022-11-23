include_guard(GLOBAL)

load_targets(parser db export)

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl)

target_link_libraries(${target_name} PRIVATE parser db export)

end_target()
