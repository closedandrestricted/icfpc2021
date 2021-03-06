#!/usr/bin/python3

import sys
import json
import os
from posixpath import join
import tornado.ioloop
import tornado.web


this_dir = os.path.dirname(os.path.abspath(__file__))


class MainHandler(tornado.web.RequestHandler):
    def prepare(self):
        header = "Content-Type"
        body = "text/html"
        self.set_header(header, body)
    
    def get(self):
        with open(os.path.join(this_dir, './static/index.html')) as f:
            data = f.read()
            self.write(data)


class ProblemHandler(tornado.web.RequestHandler):
    def prepare(self):
        header = "Content-Type"
        body = "application/json"
        self.set_header(header, body)

    def get(self):
        id = self.get_argument('id')
        with open(os.path.join(this_dir, '../problems/%s.json' % id)) as f:
            data = f.read()
            self.write(data)


class SolutionHandler(tornado.web.RequestHandler):
    def prepare(self):
        header = "Content-Type"
        body = "application/json"
        self.set_header(header, body)

    def get(self):
        id = self.get_argument('id')
        kind = self.get_argument('kind', 'staging')
        with open(os.path.join(this_dir, '../solutions/%s/%s.json' % (kind, id))) as f:
            data = f.read()
            self.write(data)


class SolutionSaveHandler(tornado.web.RequestHandler):

    def post(self):
        id = self.get_argument('id')
        sol = json.loads(self.request.body)
        with open(os.path.join(this_dir, '../solutions/webedit/%s.json' % id), 'w') as f:
            f.write(json.dumps(sol))


class StopHandler(tornado.web.RequestHandler):

    def get(self):
        tornado.ioloop.IOLoop.current().stop()
        sys.exit(0)

class NoCacheStaticFileHandler(tornado.web.StaticFileHandler):
    def set_extra_headers(self, path):
        self.set_header('Cache-Control', 'no-store, no-cache, must-revalidate, max-age=0')

def make_app():
    return tornado.web.Application([
        (r"/", MainHandler),
        (r"/problem", ProblemHandler),
        (r"/solution", SolutionHandler),
        (r"/save_sol", SolutionSaveHandler),
        (r"/stop", StopHandler),
        (r"/static/(.*)", NoCacheStaticFileHandler, {"path": os.path.join(this_dir, "static")}),
    ], debug=True)


if __name__ == "__main__":
    app = make_app()
    app.listen(8888)
    tornado.ioloop.IOLoop.current().start()
