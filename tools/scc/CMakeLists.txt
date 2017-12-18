add_scc_tool(scc
	main.c
	scc.h
	scc.c
	scc-args.c

	DEPENDS
	cc

	INCLUDE
	${SCC_INC_DIR}
)