import os, subprocess

def exec_ext(file):
	return file + '.exe' if os.name == 'nt' else file

def scc():
	return os.path.join(os.getcwd(), '..', '..', 'bin', exec_ext('scc'))

def execute(exec, args, stdout=None):
	return subprocess.call([exec] + args, stdout=stdout)

def lex(test, ex_args=[]):
	execute(scc(), [test.input, '-dump-tokens', '-o', test.output] + ex_args)

def lex_errors(test, ex_args=[]):
	execute(scc(), [test.input, '-dump-tokens', '-log', test.output] + ex_args)

def parse(test, ex_args=[]):
	execute(scc(), [test.input, '-fsyntax-only', '-dump-tree', '-o', test.output] + ex_args)

def parse_errors(test, ex_args=[]):
	execute(scc(), [test.input, '-fsyntax-only', '-dump-tree', '-log', test.output] + ex_args)

def ssaize(test, ex_args=[]):
	execute(scc(), [test.input, '-S', '-emit-ssa', '-o', test.output] + ex_args)

def compile_and_run(test, ex_args=[]):
	exe = os.path.join(test.output_dir, exec_ext('out'))
	execute(scc(), [test.input, '-o', exe] + ex_args)
	execute(exe, [], stdout=open(test.output, 'w'))
