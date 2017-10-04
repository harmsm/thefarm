from picamera import PiCamera
from PIL import Image, ImageDraw, ImageFont

import time, os, random, string

class CameraException(Exception):
    """
    Exception for this module.
    """
    def __init__(self,*args,**kwargs):
        logging.warning(args[0])
        super().__init__(*args,**kwargs)

class CameraMonitor:

    def __init__(self,resolution=(2592,1944),font_size=100):

        self._resolution = resolution
        self._font_size = font_size

        directory = os.path.dirname(os.path.realpath(__file__))
        self._font = os.path.join(directory,"LiberationSans-Regular.ttf") 

    def capture(self,output_file="cam_output.jpg"):
        """
        Capture an image and time stamp it.
        """
         
        tmp = "{}_image.jpg".format("".join([random.choice(string.ascii_letters)
                                             for i in range(10)]))

        # Take image
        c = PiCamera()
        c.resolution = self._resolution
        c.capture(tmp)

        img = Image.open(tmp).convert('RGBA')

        # make a blank image for the text, initialized to transparent text color
        txt = Image.new('RGBA', img.size, (255,255,255,0))

        # get a font
        fnt = ImageFont.truetype(self._font,size=self._font_size)
        d = ImageDraw.Draw(txt)

        # Text location
        location = (int(round(0.02*self._resolution[0],0)),
                    int(round(0.92*self._resolution[1],0)))

        # draw text, full opacity
        d.text(location,time.strftime("%c"),fill=(255,255,255,200),font=fnt)

        out = Image.alpha_composite(img, txt)
        out.save(output_file,"JPEG")

        os.remove(tmp)

    @property
    def web_content(self):
        """
        """     

        # Take new image (on it's own thread)
        p = Process(target=self.capture)
        p.start() 

        # Return html pointing to image
        out = []
        out.append("<div class=\"well\">")
        out.append("<img class=\"img-responsive\" src=\"cam_output.jpg\"/>")
        out.append("</div>")
 
        return "".join(out)
