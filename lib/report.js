/*!
 * libzmap
 * Copyright(c) 2014 Jason Gerfen <jason.gerfen@gmail.com>
 * License: Apache-2.0
 */
'use strict';

// Builds the JSON report of one scan: every result, the hosts that answered, and zmap's metadata.
// The report itself (report.json) is a plain object.
class Report {
  constructor({ collect = true } = {}) {
    this.collect = collect;
    this.started = Date.now();
    this.byAddress = new Map();
    this.json = {
      startedAt: new Date(this.started).toISOString(),
      finishedAt: null,
      durationMs: null,
      counts: { results: 0, success: 0, hosts: 0 },
      hosts: [],
      results: [],
      metadata: null,
    };
  }

  add(result) {
    const { json } = this;
    json.counts.results++;
    // zmap gives success as 1 or 0.
    const success = result.success === true || result.success === 1;
    if (success) json.counts.success++;
    if (!this.collect) return;
    json.results.push(result);
    if (success && typeof result.saddr === 'string') this.addHost(result);
  }

  // One hosts entry for each address that answered, with the ports that answered.
  addHost({ saddr, sport }) {
    let host = this.byAddress.get(saddr);
    if (!host) {
      host = { ip: saddr };
      this.byAddress.set(saddr, host);
      this.json.hosts.push(host);
      this.json.counts.hosts++;
    }
    if (Number.isInteger(sport) && sport > 0) {
      host.ports ??= [];
      if (!host.ports.includes(sport)) host.ports.push(sport);
    }
  }

  finish(metadata) {
    const { json } = this;
    for (const host of json.hosts) host.ports?.sort((a, b) => a - b);
    if (!this.collect) json.counts.hosts = null;
    json.finishedAt = new Date().toISOString();
    json.durationMs = Date.now() - this.started;
    json.metadata = metadata;
    return json;
  }
}

module.exports = { Report };
