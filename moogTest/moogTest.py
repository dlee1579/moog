import time
from datetime import datetime;
from ISStreamer.Streamer import Streamer

streamer = Streamer(bucket_name="MoogTest", bucket_key="5LRM9UG8CASH",
                    access_key="NCbUQzFnRPMVoXDSjUL40Paxs0ICSV0Q")

#streamer.log("My Messages", "Stream Starting")
print(datetime.now().time())
for num in range(1, 420):
    streamer.log("Test Values", num)
    # time.sleep(0.1)
    # streamer.log("My Numbers", num)
    # if num % 2 == 0:
    #     streamer.log("My Booleans", "false")
    # else:
    #     streamer.log("My Booleans", "true")
    # if num % 3 == 0:
    #     streamer.log("My Events", "pop")
    # if num % 500 == 0:
    #     streamer.log("My Messages", "Stream Half Done")
#streamer.log("My Messages", "Stream Done")
print(datetime.now().time())
