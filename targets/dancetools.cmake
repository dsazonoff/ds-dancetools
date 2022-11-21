include_guard(GLOBAL)

load_targets(parser)

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl)

target_link_libraries(${target_name} PRIVATE parser)

end_target()
