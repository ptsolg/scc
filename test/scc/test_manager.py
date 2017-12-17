import utils, os, time, re

class TestCase:
	def __init__(self, input, output, output_dir, test_dir):
		self.input = input
		self.output = output
		self.output_dir = output_dir
		self.cd = test_dir

class TestManager:

	def __init__(self):
		cd = os.getcwd()
		self.test_output_dir = os.path.join(cd, '__tmp__')
		self.test_output = os.path.join(self.test_output_dir, 'out.txt')
		os.makedirs(self.test_output_dir, exist_ok=True)
		open(self.test_output, 'w').close()
		self.total = 0
		self.passed = 0
		self.failed = 0
		self.presets = __import__('presets')

	def check_test(self, test, answer):
		print('Testing ' + os.path.basename(test) + ': ', end='')
		result = open(self.test_output, 'r').read()
		answer = open(answer, 'r').read()

		self.total += 1
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

		config_file = os.path.join(dir, 'config.py')
		if not os.path.isfile(config_file):
			return

		presets = self.presets
		config_scope = locals()
		exec(open(config_file).read(), config_scope)
		if 'run' not in config_scope:
			return

		files = utils.get_files(dir, '.t')
		if not files:
			return

		print(dir)
		for test in files:
			answer = test.replace('.t', '.a')
			if not os.path.isfile(answer):
				answer = test

			config_scope['run'](TestCase(test, self.test_output, self.test_output_dir, dir))
			self.check_test(test, answer)

	def run(self, root):
		start = time.time()
		self.run_tests(root)
		self.total_time = time.time() - start
		self.print_stats()
		return self
