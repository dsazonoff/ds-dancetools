include_guard(GLOBAL)

# load_targets()

begin_target(EXECUTABLE)

populate_sources()
configure_packages_and_pch(DEPENDENCIES stl boost)
target_link_libraries(${target_name} PUBLIC ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

end_target()
