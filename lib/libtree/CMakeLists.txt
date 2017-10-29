create_module(libtree
	FOLDER lib

	tree.h
	tree-type.h
	tree-type.c
	tree-decl.h
	tree-decl.c
	tree-context.h
	tree-context.c
	tree-stmt.h
	tree-stmt.c
	tree-common.h
	tree-common.c
	tree-exp.h
	tree-exp.c
	tree-module.h
	tree-module.c
	tree-target.h
	tree-target.c
	tree-eval.h
	tree-eval.c

	USING
	libscl

	INCLUDE
	${SCC_LIB_DIR}
)

export_module(libtree)