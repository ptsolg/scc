set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT SCC_BIN_DIR)
	set(SCC_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
endif()

set(SCC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(SCC_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
