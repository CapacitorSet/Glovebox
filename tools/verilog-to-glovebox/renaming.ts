import {strict as assert} from "assert";
import {parametersOf} from "./utils";

type DeclMetadatum = {
	type: "input" | "output" | "wire";
	size: number;
}
type Metadata = {[key: string]: DeclMetadatum}
type DeclLifetime = {
	firstDeclared: number;
	lastDeclared: number;
}
type Lifetimes = {[key: string]: DeclLifetime}

export default class RenamingEngine {
	renamings: {[old: string]: Identifier} = {};

	constructor(gates: Gate[], metadata: Metadata, lifetimes: Lifetimes) {
		// Array of wires that have been used for the last time (and thus can be reused)
		let free_wires: Identifier[] = [];
		for (let pos = 0; pos < gates.length; pos++) {
			const gate = gates[pos];
			/* Freeing is done in two steps via a buffer. Otherwise, we could have instances
			* like `_not(z, a)` where `a` is used for the last time, freed... and `z` gets
			* renamed to `a`, producing `_not(a, a)` which is not safe. */
			const free_wires_buffer: Identifier[] = [];
			for (const wire of parametersOf(gate)) {
				// We can't exactly rename inputs/outputs...
				if (metadata[wire.value].type != "wire")
					continue;

				const lifetime = lifetimes[wire.value];
				// If this is the first time that this wire is used, try to rename
				// it as an existing free wire.
				if (lifetime.firstDeclared == pos) {
					// There are no free wires: this wire "renames" to itself.
					if (free_wires.length == 0) {
						this.renamings[wire.value] = wire;
					} else {
						const replacement = free_wires.shift();
						this.renamings[wire.value] = replacement;
					}
				/* If this is the last time that this wire is used, mark it as a
				* free wire, or its replacement if possible.
				* "Why its replacement?" So we can reuse the same variable
				* multiple times and reduce code size more effectively.
				*/
				} else if (lifetime.lastDeclared == pos) {
					/* Note that we put replacements at the end of the array (will
					* be picked first and reused more often), and new wires at the
					* start.
					*/
					if (wire.value in this.renamings)
						free_wires_buffer.push(this.renamings[wire.value]);
					else
						free_wires_buffer.unshift(wire);
				}
			}
			free_wires = free_wires.concat(free_wires_buffer);
		}
	}

	rename(metadata: Metadata, wire: Identifier): Identifier {
		if (metadata[wire.value].type != "wire")
			return wire;
		// If anything, a wire will be renamed to itself.
		assert(wire.value in this.renamings);
		return this.renamings[wire.value];
	}

	renameGateParameters(metadata: Metadata, gate: Gate): Gate {
		switch (gate.name) {
			case "mux":
				return {
					type: "gate",
					name: "mux",
					z: this.rename(metadata, gate.z),
					sel: this.rename(metadata, gate.sel),
					t: this.rename(metadata, gate.t),
					f: this.rename(metadata, gate.f)
				};
			case "copy":
			case "not":
				return {
					type: "gate",
					name: gate.name,
					z: this.rename(metadata, gate.z),
					a: this.rename(metadata, gate.a)
				};
			default:
				return {
					type: "gate",
					name: gate.name,
					z: this.rename(metadata, gate.z),
					a: this.rename(metadata, gate.a),
					b: this.rename(metadata, gate.b)
				};
		}
	}	
}