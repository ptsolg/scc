import os, subprocess

def exec_ext(file):
	return file + '.exe' if os.name == 'nt' else file

def scc():
	return os.path.join(os.getcwd(), '..', '..', 'bin', exec_ext('scc'))

def execute(exec, args, stdout=None):
	return subprocess.call([exec] + args, stdout=stdout)

def lex(test, ex_args=[]):
	test.exit_code = execute(scc(), [test.input, '-dump-tokens', '-o', test.output] + ex_args)

def lex_errors(test, ex_args=[]):
	test.ignore_exit_code = True
	execute(scc(), [test.input, '-dump-tokens', '-log', test.output] + ex_args)

def parse(test, ex_args=[]):
	test.exit_code = execute(scc(), [test.input, '-fsyntax-only', '-dump-tree', '-o', test.output] + ex_args)

def parse_errors(test, ex_args=[]):
	test.ignore_exit_code = True
	execute(scc(), [test.input, '-fsyntax-only', '-dump-tree', '-log', test.output] + ex_args)

def ssaize(test, ex_args=[]):
	test.exit_code = execute(scc(), [test.input, '-S', '-emit-ssa', '-o', test.output] + ex_args)

def compile_and_run(test, check_exit_code_only=True, ex_args=[]):
	test.ignore = check_exit_code_only
	exe = os.path.join(test.output_dir, exec_ext('out'))
	if execute(scc(), [test.input, '-m32', '-o', exe] + ex_args) != 0:
		test.exit_code = 533
		return
		
	output = None if check_exit_code_only else open(test.output, 'w')
	test.exit_code = execute(exe, [], output)
