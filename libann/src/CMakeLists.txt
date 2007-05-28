include_directories(../include/ )

set(kritaann_LIB_SRCS ANN.cpp bd_tree.cpp kd_pr_search.cpp kd_util.cpp bd_fix_rad_search.cpp brute.cpp kd_search.cpp perf.cpp bd_pr_search.cpp kd_dump.cpp kd_split.cpp bd_search.cpp kd_fix_rad_search.cpp kd_tree.cpp )

add_library(kritaann SHARED ${kritaann_LIB_SRCS})
install(TARGETS kritaann DESTINATION ${LIB_INSTALL_DIR})
