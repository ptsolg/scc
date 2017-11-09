import utils, os, subprocess, sys, time, re

class SCC:

	def __init__(self):
		name = 'scc'
		if os.name == 'nt':
			name = 'scc.exe'

		self.path = os.path.join(os.getcwd(), '..', '..', 'bin', name)

	def call(self, args):
		return subprocess.call([self.path] + args)

class TestEnv:

	def __init__(self, test_output):
		self.scc = SCC()
		self.test_output = test_output
		self.total = 0
		self.passed = 0
		self.failed = 0

	def run_test(self, test, ans, cmds):
		self.total += 1
		print('Testing ' + os.path.basename(test) + ': ', end='')
		open(self.test_output, 'w')
		self.scc.call(cmds)

		result = open(self.test_output, 'r').read()
		answer = open(ans, 'r').read()

		if re.sub('\s+', '', result) != re.sub('\s+', '', answer):
			print('FAILED\ngot:\n' + result + '\nexpected:\n' + answer + '\n')
			self.failed += 1
		else:
			print('PASSED')
			self.passed += 1

	def print_stats(self):
		print('\n\n-=====================================================================-')
		print('Ran {} tests in {}s.'.format(self.total, self.total_time))
		print('Passed {} Failed {}.'.format(self.passed, self.failed))

	def run_tests(self, dir):
		def add_subdirectory(sub):
			self.run_tests(os.path.join(dir, sub))

		config = os.path.join(dir, 'config.py')
		if not os.path.isfile(config):
			return

		_locals = locals()
		exec(open(config).read(), _locals)
		if 'cmds' not in _locals:
			return

		cmds = _locals['cmds']
		files = utils.get_files(dir, '.t')
		if not files:
			return

		print(dir)
		for test in files:
			ans = test.replace('.t', '.a')
			if not os.path.isfile(ans):
				ans = test

			_cmds = [cmd for cmd in cmds]
			for i in range(len(_cmds)):
				if _cmds[i] == '%test':
					_cmds[i] = test
				if _cmds[i] == '%output':
					_cmds[i] = self.test_output
				if _cmds[i] == '%dir':
					_cmds[i] = dir

			self.run_test(test, ans, _cmds)

	def run(self, root):
		start = time.time()
		self.run_tests(root)
		self.total_time = time.time() - start
		self.print_stats()

if __name__ == '__main__':
	dir = os.path.join(os.getcwd(),  sys.argv[1] if len(sys.argv) > 1 else '')
	TestEnv('out.txt').run(dir)
