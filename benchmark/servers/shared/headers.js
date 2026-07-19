// Approximates the headers the example expresso app attaches on every
// response through its Cors and Cacher middlewares, so response sizes stay
// comparable across frameworks. Kept in one place on purpose: change it here
// and every benchmark server stays in sync.

export const parityHeaders = {
  "access-control-allow-origin": "*",
  "access-control-allow-credentials": "true",
  "cache-control": "max-age=3600",
};

export function applyParityHeaders(res) {
  for (const [name, value] of Object.entries(parityHeaders)) {
    res.setHeader(name, value);
  }
}
