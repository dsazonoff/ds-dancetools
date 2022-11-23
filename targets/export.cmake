include_guard(GLOBAL)

load_targets(db types utils)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl)

target_link_libraries(${target_name} PUBLIC db types utils)

end_target()
