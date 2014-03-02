
data = load('..\data\NewData\dicom_data_trimmed.mat','-mat');

volume_data = data.volume_data;

imagesc(volume_data(:,:,150))
colormap(gray)

%clear