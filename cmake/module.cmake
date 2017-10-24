function(strip_headers result)
	set(list ${ARGN})
	set(sources "")
	foreach(source IN LISTS list)
		IF(source MATCHES "\.(c|cpp)$")
			set(sources ${sources} ${source})
		ENDIF()
	endforeach()
	set(${result} ${sources} PARENT_SCOPE)
endfunction()

function(add_dir_to_sources result dir)
	set(list ${ARGN})
	foreach(source IN LISTS list)
		string(CONCAT source ${dir} "/" ${source})
		set(sources ${sources} ${source})
	endforeach()
	set(${result} ${sources} PARENT_SCOPE)
endfunction()

function(append_module_property module property)
	string(CONCAT propname ${module} ${property})
	set(list ${ARGN})
	foreach(val IN LISTS list)
		set_property(GLOBAL APPEND PROPERTY ${propname} ${val})
	endforeach()
endfunction()

function(set_module_property module property val)
	string(CONCAT propname ${module} ${property})
	set_property(GLOBAL PROPERTY ${propname} ${val})
endfunction()

function(get_module_property var module property)
	string(CONCAT propname ${module} ${property})
	get_property(prop GLOBAL PROPERTY ${propname})
	set(${var} ${prop} PARENT_SCOPE)
endfunction()

function(create_module name)
	cmake_parse_arguments(ARGS
		""
		"FOLDER"
		"USING;INCLUDE"
		${ARGN}
	)

	set_module_property(${name} submodules "")
	set_module_property(${name} folder ${ARGS_FOLDER})

	foreach(dir IN LISTS ARGS_INCLUDE)
		append_module_property(${name} include ${dir})
	endforeach()

	foreach(module IN LISTS ARGS_USING)
		append_module_property(${name} using ${module})
	endforeach()

	add_dir_to_sources(sources ${CMAKE_CURRENT_SOURCE_DIR} ${ARGS_UNPARSED_ARGUMENTS})
	append_module_property(${name} sources ${sources})

	source_group(sources FILES ${ARGS_UNPARSED_ARGUMENTS})
endfunction()

function(create_submodule parent name)
	append_module_property(${parent} submodules ${name})
	set_module_property(${name} submodules "")
	add_dir_to_sources(sources ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
	append_module_property(${name} sources ${sources})
endfunction()

function(get_module_includes result)
	set(modules ${ARGN})
	set(includes "")

	foreach(module IN LISTS modules)
		get_module_property(dirs ${module} include)
		get_module_property(using ${module} using)
		get_module_includes(using_dirs ${using})
		set(includes ${includes} ${dirs} ${using_dirs})
	endforeach()

	set(${result} ${includes} PARENT_SCOPE)
endfunction()

function(include_module_directories)
	get_module_includes(includes ${ARGN})
	include_directories(${includes})
endfunction()

function(create_module_source_hierarchy result module parent_group)
	string(CONCAT group_name ${parent_group}\\${module})
	IF(parent_group STREQUAL "")
		set(group_name "sources")
	ENDIF()

	get_module_property(sources ${module} sources)
	source_group(${group_name} FILES ${sources})

	get_module_property(submodules ${module} submodules)
	foreach(submodule IN LISTS submodules)
		create_module_source_hierarchy(sub_sources ${submodule} ${group_name})
		set(sources ${sources} ${sub_sources})
	endforeach()

	set(${result} ${sources} PARENT_SCOPE)
endfunction()

function(export_module name)
	include_module_directories(${name})
	create_module_source_hierarchy(sources ${name} "")
	add_library(${name} OBJECT ${sources})
	get_module_property(folder ${name} folder)
	set_target_properties(${name} PROPERTIES FOLDER ${folder})
endfunction()

function(get_module_sources result)
	set(modules ${ARGN})
	set(all "")

	foreach(module IN LISTS modules)
		get_module_property(sources ${module} sources)
		strip_headers(sources ${sources})

		get_module_property(submodules ${module} submodules)
		get_module_property(using ${module} using)
		get_module_sources(submodule_sources ${submodules})
		get_module_sources(using_sources ${using})

		set(all ${all} ${sources} ${submodule_sources} ${using_sources})
	endforeach()

	set(${result} ${all} PARENT_SCOPE)
endfunction()

