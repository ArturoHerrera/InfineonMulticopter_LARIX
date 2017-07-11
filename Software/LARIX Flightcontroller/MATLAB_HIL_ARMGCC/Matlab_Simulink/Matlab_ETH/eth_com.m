% FUNCTION_NAME - eth_com()
% This function creates a TCP/IP Server which enables a data communication
% to the Raspberry-Pi. Furthermore, a Shell is opened to execute the 
% Python Script via SSH.
%
%
% Hint: The total required time for the data transmission(send /receive)
% amounts to approximately 20ms.
%
% Syntax:  eth_com()
%
% Inputs:
%    None
%
% Outputs:
%    None
%
% Other m-files required: exec_python_rpi.m
% Subfunctions: cleanUpfct()
% MAT-files required: none
%
% See also: exec_python_rpi()
%
% Author: Roman Beneder, Markus Lechner
% FH Technikum Wien - University of applied sciences
% Hoechsaedtplatz 6, 1210 Wien
% email: roman.beneder@technikum-wien.at
% email: markus.lechner@technikum-wien.at
% Website: https://embsys.technikum-wien.at/
% June 2017; Last revision: 20-Jun-2017

%------------- BEGIN CODE --------------
function eth_com()
%Pre-Defined Parameters, which describe the controlling 
%behavior. This parameters can be adjusted individually
Kp = 5;
a0 = pi / 180 * 4;

persistent tcpip_server

%Create a clean-up Object to ensure a proper socket 
%close proces
cleanupObj = onCleanup(@cleanUpfct);

%Create a TCP/IP Server Object to communicate with the Raspberry Pi
tcpip_server = tcpip('0.0.0.0', 30000, 'NetworkRole', 'server');

%Calll the separate .m file to open a Shell which enables
%the communication to the Raspberry Pi via SSH
exec_python_rpi

%Open the TCP/IP Server - Blocking Function call
fopen(tcpip_server);
disp('Server created!')

while true
    %Read commandos from the socket
    client_cmd = fscanf(tcpip_server,'%s');
    
    %Receive Raspi-Client init message
    if(strcmp(client_cmd, 'Raspi Client'))
        disp('Raspi-Client connected')
        continue;
    end
    
    %According to ETH Protocol, receive sensor data
    sensor_data = strsplit(client_cmd, ',');

    %Sensor Data need to be formated(string to double)
    sensor_data = str2double(sensor_data);
    
    %Extract the matrix according to the r and y parameter
    r_sense = sensor_data(1);
    y_sense = sensor_data(2);
    
    %Calculate the correction value
    ctr_err = r_sense - y_sense;
    
    disp('Controller-Parameter')
    %Calculate the controlled parameter
    y = ctr_err * a0 * Kp
    
    %According to ETH Protocol, receive commando CtrlReq
    client_cmd = fscanf(tcpip_server,'%s');
    
    %Verify if the Raspi-Client requests controller data
    if(strcmp(client_cmd, 'CtrlReq'))
        %Format amd transmit the controller data to
        %the Raspberry Pi
        fwrite(tcpip_server, num2str(y));
    end
end
    %Function to close the socket properly
    function cleanUpfct()
        fclose(tcpip_server);
        disp('Server Closed')
    end
end
%------------- END OF CODE --------------
%Please send suggestions for improvement of the above template header 
%to Roman Beneder and Markus Lechner..
%Your contribution towards improving this template will be acknowledged. 