add_scc_lib(codegen
	codegen.c
	llvm-printer.c
	
	DEPENDS
	ssa

	INCLUDE
	${SCC_INC_DIR}
)