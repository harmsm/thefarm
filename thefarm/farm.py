import camera, daylight, door, webserver 

import json, logging, multiprocessing

class Farm:
    """
    Main class holding the whole farm.
    """

    def __init__(self,json_file):
        """
        """
        
        self._json_file = json_file
        self._read_json()

    def _read_json(self):
        """
        Read the json file that defines this farm.
        """

        self._farm_implement_dict = {}

        parser_dict = {"camera":camera.Camera,
                       "daylight":daylight.Daylight,
                       "door":door.Door,
                       "webserver":webserver.Webserver}

        # load in json file
        data = json.loads(open(self._json_file,'r').read())

        # Make sure required data is in the json file
        required_attr_list = ["latitude","longitude"]

        for a in required_attr_list:
            if a not in data.keys():
                err = "Json file does not have all required data. Missing {}\n".format(a)
                raise ValueError(err)

        # parse resulting json
        for k in data.keys():
            self.__setattr__("_{}".format(k),data[k])

    def start(self):
        """
        Start the system rolling.
        """

        self._webserver_process = mulitprocessing.Process(target=self._webserver.run)
        self._webserver_process.start()

        while True:

            # XX MAKE SURE DAYLIGHT ONLY PINGS SERVER IF THE DAY HAS CHANGED
            for d in self._implement_list:
                d.update()    
   
    def stop(self):
       
        self._webserver_process.terminate()
        logging.info("Stopping farm.")
 

