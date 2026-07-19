// Express.js mirror of example/main.cpp. Uses express idiomatically
// (express.static, res.json, middleware chain) so the benchmark reflects what
// a real express user would deploy, while payloads and headers come from the
// shared fixtures for byte-comparable responses.

import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

import cookieParser from "cookie-parser";
import express from "express";

import { aboutPayload } from "../shared/about.js";
import { applyParityHeaders } from "../shared/headers.js";
import { zipDirectory } from "../shared/zip.js";

const here = path.dirname(fileURLToPath(import.meta.url));
const ASSETS = path.resolve(process.env.ASSETS ?? path.join(here, "../../../assets"));
const PICTURES = path.join(ASSETS, "github");
const PORT = Number(process.env.PORT ?? 8000);

const app = express();

// Mirrors the example's Cors and Cacher middlewares
app.use((req, res, next) => {
  applyParityHeaders(res);
  next();
});
app.use(cookieParser());
app.use(express.json());

app.get("/health", (req, res) => {
  res.status(200).type("text/plain").send("Running healthy\r\n");
});

app.get("/about", (req, res) => {
  res.status(200).json(aboutPayload);
});

app.post("/print", (req, res) => {
  console.log(JSON.stringify(req.body, null, 2));
  res.status(200).json({
    data: req.body,
    timestamp: new Date().toUTCString(),
    printed: true,
  });
});

app.get("/download", (req, res) => {
  const zip = zipDirectory(ASSETS);
  res
    .status(200)
    .set({
      "content-type": "application/zip",
      "content-disposition": 'inline; filename="assets.zip"',
    })
    .send(zip);
});

// Mirrors the listing-enabled StaticServe mounted at /pictures
app.get(["/pictures", "/pictures/"], (req, res) => {
  const items = fs
    .readdirSync(PICTURES)
    .sort()
    .map((entry) => `<li><a href="/pictures/${entry}">${entry}</a></li>`)
    .join("");
  res.status(200).type("text/html").send(`<html><body><ul>${items}</ul></body></html>`);
});
app.use("/pictures", express.static(PICTURES));

// Mirrors the root StaticServe of ../assets
app.use(express.static(ASSETS));

app.use((req, res) => {
  res.status(404).type("text/plain").send("Not Found");
});

app.listen(PORT, () => {
  console.log(`express mirror listening on port ${PORT}`);
});
