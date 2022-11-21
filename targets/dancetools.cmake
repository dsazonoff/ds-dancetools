include_guard(GLOBAL)

# load_targets()

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(stl)

end_target()
