function kp = rpi_out(out)
%disp('RPI_OUT');
%disp(out);
kp = 1;
fid = fopen('rpi_out.txt', 'w+' );

persistent out_old
if isempty(out_old)
    out_old = 0;
end

fprintf(fid,'%d\n',out);
fclose(fid);

persistent r;
if isempty(r)
    r = raspberrypi;
end

if out_old ~= out
    %putFile(r,'rpi_out.txt','/home/pi/'); % time = ~ 420ms
    putFile(r,'rpi_out.txt','/mnt/RAMDisk/'); % time = ~ 420ms
end

out_old = out;

end