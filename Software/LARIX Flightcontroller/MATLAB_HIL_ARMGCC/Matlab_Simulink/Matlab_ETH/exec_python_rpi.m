% FUNCTION_NAME - exec_python_rpi()
% This function opens a Shell which is required to execute 
% the Python Script.
%
%
% Hint: Enter the following commands in the Shell:
%
% $cd /Scheibtisch/Infineon
% $python3 main.py
%
% Syntax:  None, will be executed within the function eth_com()
%
% Inputs:
%    None
%
% Outputs:
%    None
%
% Other m-files required: None
% Subfunctions: None
% MAT-files required: none
%
% See also: eth_com()
%
% Author: Roman Beneder, Markus Lechner
% FH Technikum Wien - University of applied sciences
% Hoechsaedtplatz 6, 1210 Wien
% email: roman.beneder@technikum-wien.at
% email: markus.lechner@technikum-wien.at
% Website: https://embsys.technikum-wien.at/
% June 2017; Last revision: 20-Jun-2017

%------------- BEGIN CODE --------------
function exec_python_rpi()

persistent rasberryPI;

%Check if a raspberryPI Object is already existing.
%If not create one.
if isempty(rasberryPI)
    rasberryPI = raspberrypi;
end

%Opens a Shell to execute the Python-Script
openShell(rasberryPI)
end

%------------- END OF CODE --------------
%Please send suggestions for improvement of the above template header 
%to Roman Beneder and Markus Lechner..
%Your contribution towards improving this template will be acknowledged. 