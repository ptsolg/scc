create_executable(scc
	main.c
	scc-env.h
	scc-env.c
	scc-args.c
	scc-args.h

	USING
	libc

	INCLUDE
	${SCC_LIB_DIR}
)

target_compile_definitions(scc PRIVATE _CRT_SECURE_NO_WARNINGS)