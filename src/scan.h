/*
 * libzmap
 *
 * Copyright 2014 Jason Gerfen
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * options for zmap
 * See https://zmap.io/documentation.html#args
 */
typedef struct options
{
	// common
	int port;
	char* out;
	char* blacklist;

	// scan
	int max_targets;
	int max_results;
	int max_runtime;
	int rate;
	int bandwidth;
	char* bandwith_size;
	int cooldown;
	int seed;
	int shard;
	int shards;
	int sender_threads;
	int probes;
	int dry_run;

	// network
	char* source_port;
	char* source_ip;
	char* gateway_mac;
	char* interface;

	// probe
	int list_probe_modules;
	char* probe_module;
	char* probe_module_args;
	int list_probe_module_fields;

	// output

} options;

void Scan (options& argv);
