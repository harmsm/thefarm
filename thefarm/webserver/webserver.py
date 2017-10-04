import tornado.ioloop
import tornado.web

import os

class MainHandler(tornado.web.RequestHandler):

    def get(self):
    
        out = []

        top = \
        """<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no"><link rel="stylesheet" href="css/bootstrap.min.css"></head><body>"""
        out.append(top)

        # BUILD HTML IN HERE
        out.append("<h3>The farm</h3>")
        out.append('<img src="img/door_unknown.png" class="img-responsive" />')

        bottom = \
        """<script src="js/jquery-3.2.1.slim.min.js"</script><script src="js/popper.min.js"</script><script src="js/bootstrap.min.js"</script></body></html>"""
        out.append(bottom)
    
        self.write("".join(out))

if __name__ == "__main__":

    serve_dir = os.path.realpath(os.path.split(__file__)[0])

    application = tornado.web.Application([(r"/", MainHandler),
                                           (r"/(.*)", tornado.web.StaticFileHandler, {'path': serve_dir})])
    application.listen(8888)
    tornado.ioloop.IOLoop.instance().start()

