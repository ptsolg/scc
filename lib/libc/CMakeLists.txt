create_module(libc
	FOLDER lib

	c-common.h
	c-reswords.h
	c-token-kind.h
	c-limits.h
	c-error.h
	c-error.c
	c-token.h
	c-token.c
	c-source.h
	c-source.c
	c-lexer.h
	c-lexer.c
	c-tree.c
	c-tree.h
	c-parser.h
	c-parser.c
	c-parse-module.h
	c-parse-module.c
	c-parse-expr.h
	c-parse-expr.c
	c-parse-stmt.h
	c-parse-stmt.c
	c-parse-decl.h
	c-parse-decl.c
	c-sema.h
	c-sema.c
	c-sema-conv.h
	c-sema-conv.c
	c-sema-expr.h
	c-sema-expr.c
	c-sema-decl.h
	c-sema-decl.c
	c-sema-type.h
	c-sema-type.c
	c-sema-stmt.h
	c-sema-stmt.c
	c-info.h
	c-info.c
	c-env.h
	c-env.c
	c-preprocessor.h
	c-preprocessor.c
	c-printer.h
	c-printer.c

	USING
	libtree

	INCLUDE
	${SCC_LIB_DIR}
)

export_module(libc)