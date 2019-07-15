/// <reference path="ast.d.ts" />
import * as fs from "fs";
import parser = require("./parser.js");
import {transpile_identifier, parametersOf} from "./utils";
import toposort_gates from "./toposort";
import RenamingEngine from "./renaming";
import LifetimeManager from "./lifetimes";

if (process.argv.length != 3) {
	console.error(`Syntax: ${process.argv[0]} ${process.argv[1]} file.v`);
	process.exit(1);
}

const target = process.argv[2];
const source = fs.readFileSync(target, "utf8");
// If we stripped comments ourselves, the line numbering in parser errors would be off.
if (/\(\*[^*]+\*\)\n/gm.test(source)) {
	console.error(`The file contains comments, please strip them:

	grep -Fv '(*' original.txt > stripped.txt\n`);
	process.exit(2);
}

let modules: Module[];
try {
	modules = parser.parse(source);
} catch (e) {
	console.error(`[${target}:${e.location.start.line}:${e.location.start.column}] ${e.message}`);
	process.exit(3);
}

console.log(`/* Generated automatically by CapacitorSet/Glovebox */
#include <tfhe.h>

` + modules.map(transpile_module).join("\n"));

// Return the C++ declaration for this item
function get_declaration(decl: Declaration): string {
	const type = decl.size == 1 ? "bit_t" : `fixed_bitspan_t<${decl.size}>`;
	return type + " " + decl.name;
}

// Generate the prototype for these inputs and outputs
function generate_prototype(name: string, decls: Declaration[]): string {
	const outputs = decls
		.filter(it => it.type === "output")
		.map(get_declaration);
	const inputs = decls
		.filter(it => it.type === "input")
		.map(it => " const " + get_declaration(it));
	return `void ${name}(${outputs.concat(inputs).join(", ")})`;
}

type DeclMetadatum = {
	type: "input" | "output" | "wire";
	size: number;
}
type Metadata = {[declaration: string]: DeclMetadatum}

function transpile_module(module: Module) {
	const prototype = generate_prototype(module.name, module.decls);

	let metadata: Metadata = {};
	const lifetimeManager = new LifetimeManager(module.decls);
	for (const decl of module.decls) {
		metadata[decl.name] = {
			type: decl.type,
			size: decl.size
		};
	}

	// Maps identifiers to the ID of the gate whose output they are
	let outputMappings: {[transpiled_identifier: string]: number} = {};

	for (let i = 0; i < module.gates.length; i++)
		outputMappings[transpile_identifier(module.gates[i].z)] = i;

	const path: string[] = toposort_gates(module.gates);

	// Map the topologically-sorted list of identifiers to a topologically-sorted list of gates
	const gateList = path
		// Exclude everything which isn't output by a gate (i.e. circuit inputs)
		.filter(it => it in outputMappings)
		.map(it => {
			const gateID = outputMappings[it];
			return module.gates[gateID];
		});

	/* Track the first and the last time each wire was used. This information can
	 * be used to decide when to allocate and deallocate the corresponding bits.
	 */
	lifetimeManager.populate(gateList);

	// Allocate memory, if we marked this place as the first place the bit is used.
	function declare_if_needed(it: Identifier, pos: number) {
		const {type, size} = metadata[it.value];
		if (type != "wire")
			return "";
		if (lifetimeManager.lifetimes[it.value].firstDeclared != pos)
			return "";

		if (size === 1)
			return `\tbit_t ${it.value} = make_bit();\n`;
		else
			return `\tbitspan_t ${it.value} = make_bitspan(${size});\n`;
	}

	const renamer = new RenamingEngine(gateList, metadata, lifetimeManager.lifetimes);
	const body = gateList
		// Rename wires to reuse allocations
		.map(gate => renamer.renameGateParameters(metadata, gate))
		// Convert to string
		.map((it, pos) => {
			const paramDeclarations = parametersOf(it).map(it => declare_if_needed(it, pos)).join("");
			const parameterList = parametersOf(it).map(transpile_identifier).join(", ")
			const functionCall = `\t_${it.name}(${parameterList});`;
			return paramDeclarations + functionCall;
		})
		.join("\n");

	return prototype + " {\n" + body + "\n};";
}
