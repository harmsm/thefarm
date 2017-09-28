import tornado.ioloop
import tornado.web

class MainHandler(tornado.web.RequestHandler):

    def get(self):
        name = "yo.jpg"
        self.write("<img src=\"{}\">".format(name))

if __name__ == "__main__":

    application = tornado.web.Application([(r"/", MainHandler),
                                           (r"/(.*)", tornado.web.StaticFileHandler, {'path': ''})])
    application.listen(8888)
    tornado.ioloop.IOLoop.instance().start()

