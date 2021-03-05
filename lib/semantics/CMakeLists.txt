add_scc_lib(semantics
	errors.c
	errors.h
	sema.c
	sema-conv.c
	sema-decl.c
	sema-expr.c
	sema-init.c
	sema-stmt.c
	sema-type.c

	DEPENDS
	c-common
	tree

	INCLUDE
	${SCC_INC_DIR}
)
