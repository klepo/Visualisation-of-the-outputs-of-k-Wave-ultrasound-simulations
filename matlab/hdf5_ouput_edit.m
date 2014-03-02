
%output_filename = '..\data\output_data_128_128_128_case_1_no_compression.h5';
%output_filename = '..\data\output_data_256_256_256_case_1_no_compression.h5';
output_filename = '..\data\output_data_512_512_512_case_1_no_compression.h5';
%output_filename = '..\data\output_data_1024_1024_1024_case_1_no_compression.h5';

out_data = hdf5read(output_filename, 'p_max_all');

size(out_data)

h5create(output_filename,'/p_max_all32', size(out_data), 'Datatype', 'single', 'ChunkSize', size(out_data)/32);

h5write(output_filename, '/p_max_all32', out_data);

h5disp(output_filename);