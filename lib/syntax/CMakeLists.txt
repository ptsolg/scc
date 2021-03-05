add_scc_lib(syntax
	builtin-type.c
	builtin-type.h
	errors.c
	errors.h
	parse-decl.c
	parse-expr.c
	parser.c
	parse-stmt.c
	printer.c

	DEPENDS
	lex
	semantics

	INCLUDE
	${SCC_INC_DIR}
)
