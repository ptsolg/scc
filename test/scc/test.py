import test_manager, presets, os, sys

if __name__ == '__main__':
	dir = os.path.join(os.getcwd(), sys.argv[1] if len(sys.argv) > 1 else '')
	test_manager.TestManager().run(dir)