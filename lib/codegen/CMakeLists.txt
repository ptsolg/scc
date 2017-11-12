add_scc_lib(codegen
	codegen-llvm.c

	DEPENDS
	ssa

	INCLUDE
	${SCC_INC_DIR}
)
