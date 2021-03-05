add_scc_lib(lex
	charset.c
	errors.c
	errors.h
	lexer.c
	macro.c
	macro.h
	macro-lexer.c
	misc.c
	numeric-literal.c
	numeric-literal.h
	pragma.c
	preprocessor.c
	preprocessor-directive.c
	preprocessor-directive.h
	preprocessor-expression.c
	preprocessor-expression.h
	preprocessor-lexer.c
	reswords.c
	token.c
	token-lexer.c

	DEPENDS
	c-common

	INCLUDE
	${SCC_INC_DIR}
)
