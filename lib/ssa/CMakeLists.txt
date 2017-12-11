add_scc_lib(ssa
	ssa-builder.c
	ssa-printer.c
	ssa-context.c
	ssa-instr.c
	ssa-value.c
	ssa-block.c
	ssa-branch.c
	ssa-module.c
	ssa-function.c
	ssaizer.c
	ssaize-expr.c
	ssaize-stmt.c
	ssaize-decl.c
	ssa-opt.c
	ssa-pass.c
	
	DEPENDS
	tree

	INCLUDE
	${SCC_INC_DIR}
)
