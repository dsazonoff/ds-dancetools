include_guard(GLOBAL)

# load_targets()

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl sqlite_orm)

end_target()
