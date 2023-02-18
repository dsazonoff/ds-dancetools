include_guard(GLOBAL)

begin_target(LIBRARY)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl boost fmt)

end_target()
