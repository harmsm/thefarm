import json, logging

class Farm:
    """
    Main class holding the whole farm.
    """

    def __init__(self,json_file):
        """
        """

        # load in json file
        data = json.loads(open(json_file,'r').read())

        # Make sure required data is in the json file
        required_attr_list = ["latitude","longitude"]

        for a in required_attr_list:
            if a not in data.keys():
                err = "Json file does not have all required data. Missing {}\n".format(a)
                raise ValueError(err)

        # parse resulting json
        for k in data.keys():
            self.__setattr__("_{}".format(k),data[k])

        # Get the utc offset for our current time
        self._utc_offset = datetime.now() - datetime.utcnow()

        # get the current sunrise, sunset etc.
        self._get_suntimes()


