add_scc_lib(c
	c-error.c
	c-token.c
	c-source.c
	c-lexer.c
	c-tree.c
	c-parser.c
	c-parse-module.c
	c-parse-expr.c
	c-parse-stmt.c
	c-parse-decl.c
	c-sema.c
	c-sema-conv.c
	c-sema-expr.c
	c-sema-decl.c
	c-sema-type.c
	c-sema-stmt.c
	c-info.c
	c-env.c
	c-preprocessor.c
	c-printer.c

	DEPENDS
	tree

	INCLUDE
	${SCC_INC_DIR}
)