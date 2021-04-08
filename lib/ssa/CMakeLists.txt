add_scc_lib(ssa
	block.c
	builder.c
	const.c
	context.c
	instr.c
	llvm-pretty-print.c
	module.c
	pass.c
	pretty-print.c
	printer.c
	printer.h
	value.c

	DEPENDS
	tree

	INCLUDE
	${SCC_INC_DIR}
)
