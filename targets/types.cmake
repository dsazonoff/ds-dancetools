include_guard(GLOBAL)

# load_targets()

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl fmt)

end_target()
