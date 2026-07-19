const MIME_TYPES = {
  ".css": "text/css",
  ".html": "text/html",
  ".ico": "image/x-icon",
  ".js": "application/javascript",
  ".json": "application/json",
  ".png": "image/png",
  ".svg": "image/svg+xml",
  ".txt": "text/plain",
  ".zip": "application/zip",
};

export function mimeType(fileName) {
  const dot = fileName.lastIndexOf(".");
  if (dot < 0) {
    return "application/octet-stream";
  }
  return MIME_TYPES[fileName.slice(dot).toLowerCase()] ?? "application/octet-stream";
}
