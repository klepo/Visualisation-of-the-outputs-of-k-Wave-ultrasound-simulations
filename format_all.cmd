@echo on
clang-format -i ./k-wave-h5-helper/*.cpp ./k-wave-h5-helper/*.h
clang-format -i ./k-wave-h5-helper-parallel/*.cpp ./k-wave-h5-helper-parallel/*.h
clang-format -i ./k-wave-h5-processing/*.cpp ./k-wave-h5-processing/*.h
clang-format -i ./k-wave-h5-processing/processing/*.cpp ./k-wave-h5-processing/processing/*.h
clang-format -i ./k-wave-h5-processing-gui/*.cpp ./k-wave-h5-processing-gui/*.h
clang-format -i ./k-wave-h5-processing-parallel/*.cpp ./k-wave-h5-processing-parallel/*.h
clang-format -i ./k-wave-h5-visualizer/*.cpp ./k-wave-h5-visualizer/*.h
clang-format -i ./k-wave-h5-visualizer/widgets/*.cpp ./k-wave-h5-visualizer/widgets/*.h
clang-format -i ./rewrite-ram/*.cpp ./rewrite-ram/*.h
clang-format -i ./tests/vector3d/*.cpp ./tests/vector3d/*.h




