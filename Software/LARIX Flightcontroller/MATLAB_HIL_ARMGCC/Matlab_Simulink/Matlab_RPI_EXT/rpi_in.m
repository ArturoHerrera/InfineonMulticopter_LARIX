function kp = rpi_in(i)

persistent kp_old;
if isempty(kp_old)
    kp_old = 0;
end

persistent r;
if isempty(r)
    r = raspberrypi;
end

%getFile(r,'/home/pi/rpi_in.txt'); % time = ~ 150ms
getFile(r,'/mnt/RAMDisk/rpi_in.txt');
fid = fopen( 'rpi_in.txt', 'r' );

formatSpec = '%f, %f';
A = fscanf(fid,formatSpec);

if numel(A) == 2
    kp = A(1) - A(2);
else
    kp = kp_old;
end
%disp('RPI_IN');
%disp(kp);
fclose(fid);

kp_old = kp;

end