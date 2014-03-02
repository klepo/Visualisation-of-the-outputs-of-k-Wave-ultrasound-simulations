clear

%intput_filename = 'e:\FIT\MIT\3s\SEP\data\NewData\Data\kidney_input_data_sc_1_linear_single_source_sensor_mode_2.h5'
%output_filename = 'e:\FIT\MIT\3s\SEP\data\NewData\Data\kidney_output_data_sc_1_linear_single_source_sensor_mode_2.h5'

%intput_filename = '..\data\input_data_256_256_256_case_1.h5';
%output_filename = '..\data\output_data_256_256_256_case_1_no_compression.h5';

%intput_filename = '..\data\input_data_512_512_512_case_1.h5';
%output_filename = '..\data\output_data_512_512_512_case_1_no_compression.h5';

%intput_filename = '..\data\input_data_1024_1024_1024_case_1.h5';
%output_filename = '..\data\output_data_1024_1024_1024_case_1_no_compression.h5';

intput_filename = '..\data\input_data_128_128_128_case_1.h5';
output_filename = '..\data\output_data_128_128_128_case_1_no_compression.h5';

Nt = double(hdf5read(intput_filename, 'Nt'));
Nx = double(hdf5read(intput_filename, 'Nx'));
Ny = double(hdf5read(intput_filename, 'Ny'));
Nz = double(hdf5read(intput_filename, 'Nz'));

sensor_mask_data = h5read(intput_filename, '/sensor_mask_index');

%sensor_mask_data = sensor_mask_data(1:32768);

l = length(sensor_mask_data);
nt = 500;

out_data = h5read(output_filename, '/p', [1 nt 1], [l 1 1]);

%z = fix(sensor_mask / (X * Y))
%y = fix((mod(sensor_mask, (X * Y))) / X)
%x = mod(mod(sensor_mask, (X * Y)), X)

%-------------
% Downsampling sensor_mask_index

%{
smpl = 0.5;

Nx_d = ceil(Nx * smpl);
Ny_d = ceil(Ny * smpl);
Nz_d = ceil(Nz * smpl);

for i = 1:l
    sensor_mask = double(sensor_mask_data(i));
    z = ceil(sensor_mask / (Nx * Ny));
    y = ceil((mod(sensor_mask - 1, (Nx * Ny)) + 1) / Nx);
    x = mod(mod(sensor_mask - 1, (Nx * Ny)), Nx) + 1;
    z_d = ceil(z * Nz_d / Nz);
    y_d = ceil(y * Ny_d / Ny);
    x_d = ceil(x * Nx_d / Nx);
    sensor_mask_d = x_d + Nx_d * (y_d - 1) + (z_d - 1) * Nx_d * Ny_d;
    
    z_i = ceil(i / (Nx * Ny));
    y_i = ceil((mod(i - 1, (Nx * Ny)) + 1) / Nx);
    x_i = mod(mod(i - 1, (Nx * Ny)), Nx) + 1;
    z_d_i = ceil(z_i * Nz_d / Nz);
    y_d_i = ceil(y_i * Ny_d / Ny);
    x_d_i = ceil(x_i * Nx_d / Nx);
    i_d = x_d_i + Nx_d * (y_d_i - 1) + (z_d_i - 1) * Nx_d * Ny_d;
    
    %sensor_mask = double(sensor_mask_data(ceil(i * l / l_d)));
    sensor_mask_data_d(i_d) = sensor_mask_d;
    out_data_d(i_d) = out_data(i);
    %i
    %uint64(floor(sensor_mask*smpl*smpl))
end
%-------------
sensor_mask_data_d = sensor_mask_data_d(1:i_d);
out_data_d = out_data_d(1:i_d);
l = length(sensor_mask_data_d);
sensor_mask_data = sensor_mask_data_d;
out_data1 = out_data;
out_data = out_data_d;
Nx = Nx_d;
Ny = Ny_d;
Nz = Nz_d;
%}

%%

%sensor_mask_data(1) = sensor_mask_data(1) - 89600*2;

sensor_mask = double(sensor_mask_data(1));
min_z = ceil(sensor_mask / (Nx * Ny));
min_y = ceil((mod(sensor_mask - 1, (Nx * Ny)) + 1) / Nx);
min_x = mod(mod(sensor_mask - 1, (Nx * Ny)), Nx) + 1;

%sensor_mask = double(sensor_mask_data(l));
max_z = min_z;%ceil(sensor_mask / (Nx * Ny));
max_y = min_y;%ceil((mod(sensor_mask - 1, (Nx * Ny)) + 1) / Nx);
max_x = min_x;%mod(mod(sensor_mask - 1, (Nx * Ny)), Nx) + 1;

count = 1;
cnt = 0;

for i = 1:l
    cnt = cnt + 1;
    sensor_mask = double(sensor_mask_data(i));
    z = ceil(sensor_mask / (Nx * Ny));
    y = ceil((mod(sensor_mask - 1, (Nx * Ny)) + 1) / Nx);
    x = mod(mod(sensor_mask - 1, (Nx * Ny)), Nx) + 1;
    
    if z > max_z + 1
        masks(count, :) = [min_x, min_y, min_z, max_x, max_y, max_z, cnt - 1, i - 1];
        count = count + 1;
        cnt = 0;
        min_z = z;
        min_x = x;
        min_y = y;
        max_z = min_z;
        max_y = min_y;
        max_x = min_x;
        continue;
    end
    
    max_z = z;
    
    if min_x > x     
        min_x = x;
    end
    if min_y > y
        min_y = y;
    end
    if max_x < x
        max_x = x;
    end
    if max_y < y
        max_y = y;
    end
