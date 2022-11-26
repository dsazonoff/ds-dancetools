include_guard(GLOBAL)

load_targets(types utils)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl sqlite_orm)

target_link_libraries(${target_name} PUBLIC types utils)

end_target()
