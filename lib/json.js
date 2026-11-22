/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

// zmap writes 64-bit integers (seed, raw addresses) and NaN, which JSON.parse cannot keep.
// Large integers become strings, NaN and infinities become null.
function parseJson(text) {
  if (typeof text !== 'string' || text === '') return null;
  const safe = text
    .replace(/([:[,]\s*)(-?\d{16,})(?=\s*[,}\]])/g, '$1"$2"')
    .replace(/([:[,]\s*)(-?nan|-?NaN|-?inf(?:inity)?|-?Infinity)(?=\s*[,}\]])/g, '$1null');
  try {
    return JSON.parse(safe);
  } catch {
    return null;
  }
}

module.exports = { parseJson };
