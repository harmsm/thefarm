#!/usr/bin/env python3
__description__ = \
"""
"""
__author__ = "Michael J. Harms (harmsm@gmail.com)"
__date__ = "2017-04-30"
__usage__ = "thefarm.py FARM_JSON_FILE [LOG_FILE]"

from farm import Farm

import sys, logging, datetime

def main(argv=None):

    if argv == None:
        argv = sys.argv[1:]

    try:
        json_file = argv[0]
    except IndexError:
        err = "Incorrect arguments.  Usage:\n\n{}\n\n".format(__usage__)
        raise IndexError(err)

    try:
        log_file = argv[1]
    except IndexError:
        log_file = "{}_thefarm.log".format(datetime.datetime.now())
        pass

    print("Logging to {}".format(log_file))
    logging.basicConfig(filename=log_file,
                        format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO)
    logging.info("Starting The Farm using {}.".format(json_file))

    try:
        f = Farm(json_file)
        f.start() 
    except KeyboardInterrupt:
        f.stop()

    logging.info("Stopping The Farm.")        

if __name__ == "__main__":
    main()
