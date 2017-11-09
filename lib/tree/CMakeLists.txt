add_scc_lib(tree
	tree-type.c
	tree-decl.c
	tree-context.c
	tree-stmt.c
	tree-common.c
	tree-expr.c
	tree-module.c
	tree-target.c
	tree-eval.c

	DEPENDS
	scl

	INCLUDE
	${SCC_INC_DIR}
)
