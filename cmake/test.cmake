set(LIBTEST_DIR ${SCC_TEST_DIR}/libtest)

function(gentest dir)
	execute_process(COMMAND python ${LIBTEST_DIR}/gen.py ${dir})
endfunction()