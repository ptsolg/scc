import utils, os, time, re

class TestCase:
	def __init__(self, input, answer, output, output_dir, test_dir):
		self.input = input
		self.output = output
		self.output_dir = output_dir
		self.cd = test_dir
		self.answer = answer
		self.ignore = False
		self.exit_code = 123
		self.ignore_exit_code = False

class TestManager:

	def __init__(self, hide_passed=False):
		cd = os.getcwd()
		self.test_output_dir = os.path.join(cd, '__tmp__')
		self.test_output = os.path.join(self.test_output_dir, 'out.txt')
		os.makedirs(self.test_output_dir, exist_ok=True)
		open(self.test_output, 'w').close()
		self.total = 0
		self.passed = 0
		self.failed = 0
		self.hide_passed = hide_passed
		self.presets = __import__('presets')

	def test_failed(self, test, msg):
		self.failed += 1
		print(f'Test {test}: FAILED\n{msg}\n')

	def test_passed(self, test):
		self.passed += 1
		if not self.hide_passed:
			print(f'Test {test}: PASSED')

	def check_test(self, test):
		self.total += 1
		test_name = os.path.basename(test.input)

		if not test.ignore_exit_code and test.exit_code != 0:
			self.test_failed(test_name, f'exit code = {test.exit_code}')
			return

		if test.ignore:
			self.test_passed(test_name)
			return

		result = open(self.test_output, 'r').read()
		answer = open(test.answer, 'r').read()

		if re.sub('\s+', '', result) != re.sub('\s+', '', answer):
			self.test_failed(test_name, f'got:\n{result}\nexpected:\n{answer}')
		else:
			self.test_passed(test_name)

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

		test_ext = '.t'
		ans_ext = '.a'
		if 'test_ext' in config_scope:
			test_ext = config_scope['test_ext']
		if 'ans_ext' in config_scope:
			ans_ext = config_scope['ans_ext']

		files = utils.get_files(dir, test_ext)
		if not files:
			return

		print(dir)
		for test in files:
			answer = test.replace(test_ext, ans_ext)
			if not os.path.isfile(answer):
				answer = test

			test_case = TestCase(test, answer, self.test_output, self.test_output_dir, dir)
			config_scope['run'](test_case)
			self.check_test(test_case)

	def run(self, root):
		start = time.time()
		self.run_tests(root)
		self.total_time = time.time() - start
		self.print_stats()
		return self
