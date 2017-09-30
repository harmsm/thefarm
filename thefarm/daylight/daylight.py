__description__ = \
"""
To change servers, make a subclass of DaylightServer and re-define the 
_grab_from_server method.
"""
__date__ = "2017-04-12"
__author__ = "Michael J. Harms (harmsm@gmail.com)"

import urllib.request, json
from datetime import datetime, timedelta
import logging

class DaylightException(Exception):
    """
    Exception for this module.
    """
    def __init__(self,*args,**kwargs):
        logging.warning(args[0])
        super().__init__(*args,**kwargs)

class DaylightServer:
    """
    Get information about daylight (sunrise, sunset, etc.) from a server.
    """

    def __init__(self,latitude,longitude,twilight="civil"):
        """
        """

        self._latitude = latitude
        self._longitude = longitude

        # Deal with twilight definition
        self._twilight = twilight
        if self._twilight not in ["civil","nautical","astronomical"]:
            err = "twilight \"{}\" not recognized.  should be civil, nautical or astronomical\n".format(self._twilight)
            raise DaylightException(err) 

        self._utc_offset = datetime.now() - datetime.utcnow()

        # Update 
        self.update()

    def update(self):
        """
        """
       
        try:
            self._grab_from_server()
            self._last_check_time = datetime.now() 
        except (ValueError,urllib.error.URLError) as e:
            err = "Problem downloading sunrise/sunset times"
            raise DaylightException(err)
        except:
            err = "Unknown problem updating sunrise/sunset times!"
            raise DaylightException(err)
 
    def _grab_from_server(self):
        """
        Grab sunrise, sunset, twilight_begin, and twilight_end from the 
        sunrise-sunset.org server, storing the information as datetime.datetime
        objects that can be accessed using the Daylight properties.
       
        Sets the following (as datetime objects in current time zone):
            self._twilight_begin 
            self._twilight_end
            self._sunrise     
            self._sunset 
        """       

        url_base = "https://api.sunrise-sunset.org/json?lat={:.7f}&lng={:.7f}&date=today&formatted=0"
        time_format = "%Y-%m-%dT%H:%M:%S+00:00"

        url = url_base.format(self._latitude,self._longitude)

        # Grab the date 
        with urllib.request.urlopen(url) as u:
            json_data = json.loads(u.read().decode())
       
        # Make sure the read worked 
        if json_data['status'] != "OK":
            err = "server json could not be read\n"
            raise DaylightException(err)

        # Parse resulting json
        twilight_begin = json_data["results"]["{}_twilight_begin".format(self._twilight)]
        twilight_end   = json_data["results"]["{}_twilight_end".format(self._twilight)]
        sunrise = json_data["results"]["sunrise"]
        sunset  = json_data["results"]["sunset"]

        # Convert to dateime objects 
        self._twilight_begin = datetime.strptime(twilight_begin,time_format) + self._utc_offset
        self._twilight_end   = datetime.strptime(twilight_end,time_format)   + self._utc_offset
        self._sunrise        = datetime.strptime(sunrise,time_format)        + self._utc_offset
        self._sunset         = datetime.strptime(sunset,time_format)         + self._utc_offset

    @property
    def last_update(self):
        return self._last_check_time
 
    @property
    def sunrise(self):
        if self._sunrise.day != self._last_check_time.day:
            self._grab_from_server() 
        return self._sunrise

    @property
    def sunset(self):
        if self._sunset.day != self._last_check_time.day:
            self._grab_from_server() 
        return self._sunset

    @property
    def twilight_begin(self):
        if self._twilight_end != self._last_check_time.day:
            self._grab_from_server() 
        return self._twilight_begin

    @property
    def twilight_end(self):
        if self._twilight_end.day != self._last_check_time.day:
            self._grab_from_server() 
        return self._twilight_end


