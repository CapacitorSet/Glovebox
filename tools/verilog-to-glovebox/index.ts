const assert = require("assert");
const fs = require("fs");
const parser = require("./parser.js");
const toposort = require("toposort");

if (process.argv.length != 3) {
	console.error(`Syntax: ${process.argv[0]} ${process.argv[1]} file.v`);
	process.exit(1);
}

const target = process.argv[2];
const source = fs.readFileSync(target, "utf8");
// If we stripped comments ourselves, the line numbering in parser errors would be off.
if (/\(\*[^*]+\*\)\n/gm.test(source)) {
	console.error("The file contains comments, please strip them:\n\n    grep -Fv '(*' original.txt > stripped.txt\n");
	process.exit(2);
}

let ast: Module[];
try {
	ast = parser.parse(source);
} catch (e) {
	console.error(`[${target}:${e.location.start.line}:${e.location.start.column}] ${e.message}`);
	process.exit(3);
}

let buf = `/* Generated automatically by CapacitorSet/Glovebox */
#include <tfhe.h>

`;

buf += ast.map(transpile_module).join("");

console.log(buf);

// Return the C++ type appropriate for this size
function get_type(size: number): string {
	return size == 1 ? "bit_t" : `fixed_bitspan_t<${size}>`;
}

// Generate the prototype for these inputs and outputs
function generate_prototype(name: string, decls: Declaration[]): string {
	const outputs = decls
		.filter(it => it.type === "output")
		.map(it => get_type(it.size) + " " + it.name);
	const inputs = decls
		.filter(it => it.type === "input")
		.map(it => "const " + get_type(it.size) + " " + it.name);
	return `void ${name}(${outputs.concat(inputs).join(", ")}, weak_params_t p)`;
}

// Add indices where necessary
function transpile_identifier(it: Identifier): string {
	if (!it.index)
		return it.value;
	assert(!it.index.end);
	return it.value + "[" + it.index.begin + "]";
}

type DeclMetadatum = {
	type: "input" | "output" | "wire";
	size: number;
	firstDeclared: number;
	lastDeclared: number;
}
type Metadata = {[key: string]: DeclMetadatum}

type Edge = [string, string];
type Graph = Edge[];

function inputsOf(gate: Gate): Identifier[] {
	switch (gate.gate) {
		case "mux":
			return [gate.sel, gate.t, gate.f];
		case "copy":
		case "not":
			return [gate.a];
		default:
			return [gate.a, gate.b];
	}
}

// Return inputs and outputs of a gate
function bitsOf(gate: Gate): Identifier[] {
	return inputsOf(gate).concat(gate.z);
}

function gateToEdges(gate: Gate): Edge[] {
	function makeEdge(it: Identifier): Edge {
		return [transpile_identifier(it), transpile_identifier(gate.z)];
	}
	return inputsOf(gate).map(makeEdge);
}

// Add firstDeclared and return the updated metadata
function addLifetimeStartMetadata(md: Metadata, gate: Gate, i: number): Metadata {
	for (const ident of bitsOf(gate))
		if (md[ident.value].firstDeclared === undefined)
			md[ident.value].firstDeclared = i;
	return md;
}

// Add lastDeclared
function addLifetimeEndMetadata(md: Metadata, gate: Gate, i: number): Metadata {
	for (const ident of bitsOf(gate))
		if (md[ident.value].lastDeclared === undefined)
			md[ident.value].lastDeclared = i;
	return md;
}

function renameGateBits(renamings: {[old: string]: Identifier}, metadata: Metadata, gate: Gate): Gate {
	function rename(wire: Identifier): Identifier {
		if (metadata[wire.value].type != "wire")
			return wire;
		// If anything, a wire will be renamed to itself.
		assert(wire.value in renamings);
		return renamings[wire.value];
	}
	switch (gate.gate) {
		case "mux":
			return {
				type: "gate",
				gate: "mux",
				z: rename(gate.z),
				sel: rename(gate.sel),
				t: rename(gate.t),
				f: rename(gate.f)
			};
		case "copy":
		case "not":
			return {
				type: "gate",
				gate: gate.gate,
				z: rename(gate.z),
				a: rename(gate.a)
			};
		default:
			return {
				type: "gate",
				gate: gate.gate,
				z: rename(gate.z),
				a: rename(gate.a),
				b: rename(gate.b)
			};
	}
}

