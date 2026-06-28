#!/usr/bin/env python3
"""Tiny live viewer for ESP-MoNet camera frames.

Serves an auto-refreshing page showing the newest JPEG that the HTTPS server
saved under <server>/build/images/. Run it, then open http://127.0.0.1:8090.

Usage: ./viewer.py [IMAGES_DIR] [PORT]
"""
import http.server, socketserver, os, sys, glob, json

IMAGES_DIR = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "build", "images")
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 8090

PAGE = """<!doctype html><html><head><meta charset=utf-8>
<title>ESP-MoNet live</title>
<style>
 body{background:#0d0d0f;color:#cfcfd4;font-family:system-ui,sans-serif;text-align:center;margin:0}
 h1{font-size:15px;font-weight:500;letter-spacing:.04em;padding:12px;margin:0;color:#8ab4f8}
 img{image-rendering:pixelated;width:min(92vw,640px);border:1px solid #2a2a30;background:#000;border-radius:4px}
 #meta{font-size:12px;color:#777;margin-top:8px;font-variant-numeric:tabular-nums}
 #fps{color:#5fd35f}
</style></head><body>
<h1>ESP-MoNet — live camera (OV2640 / QVGA)</h1>
<img id=cam alt="waiting for first frame…">
<div id=meta>waiting for first frame…</div>
<script>
let n=0,last="",t0=Date.now();
async function tick(){
 try{
  const j=await (await fetch('/latest?'+Date.now())).json();
  if(j.name && j.name!==last){
   last=j.name; n++;
   document.getElementById('cam').src='/img/'+j.name+'?'+Date.now();
   const fps=(n/((Date.now()-t0)/1000)).toFixed(1);
   document.getElementById('meta').innerHTML=
     j.name+'  ·  '+j.size+' B  ·  frame #'+n+'  ·  <span id=fps>~'+fps+' fps</span>';
  }
 }catch(e){}
}
setInterval(tick,250); tick();
</script></body></html>"""

class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def _send(self, code, ctype, body):
        self.send_response(code); self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', str(len(body))); self.end_headers()
        self.wfile.write(body)
    def do_GET(self):
        if self.path == '/':
            return self._send(200, 'text/html', PAGE.encode())
        if self.path.startswith('/latest'):
            files = glob.glob(os.path.join(IMAGES_DIR, '*.jpg'))
            if files:
                newest = max(files, key=os.path.getmtime)
                body = json.dumps({'name': os.path.basename(newest),
                                   'size': os.path.getsize(newest)}).encode()
            else:
                body = b'{"name":null}'
            return self._send(200, 'application/json', body)
        if self.path.startswith('/img/'):
            name = os.path.basename(self.path.split('?')[0][5:])
            p = os.path.join(IMAGES_DIR, name)
            if os.path.isfile(p):
                with open(p, 'rb') as fh:
                    return self._send(200, 'image/jpeg', fh.read())
        self._send(404, 'text/plain', b'not found')

socketserver.ThreadingTCPServer.allow_reuse_address = True
print("ESP-MoNet viewer: http://127.0.0.1:%d  (images: %s)" % (PORT, IMAGES_DIR))
socketserver.ThreadingTCPServer(("127.0.0.1", PORT), H).serve_forever()
