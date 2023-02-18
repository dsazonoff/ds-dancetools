include_guard(GLOBAL)

load_targets(types utils)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl boost fmt)

target_link_libraries(${target_name} PUBLIC utils types)

end_target()