function transpile_module(_: Module) {
	let buf = "";
	
	buf += generate_prototype(_.name, _.decls) + " {\n";

	let metadata: Metadata = {};
	for (const decl of _.decls) {
		metadata[decl.name] = {
			type: decl.type,
			size: decl.size,
			firstDeclared: undefined,
			lastDeclared: undefined,
		};
	}

	// Maps identifiers to the ID of the gate whose output they are
	let outputMappings: {[transpiled_identifier: string]: number} = {};

	for (let i = 0; i < _.gates.length; i++)
		outputMappings[transpile_identifier(_.gates[i].z)] = i;

	/* The circuit must be topologically sorted, i.e. the inputs of each gate must
	 * be calculated before the gate itself.
	 */
	type Edge = [string, string];
	const graph: Edge[] = _.gates
		.map(gateToEdges)
		.reduce((arr, it) => arr.concat(it), []);

	// The list of wires/outputs in topological order
	const path: string[] = toposort(graph);
	// The list of corresponding gates in topological order
	const gateList = path
		// Exclude everything which isn't output by a gate (i.e. circuit inputs)
		.filter(it => it in outputMappings)
		.map(it => {
			const gateID = outputMappings[it];
			return _.gates[gateID];
		});


	/* Track the first and the last time each wire was used. This information can
	 * be used to decide when to allocate and deallocate the corresponding bits.
	 */
	metadata = gateList.reduce(addLifetimeStartMetadata, metadata);
	metadata = gateList.reduceRight(addLifetimeEndMetadata, metadata);

	let renamings: {[old: string]: Identifier} = {};
	// Array of wires that have been used for the last time (and thus can be reused)
	let free_wires: Identifier[] = [];
	for (let i = 0; i < gateList.length; i++) {
		const gate = gateList[i];
		// Freeing is done in two steps to avoid renaming eg. _not(z, a) to _not(a, a)
		const wires_freed_this_step: Identifier[] = [];
		for (const wire of bitsOf(gate)) {
			const md = metadata[wire.value];
			// We can't exactly rename inputs/outputs...
			if (md.type != "wire")
				continue;
			// If this is the first time that this wire is used, try to rename
			// it as an existing free wire.
			if (md.firstDeclared == i) {
				// There are no free wires: this wire "renames" to itself.
				if (free_wires.length == 0) {
					renamings[wire.value] = wire;
				} else {
					const replacement = free_wires.shift();
					renamings[wire.value] = replacement;
				}
			/* If this is the last time that this wire is used, mark it as a
			 * free wire, or its replacement if possible.
			 * "Why its replacement?" So we can reuse the same variable
			 * multiple times and reduce code size more effectively.
			 */
			} else if (md.lastDeclared == i) {
				/* Note that we put replacements at the end of the array (will
				 * be picked first and reused more often), and new wires at the
				 * start.
				 */
				if (wire.value in renamings)
					wires_freed_this_step.push(renamings[wire.value]);
				else
					wires_freed_this_step.unshift(wire);
			}
		}

		free_wires = free_wires.concat(wires_freed_this_step);
	}

	const renamedGates = gateList.map(gate => renameGateBits(renamings, metadata, gate));

	// Allocate memory, if we marked this place as the first place the bit is used.
	function declare_if_needed(it: Identifier, i: number) {
		const {type, size, firstDeclared} = metadata[it.value];
		if (type != "wire")
			return "";
		if (firstDeclared != i)
			return "";

		if (size === 1)
			return `  bit_t ${it.value} = make_bit(p);\n`;
		else
			return `  bitspan_t ${it.value} = make_bitspan(${size}, p);\n`;
	}

	const gates = renamedGates
		.map((it, i) => {
			let ret = "";
			ret += bitsOf(it).map(it => declare_if_needed(it, i)).join("");
			switch (it.gate) {
			case "mux":
				ret += `  _mux(${transpile_identifier(it.z)}, ${transpile_identifier(it.sel)}, ${transpile_identifier(it.t)}, ${transpile_identifier(it.f)}, p);\n`;
				break;
			case "copy":
			case "not":
				ret += `  _${it.gate}(${transpile_identifier(it.z)}, ${transpile_identifier(it.a)}, p);\n`;
				break;
			default:
				ret += `  _${it.gate}(${transpile_identifier(it.z)}, ${transpile_identifier(it.a)}, ${transpile_identifier(it.b)}, p);\n`;
				break;
			}
			return ret;
		})
		.join("");
	buf += gates;

	/*
	const final_assignments = _.assignments
		.filter(is_final_assignment)
		.map(transpile_assignment)
		.join("");
	buf += final_assignments;
	*/

	buf += "}\n"

	return buf;
}
