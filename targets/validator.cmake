include_guard(GLOBAL)

load_targets(db types)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES boost stl fmt)

target_link_libraries(${target_name} PUBLIC db types)

end_target()
