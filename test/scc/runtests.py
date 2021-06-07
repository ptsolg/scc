import test_manager, presets, os, sys, argparse

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Run tests')
	parser.add_argument('--hide-passed', action='store_true', help='Hide passed tests')
	args, unknown = parser.parse_known_args()
	dir = os.path.join(os.getcwd(), unknown[0] if len(unknown) > 0 else '')
	exit(test_manager.TestManager(hide_passed=args.hide_passed).run(dir).failed)
