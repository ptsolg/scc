add_scc_lib(ssa
	ssa-builder.c
	ssa-printer.c
	ssa-context.c
	ssa-instr.c
	ssa-value.c
	ssa-block.c
	ssa-branch.c
	ssa-module.c
	ssaizer.c
	ssa-ir.c

	DEPENDS
	tree

	INCLUDE
	${SCC_INC_DIR}
)
