include_guard(GLOBAL)

load_targets(db utils)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl boost)

target_link_libraries(${target_name} PUBLIC utils)

end_target()
