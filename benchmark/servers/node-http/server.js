// Plain node:http mirror of example/main.cpp. Routes, payloads, and headers
// match the expresso example so load tests compare frameworks, not fixtures.
// Runs unchanged under node, bun, and deno (node-compat).

import fs from "node:fs";
import http from "node:http";
import path from "node:path";
import { fileURLToPath } from "node:url";

import { aboutPayload } from "../shared/about.js";
import { applyParityHeaders } from "../shared/headers.js";
import { mimeType } from "../shared/mime.js";
import { zipDirectory } from "../shared/zip.js";

const here = path.dirname(fileURLToPath(import.meta.url));
const ASSETS = path.resolve(process.env.ASSETS ?? path.join(here, "../../../assets"));
const PICTURES = path.join(ASSETS, "github");
const PORT = Number(process.env.PORT ?? 8000);
const MAX_BODY_SIZE = 1024 * 1024;

function sendText(res, statusCode, body, contentType = "text/plain") {
  res.writeHead(statusCode, {
    "content-type": contentType,
    "content-length": Buffer.byteLength(body),
  });
  res.end(body);
}

function sendJson(res, statusCode, payload) {
  sendText(res, statusCode, JSON.stringify(payload), "application/json");
}

// Resolves a URL path against a root directory, refusing escapes above it.
function resolveWithin(root, urlPath) {
  const resolved = path.resolve(root, "." + path.posix.normalize(urlPath));
  if (resolved !== root && !resolved.startsWith(root + path.sep)) {
    return null;
  }
  return resolved;
}

function serveFile(req, res, filePath) {
  let stats;
  try {
    stats = fs.statSync(filePath);
  } catch {
    return sendText(res, 404, "Not Found");
  }
  if (stats.isDirectory()) {
    filePath = path.join(filePath, "index.html");
    try {
      stats = fs.statSync(filePath);
    } catch {
      return sendText(res, 404, "Not Found");
    }
  }

  const fileName = path.basename(filePath);
  res.setHeader("content-type", mimeType(fileName));
  res.setHeader("content-disposition", `inline; filename="${fileName}"`);
  res.setHeader("accept-ranges", "bytes");

  let start = 0;
  let end = stats.size - 1;
  let statusCode = 200;
  const range = /^bytes=(\d*)-(\d*)$/.exec(req.headers.range ?? "");
  if (range && (range[1] !== "" || range[2] !== "")) {
    start = range[1] === "" ? 0 : Number(range[1]);
    end = range[2] === "" ? stats.size - 1 : Number(range[2]);
    if (start >= stats.size || end >= stats.size || start > end) {
      return sendText(res, 416, "Invalid Range");
    }
    statusCode = 206;
    res.setHeader("content-range", `bytes ${start}-${end}/${stats.size}`);
  }

  res.writeHead(statusCode, { "content-length": end - start + 1 });
  fs.createReadStream(filePath, { start, end }).pipe(res);
}

function serveListing(res, directory, urlPrefix) {
  let entries;
  try {
    entries = fs.readdirSync(directory).sort();
  } catch {
    return sendText(res, 404, "Not Found");
  }
  const items = entries
    .map((entry) => `<li><a href="${urlPrefix}/${entry}">${entry}</a></li>`)
    .join("");
  sendText(res, 200, `<html><body><ul>${items}</ul></body></html>`, "text/html");
}

function readBody(req) {
  return new Promise((resolve, reject) => {
    const chunks = [];
    let size = 0;
    req.on("data", (chunk) => {
      size += chunk.length;
      if (size > MAX_BODY_SIZE) {
        reject(new Error("Body too large"));
        req.destroy();
        return;
      }
      chunks.push(chunk);
    });
    req.on("end", () => resolve(Buffer.concat(chunks).toString()));
    req.on("error", reject);
  });
}

async function handlePrint(req, res) {
  let body;
  try {
    const raw = await readBody(req);
    body =
      req.headers["content-type"] === "application/json" && raw !== ""
        ? JSON.parse(raw)
        : raw;
  } catch {
    return sendText(res, 400, "Bad Request");
  }

  console.log(JSON.stringify(body, null, 2));
  sendJson(res, 200, {
    data: body,
    timestamp: new Date().toUTCString(),
    printed: true,
  });
}

const server = http.createServer((req, res) => {
  applyParityHeaders(res);

  // Mirrors expresso's CookieParser middleware
  req.cookies = Object.fromEntries(
    (req.headers.cookie ?? "")
      .split(";")
      .map((cookie) => cookie.trim().split("="))
      .filter((pair) => pair.length === 2),
  );

  const urlPath = decodeURIComponent(new URL(req.url, "http://localhost").pathname);

  if (req.method === "GET" && urlPath === "/health") {
    return sendText(res, 200, "Running healthy\r\n");
  }
  if (req.method === "GET" && urlPath === "/about") {
    return sendJson(res, 200, aboutPayload);
  }
  if (req.method === "POST" && urlPath === "/print") {
    return handlePrint(req, res);
  }
  if (req.method === "GET" && urlPath === "/download") {
    const zip = zipDirectory(ASSETS);
    res.writeHead(200, {
      "content-type": "application/zip",
      "content-disposition": 'inline; filename="assets.zip"',
      "content-length": zip.length,
    });
    return res.end(zip);
  }
  if (req.method === "GET" && (urlPath === "/pictures" || urlPath === "/pictures/")) {
    return serveListing(res, PICTURES, "/pictures");
  }
  if (req.method === "GET" && urlPath.startsWith("/pictures/")) {
    const filePath = resolveWithin(PICTURES, urlPath.slice("/pictures".length));
    if (filePath === null) {
      return sendText(res, 404, "Not Found");
    }
    return serveFile(req, res, filePath);
  }
  if (req.method === "GET") {
    const filePath = resolveWithin(ASSETS, urlPath);
    if (filePath === null) {
      return sendText(res, 404, "Not Found");
    }
    return serveFile(req, res, filePath);
  }

  return sendText(res, 404, "Not Found");
});

server.listen(PORT, () => {
  console.log(`node-http mirror listening on port ${PORT}`);
});
