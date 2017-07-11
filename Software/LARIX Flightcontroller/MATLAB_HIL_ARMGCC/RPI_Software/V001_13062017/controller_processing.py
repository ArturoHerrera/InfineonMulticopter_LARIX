__author__ = "Markus Lechner"
__license__ = "GPL"
__version__ = "0.0.1"
__maintainer__ = "Markus Lechner"
__email__ = "markus.lechner@technikum-wien.at"
__status__ = "Production"

import re
import collections

class Controller_Processing:    
    def get_controlled_param(self):
        with open('/home/pi/rpi_out.txt', 'r') as f:
            ctrld_value = float(f.readline())
            return ctrld_value
        
    def set_controller_param(self, param):        
        with open('/home/pi/rpi_in.txt', 'w') as f:                 
            if isinstance(param, collections.Iterable):       
                for index in range(len(param)):                    
                    param_str = str(param[index])                    
                    param_str = re.sub('[(),]', '', param_str)           
                    f.write(param_str + ' ') 
            else:
                f.write(str(param))
                f.write(' ')  
        return
#EOF