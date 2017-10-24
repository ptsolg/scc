import os

def get_files(dir, trail):
	return [os.path.join(dir, f) for f in os.listdir(dir) if f.endswith(trail)]

def get_filext(path):
	return path.split('.').pop()

def group_files_by_name(files):
	groups = []
	file_added = [False for f in files]

	for i in files:
		group = []
		ext = get_filext(i)

		for j in range(0, len(files)):
			if not file_added[j] and get_filext(files[j]) == ext:
				file_added[j] = True
				group.append(files[j])

		if group:
			groups.append(group)

	return groups

def find_file_with_extension(files, ext):
	for file in files:
		if file.endswith(ext):
			return file

	return ''