from sys import argv

outname = '../run_all_sorts'

class SortingAlgorithm:
	def __init__(self, app_name, delay, nitems):
		self.app_name = app_name
		self.delay = delay
		self.nitems = nitems if nitems > 0 else 512

alg_list = [
	SortingAlgorithm("selection_sort", 2, 128),
	SortingAlgorithm("insertion_sort", 2, 128),
	SortingAlgorithm("binary_insertion_sort", 2, 128),
	SortingAlgorithm("quick_sort", 10, -1),
	SortingAlgorithm("quick_sort_random_pivot", 10, -1),
	SortingAlgorithm("merge_sort", 10, -1),
	SortingAlgorithm("heap_sort", 10, -1),
	SortingAlgorithm("radix_sort_lsd", 10, -1),
	SortingAlgorithm("radix_sort_lsd_alt", 10, -1),
	SortingAlgorithm("radix_sort_msd", 10, -1),
	SortingAlgorithm("intro_sort", 10, -1),
	SortingAlgorithm("tim_sort_3_min_run", 10, -1),
	SortingAlgorithm("tim_sort", 10, -1),
	SortingAlgorithm("shell_sort", 5, -1),
	SortingAlgorithm("shell_sort_ciura", 5, -1),
	SortingAlgorithm("bubble_sort", 2, 128),
	SortingAlgorithm("optimised_bubble_sort", 2, 128),
	SortingAlgorithm("cocktail_shaker_sort", 2, 128),
	SortingAlgorithm("optimised_cocktail_shaker_sort", 2, 128),
	SortingAlgorithm("gnome_sort", 2, 128),
	SortingAlgorithm("bitonic_sort", 5, -1),
	SortingAlgorithm("circle_sort", 5, -1),
	SortingAlgorithm("cycle_sort", 2, 128),
	SortingAlgorithm("stooge_sort", 0, 128),
	SortingAlgorithm("slow_sort", 0, 64),
	SortingAlgorithm("bogo_sort", 0, 8),
	SortingAlgorithm("bozo_sort", 0, 8),
	SortingAlgorithm("better_bozo_sort", 0, 64)
	# SortingAlgorithm("intro_bozo_sort", 0, 256)
]

shuffle_patterns = [
	'random',
	'reverse',
	'noshuffle',
	'slightshuffle',
	'shuffletail',
	'shufflehead',
	'mergepass',
	'quarters',
	'reversemergepass',
	'mountain',
	'radixpass',
	'binarysearchtree',
	'heapified',
	'halfreverse',
	'reverseevens',
	'shuffleodds',
	'reversequarters',
	'circlepass',
	'sortedheapinput'
]

def create_bash():
	with open(f'{outname}.bash', 'wb') as f:
		for i in alg_list:
			for j in shuffle_patterns:
				f.write(f'./{i.app_name} {i.delay} {i.nitems} {j} chord;\n'.encode())
				f.write(f'sleep 1;\n'.encode())

def create_bat():
	with open(f'{outname}.bat', 'wb') as f:
		f.write('@echo off\n'.encode())
		for i in alg_list:
			for j in shuffle_patterns:
				f.write(f'{i.app_name}.exe {i.delay} {i.nitems} {j}\n'.encode())
				f.write(f'timeout /t 1 > NUL\n'.encode())

def main(argc, argv):
	print("NOTE: If you're using windows, use python gen_run_script.py bat to generate bat script instead of bash script.")
	use_bat = False
	if argc > 1:
		use_bat = argv[1].lower() == 'bat'
	if use_bat:
		create_bat()
	else:
		create_bash()

if __name__ == '__main__':
	main(len(argv), argv)