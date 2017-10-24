function(create_executable name)
	cmake_parse_arguments(ARGS
		""
		"FOLDER"
		"USING;INCLUDE"
		${ARGN}
	)
	message(STATUS ${name})
	get_module_sources(module_sources ${ARGS_USING})
	get_module_includes(module_includes ${ARGS_USING})

	source_group("External Modules" FILES ${module_sources})
	source_group(sources FILES ${ARGS_UNPARSED_ARGUMENTS})

	include_directories(${module_includes} ${ARGS_INCLUDE})
	add_executable(${name} ${module_sources} ${ARGS_UNPARSED_ARGUMENTS})

	IF(ARGS_FOLDER)
		set_target_properties(${name} PROPERTIES FOLDER ${ARGS_FOLDER})
	ENDIF()

endfunction()