end

if count > 1
    masks(count, :) = [min_x, min_y, min_z, max_x, max_y, max_z, cnt, masks(count - 1, 8) + 1];
else
    masks(count, :) = [min_x, min_y, min_z, max_x, max_y, max_z, cnt, 1];    
end

%%

%r = reshape(out_data(:, 1), [128 128]);

for c = 1:count
    figure;
    
    mNx = masks(c, 4) - masks(c, 1) + 1;
    mNy = masks(c, 5) - masks(c, 2) + 1;
    mNz = masks(c, 6) - masks(c, 3) + 1;

    data3D(mNx, mNy, mNz) = zeros();

    
    for i = masks(c, 8):masks(c, 7)
        sensor_mask = double(sensor_mask_data(i));
        z = ceil(sensor_mask / (Nx * Ny));
        y = ceil((mod(sensor_mask - 1, (Nx * Ny)) + 1) / Nx);
        x = mod(mod(sensor_mask - 1, (Nx * Ny)), Nx) + 1;
        data3D(x - masks(c, 1) + 1, y - masks(c, 2) + 1, z - masks(c, 3) + 1) = out_data(i);
    end


    %data3D = permute(data3D, [2 3 1]);

    ma = max(max(out_data));
    mi = min(min(out_data));

    for z = 1:length(data3D(1, 1, :))
        data(Nx, Ny) = zeros();

        data = data3D(:, :, z);
        z
        M(z) = getframe;    

        %{
        data(10, 50) = ma/2;
        data(10, 51) = ma;

        data(10, 52) = ma/2;
        data(10, 53) = ma;

        data(10, 54) = ma/2;
        data(10, 55) = ma;

        data(11, 51) = ma/2;
        data(11, 50) = ma;

        data(11, 53) = ma/2;
        data(11, 52) = ma;

        data(11, 55) = ma/2;
        data(11, 54) = ma;
        %}
        %figure
        clims = [mi ma];
        imagesc(data, clims);
        axis equal;
        %truesize;
        %k = waitforbuttonpress;
        %pause(0.5); 
    end

end

%% 

    %{
    for y = 1:Ny
        for x = 1:Nx
            if x > 1 && y > 1 && x < 256 && y < 256
            %newData(ceil(x/2), ceil(y/2)) = (0.8 * data(x, y) + 0.025 * data(x - 1, y - 1) + 0.025 * data(x, y - 1) + 0.025 * data(x + 1, y - 1) + 0.05 * data(x - 1, y) + 0.025 * data(x + 1, y) + 0.025 * data(x - 1, y + 1) + 0.025 * data(x, y + 1) + 0.025 * data(x + 1, y + 1) );
            %newData(x, y) = ( 4 * data(x, y) + data(x - 1, y - 1) + data(x, y - 1) + data(x + 1, y - 1) + data(x - 1, y) + data(x + 1, y) + data(x - 1, y + 1) + data(x, y + 1) + data(x + 1, y + 1) ) / (9 + 4);
            %newData(x, y) = (0.6 * data(x, y) + 0.05 * data(x - 1, y - 1) + 0.05 * data(x, y - 1) + 0.05 * data(x + 1, y - 1) + 0.05 * data(x - 1, y) + 0.05 * data(x + 1, y) + 0.05 * data(x - 1, y + 1) + 0.05 * data(x, y + 1) + 0.05 * data(x + 1, y + 1) );
            newData(x, y) = data(x, y);
            end            
        end
    end
    
    %figure
    %clims = [0 ma];
    %imagesc(newData, clims);
    %axis equal;
    %truesize;
    
    s = 2;
    src_height = double(size(newData, 1));
    src_width = double(size(newData, 2));
    
    dest_height = 127;
    dest_width = 127;
    
    for y = 0:dest_height - 1
        for x = 0:dest_width - 1
            
            newX = x * ((src_width - 1) / (dest_width - 1));
            newY = y * ((src_height - 1) / (dest_height - 1));
            left = floor(newX);
            right = ceil(newX);
            top = floor(newY);
            bottom = ceil(newY);
            wX = ceil(newX) - newX;
            wY = ceil(newY) - newY;
            
            topL = newData(left + 1, top + 1) * wX + newData(right + 1, top + 1) * (1 - wX);
            
            bottomL = newData(left + 1, bottom + 1) * wX + newData(right + 1, bottom + 1) * (1 - wX);
            
            %newData2(ceil(x / s), ceil(y / s)) = newData(x, y);
            newData2(x+1, y+1) = topL * wY + bottomL  * (1 - wY);
            %newData2(x+1, y+1) = newData(round(newX) + 1, round(newY) + 1);
        end
    end
    
    figure
    clims = [0 ma];
    imagesc(newData2, clims);
    axis equal;
    %truesize;
    
    M(t) = getframe;
    %}

