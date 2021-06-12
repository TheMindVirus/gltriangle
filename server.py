from http.server import *
HTTPServer(("0.0.0.0", 9001), SimpleHTTPRequestHandler).serve_forever()
