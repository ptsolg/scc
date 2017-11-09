function(add_scc_target_sources name)
	set(cd ${CMAKE_CURRENT_LIST_DIR})
	foreach(src ${ARGN})
		string(CONCAT full ${cd}/${src})
		target_sources(${name} PRIVATE ${full})
	endforeach()
endfunction()

function(add_scc_target name)
	cmake_parse_arguments(ARGS
		"LIB"
		"FOLDER"
		"DEPENDS;INCLUDE;LINK;"
		${ARGN}
	)

	if (ARGS_LIB)
		add_library(${name} STATIC ${ARGS_UNPARSED_ARGUMENTS})
	else()
		add_executable(${name} ${ARGS_UNPARSED_ARGUMENTS})
	endif()

	set_target_properties(${name} PROPERTIES FOLDER ${ARGS_FOLDER})

	include_directories(${ARGS_INCLUDE})
	if (ARGS_DEPENDS)
		add_dependencies(${name} ${ARGS_DEPENDS})
		target_link_libraries(${name} ${ARGS_DEPENDS})
	endif()

	if (ARGS_LINK)
		target_link_libraries(${name} ${ARGS_LINK})
	endif()

	target_compile_definitions(${name}
		PRIVATE _CRT_SECURE_NO_WARNINGS
	)
	
	set_target_properties(${name}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${SCC_BIN_DIR}
		LIBRARY_OUTPUT_DIRECTORY ${SCC_BIN_DIR}
		RUNTIME_OUTPUT_DIRECTORY ${SCC_BIN_DIR}
	)
	
	foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${cfg} cfg)
		set_target_properties(${name}
			PROPERTIES
			ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${SCC_BIN_DIR}
			LIBRARY_OUTPUT_DIRECTORY_${cfg} ${SCC_BIN_DIR}
			RUNTIME_OUTPUT_DIRECTORY_${cfg} ${SCC_BIN_DIR}
		)
	endforeach()

endfunction()

function(add_scc_lib name)
	add_scc_target(${name}
		LIB
		FOLDER Libraries
		${ARGN}
	)
endfunction()

function(add_scc_tool name)
	add_scc_target(${name}
		FOLDER Tools
		${ARGN}
	)
endfunction()