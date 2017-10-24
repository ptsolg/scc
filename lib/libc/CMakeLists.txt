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
	c-parse-exp.h
	c-parse-exp.c
	c-parse-stmt.h
	c-parse-stmt.c
	c-parse-decl.h
	c-parse-decl.c
	c-prog.h
	c-prog.c
	c-prog-conversions.h
	c-prog-conversions.c
	c-prog-exp.h
	c-prog-exp.c
	c-prog-decl.h
	c-prog-decl.c
	c-prog-type.h
	c-prog-type.c
	c-prog-stmt.h
	c-prog-stmt.c
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