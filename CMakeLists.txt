# TODO: check for an installed version of libann and use it if possible instead
add_subdirectory(libann)

include_directories(${CMAKE_CURRENT_BINARY_DIR} libann/include ${CMAKE_SOURCE_DIR}/lib)

set(kritaAnalogies_PART_SRCS Analogies.cpp FeatureSearch.cpp Feature.cpp )

kde3_automoc(${kritaAnalogies_PART_SRCS})

kde3_add_kpart(kritaAnalogies ${kritaAnalogies_PART_SRCS})

target_link_libraries(kritaAnalogies ${KRITAUI_LIBRARY} kritaann kritapluginscommon)

install(TARGETS kritaAnalogies  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kritaAnalogies.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
